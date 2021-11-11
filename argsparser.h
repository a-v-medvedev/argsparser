/*
 * Copyright (c) 2018-2021 Alexey V. Medvedev
 * This code is an extension of the parts of Intel(R) MPI Benchmarks project.
 * It keeps the same 3-Clause BSD License.
 */

/*****************************************************************************
 *                                                                           *
 * Copyright (c) 2016-2018 Intel Corporation.                                *
 * All rights reserved.                                                      *
 *                                                                           *
 *****************************************************************************

*/

#pragma once

// Note: yaml-cpp 0.6.x library binding is implemented.
// This doesn't work with earlier versions and requires C++11 compiler.
#define WITH_YAML_CPP

#include <vector>
#include <iostream>
#include <sstream>
#include <assert.h>
#include <string>
#include <map>
#include <set>
#include <stdexcept>
#include <memory>
#ifdef WITH_YAML_CPP
#include "yaml-cpp/yaml.h"
#endif

class args_parser {
    protected:
    int argc;
    const char * const *argv;
    const char *option_starter;
    const char option_delimiter;
    std::ostream &sout;
    const static int version;
    std::string program_name;
    bool parse_done = false;

    public:
    args_parser() : argc(0), argv(0), option_starter("--"), option_delimiter('='), sout(std::cout), prev_option(NULL), last_error(NONE) {}
    args_parser(int &_argc, char * *&_argv, const char *opt_st = "--", 
                char opt_delim = '=', std::ostream &_sout = std::cout) : argc(_argc), argv(_argv), 
                                                                         option_starter(opt_st), 
                                                                         option_delimiter(opt_delim), 
                                                                         sout(_sout),
                                                                         prev_option(NULL),
                                                                         last_error(NONE)  
    { auto &dummy = expected_args["EXTRA_ARGS"]; (void)dummy; } 
    typedef enum { STRING, INT, FLOAT, BOOL } arg_t;
    typedef enum { ALLOW_UNEXPECTED_ARGS, SILENT, NOHELP, NODUPLICATE /*, NODEFAULTSDUMP*/ } flag_t;
    typedef enum { NONE, NO_REQUIRED_OPTION, NO_REQUIRED_EXTRA_ARG, PARSE_ERROR_OPTION, PARSE_ERROR_EXTRA_ARGS, UNKNOWN_EXTRA_ARGS } error_t;

    class value {
        public:
            value() {}
            value(float v) : initialized(true), i(0), str("(none)"), b(false) { type = FLOAT; f = v; }
            value(int v) : initialized(true), f(0), str("(none)"), b(false) { type = INT; i = v; }
            value(bool v) : initialized(true), i(0), f(0), str("(none)") { type = BOOL; b = v; }
            value(std::string v) : initialized(true), i(0), f(0), b(false) { type = STRING; str = v; }
            value(const char *v) : initialized(true), i(0), f(0), b(false) { type = STRING; str.assign(v); }
            value(const value &other);
        public:
            bool initialized = false;
            int i;
            float f;
            std::string str;
            bool b;
            arg_t type;
        public:
            bool is_initialized() const { return initialized; };
            value &operator=(const value &other);
            bool parse(const char *sval, arg_t _type);
            friend std::ostream &operator<<(std::ostream &s, const args_parser::value &val);
            void sanity_check(arg_t _type) const;
            static const std::string get_type_str(arg_t _type); 
    };
    struct option {
        const args_parser &parser;
        enum mode { APPLY_DEFAULTS_ONLY_WHEN_MISSING };
        std::string str;
        arg_t type;
        bool required;
        bool defaultize_before_parsing;
        bool defaulted;
        bool flag;
        std::string caption;
        std::string description;
        option(const args_parser &_parser, const std::string _str, arg_t _type, bool _required) : parser(_parser), str(_str), 
                                                               type(_type), required(_required), 
                                                               defaultize_before_parsing(true), 
                                                               defaulted(false), flag(false) {};
        virtual void print() const = 0;
        virtual bool do_parse(const char *sval) = 0;
        virtual bool is_scalar() const = 0;
        virtual bool is_map() const = 0;
        virtual void set_default_value() = 0;
        virtual option &set_caption(const std::string &cap) { caption = cap; return *this; }
        virtual option &set_description(const std::string &descr) { description = descr; return *this; }
        virtual option &set_mode(mode m) { 
            if (m == APPLY_DEFAULTS_ONLY_WHEN_MISSING) 
                defaultize_before_parsing = false; 
            return *this; 
        }
        virtual bool is_default_setting_required() = 0;
        virtual bool is_required_but_not_set() = 0;
        virtual std::vector<args_parser::value> get_value_as_vector() const = 0;
        virtual bool get_value_as_map(std::map<std::string, std::string> &r) const = 0;
        virtual void to_ostream(std::ostream &s) const = 0;
        friend std::ostream &operator<<(std::ostream &s, const args_parser::option &d);
#ifdef WITH_YAML_CPP        
        virtual void to_yaml(YAML::Emitter& out) const = 0;
        virtual void from_yaml(const YAML::Node& node) = 0;
#endif        
        virtual ~option() {}
        private:
        option(const option &other) : parser(other.parser) {}
        option &operator=(const option &) { return *this; }
    };
    struct option_scalar : public option {
        args_parser::value def;
        args_parser::value val;
        option_scalar(const args_parser &_parser, const std::string _str, arg_t _type) : option(_parser,_str, _type, true) { }
        option_scalar(const args_parser &_parser, const std::string _str, arg_t _type, const value &_def) : option(_parser, _str, _type, false), def(_def)
        { def.sanity_check(type); }
        virtual ~option_scalar() {}
        virtual void print() const { parser.sout << str << ": " << val << std::endl; }
        virtual bool do_parse(const char *sval);
        virtual bool is_scalar() const { return true; }
        virtual bool is_map() const { return false; }
        virtual void to_ostream(std::ostream &s) const { s << val; }
#ifdef WITH_YAML_CPP        
        virtual void to_yaml(YAML::Emitter& out) const;
        virtual void from_yaml(const YAML::Node& node);
#endif        
        virtual void set_default_value() { val = def; defaulted = true; }
        virtual bool is_default_setting_required() { return !val.is_initialized() && !required; }
        virtual bool is_required_but_not_set() { return required && !val.is_initialized(); }
        virtual std::vector<args_parser::value> get_value_as_vector() const { std::vector<args_parser::value> r; r.push_back(val); return r; }
        virtual bool get_value_as_map(std::map<std::string, std::string> &) const { return false; }
    };
    struct option_vector : public option {
        enum { MAX_VEC_SIZE = 1024 };
        char vec_delimiter;
        int vec_min;
        int vec_max;
        int num_already_initialized_elems;
        std::vector<args_parser::value> val;
        std::string vec_def;
        option_vector(const args_parser &_parser, const std::string _str, arg_t _type, 
                     char _vec_delimiter, int _vec_min, int _vec_max)  :
            option(_parser, _str, _type, true), vec_delimiter(_vec_delimiter), vec_min(_vec_min), vec_max(_vec_max)
        { num_already_initialized_elems = 0; }
        option_vector(const args_parser &_parser, std::string _str, arg_t _type, 
                     char _vec_delimiter, int _vec_min, int _vec_max, 
                     const std::string &_vec_def)  :
            option(_parser, _str, _type, false), vec_delimiter(_vec_delimiter), vec_min(_vec_min), vec_max(_vec_max), 
            vec_def(_vec_def)
        { num_already_initialized_elems = 0; }
        virtual ~option_vector() {}
        virtual void print() const {
            parser.sout << str << ": ";
            to_ostream(parser.sout);
            parser.sout << std::endl;
        }
        virtual bool do_parse(const char *sval);
        virtual bool is_scalar() const { return false; }
        virtual bool is_map() const { return false; }
        virtual void to_ostream(std::ostream &s) const { for (size_t i = 0; i < val.size(); i++) { s << val[i]; if (i != val.size()) s << ", "; } }
#ifdef WITH_YAML_CPP        
        virtual void to_yaml(YAML::Emitter& out) const;
        virtual void from_yaml(const YAML::Node& node);
#endif        
        virtual void set_default_value();
        virtual bool is_default_setting_required() { return val.size() == 0 && !required; }
        virtual bool is_required_but_not_set() { return required && vec_min != 0 && val.size() == 0; }
        virtual std::vector<args_parser::value> get_value_as_vector() const { return val; }
        virtual bool get_value_as_map(std::map<std::string, std::string> &) const { return false; }
    };
    struct option_map : public option {
        enum { MAX_VEC_SIZE = 1024 };
        char vec_delimiter;
        char kv_delimiter = '=';
        int num_already_initialized_elems;
        bool map_always_override = false;
        std::vector<args_parser::value> val;
        std::string vec_def;
        std::map<std::string, std::string> kvmap;
        option_map(const args_parser &_parser, const std::string &_str, char _vec_delimiter, char _kv_delimiter)  :
            option(_parser, _str, STRING, true), vec_delimiter(_vec_delimiter), kv_delimiter(_kv_delimiter)
        { num_already_initialized_elems = 0; }
        option_map(const args_parser &_parser, const std::string &_str, const std::string &_vec_def, char _vec_delimiter, char _kv_delimiter)  :
            option(_parser, _str, STRING, false), vec_delimiter(_vec_delimiter), kv_delimiter(_kv_delimiter), 
            vec_def(_vec_def)
        { num_already_initialized_elems = 0; }

        virtual ~option_map() {}
        virtual void print() const {
            parser.sout << str << ": ";
            to_ostream(parser.sout);
            parser.sout << std::endl;
        }
        virtual bool do_parse(const char *sval);
        virtual bool is_scalar() const { return false; }
        virtual bool is_map() const { return true; }
        virtual void to_ostream(std::ostream &s) const { for (size_t i = 0; i < val.size(); i++) { s << val[i]; if (i != val.size()) s << ", "; } }
#ifdef WITH_YAML_CPP        
        virtual void to_yaml(YAML::Emitter& out) const;
        virtual void from_yaml(const YAML::Node& node);
#endif        
        virtual void set_default_value();
        virtual bool is_default_setting_required() { return val.size() == 0 && !required; }
        virtual bool is_required_but_not_set() { return false; }
        virtual std::vector<args_parser::value> get_value_as_vector() const { return val; }
        virtual bool get_value_as_map(std::map<std::string, std::string> &r) const { r = kvmap; return true; }
    };    

    protected:
    std::set<flag_t> flags;
    std::string current_group;
    std::map<std::string, std::vector<std::shared_ptr<option>>> expected_args;
    std::vector<std::string> unknown_args;
    option *prev_option;
    error_t last_error;
    std::string last_error_option;
    std::string last_error_extra;
   
    bool match(const std::string &arg, const std::string &pattern) const;
    bool match(const std::string &arg, option &exp) const;
    bool get_value(const std::string &arg, option &exp);
    void get_default_value(option &d);
 

    void get_extra_args_num(int &num_extra_args, int &num_required_extra_args) const;
    std::vector<std::shared_ptr<option>>  &get_extra_args_info(int &num_extra_args, int &num_required_extra_args);
    const std::vector<std::shared_ptr<option>>  &get_extra_args_info(int &num_extra_args, int &num_required_extra_args) const;

    void print_err(error_t err, std::string arg, std::string extra = "");
    void print_single_option_usage(const std::shared_ptr<option> &d, const std::string &header, bool no_option_name = false) const;

    std::vector<value> get_result_value(const std::string &s) const;
    void get_result_map(const std::string &s, std::map<std::string, std::string> &r) const;

    public:
    args_parser &set_program_name(const std::string name) { program_name = name; return *this; }
    args_parser &set_flag(flag_t flag) { flags.insert(flag); return *this; }
    bool is_flag_set(flag_t flag) const { return flags.count(flag) > 0; } 
    void print_help_advice() const;
    void print_help() const;
    void print_help(std::string str) const;
    void print() const;
    void get_command_line(std::string &) const;
    bool parse();
    template <typename T>
    option &add(const char *s);
    template <typename T>
    option &add(const char *s, T v);
    option &add_flag(const char *s);
    template <typename T>
    option &add_vector(const char *s, char delim = ',', int min = 0, int max = option_vector::MAX_VEC_SIZE);
    template <typename T>
    option &add_vector(const char *s, const char *defaults, char delim = ',', int min = 0, int max = option_vector::MAX_VEC_SIZE);
    args_parser::option &add_map(const char *s, char delim1 = ':', char delim2 = '=');
    args_parser::option &add_map(const char *s, const char *def = "", char delim1 = ':', char delim2 = '=');

    args_parser &set_current_group(const std::string &g) { current_group = g; return *this; }
    args_parser &set_default_current_group() { current_group = ""; return *this; }

    option &set_caption(int n, const char *cap);

    template <typename T>
    T get(const std::string &s) const;
    template <typename T>
    void get(const std::string &s, std::vector<T> &r) const;
    void get(const std::string &s, std::map<std::string, std::string> &r) const;
    void get_unknown_args(std::vector<std::string> &r) const;

    template <typename T>
    bool parse_special(const std::string &s, T &r) const;
    template <typename T>
    bool parse_special_vec(const std::string &s, std::vector<T> &r, char delim = ',', int min = 0, int max = option_vector::MAX_VEC_SIZE) const;

    void clean_args() { argc = 0; }
#ifdef WITH_YAML_CPP    
    std::string dump() const;
    bool load(const std::string &input);
    bool load(std::istream &in);
#endif    
    bool is_option(const std::string &str) const;
    bool is_option_defaulted(const std::string &str) const;
    bool is_help_mode() const;

    error_t get_last_error(std::string &option, std::string &extra) {
        option = last_error_option;
        extra = last_error_extra;
        return last_error;
    }

    protected:
    // NOTE: see source for usage comments
    enum foreach_t { FOREACH_FIRST, FOREACH_NEXT };
    bool in_expected_args(enum foreach_t t, const std::string *&group, std::shared_ptr<option> *&arg);    
    bool in_expected_args(enum foreach_t t, const std::string *&group, const std::shared_ptr<option> *&arg) const;    
};

template <typename T> args_parser::arg_t get_arg_t();

template <typename T> T get_val(const args_parser::value &v);

template <typename T>
void vresult_to_vector(const std::vector<args_parser::value> &in, std::vector<T> &out) {
    for (size_t i = 0; i < in.size(); i++) {
        if (in[i].type != get_arg_t<T>()) {
            throw std::logic_error("args_parser: type mismatch while accessing the result");
        }
        out.push_back(get_val<T>(in[i]));
    }
}

template <typename T>
args_parser::option &args_parser::add(const char *s) {
    std::shared_ptr<option> popt = std::make_shared<args_parser::option_scalar>(*this, s, get_arg_t<T>());
    expected_args[current_group].push_back(popt);
    return *popt.get();
}

template <typename T>
args_parser::option &args_parser::add(const char *s, T v) {
    std::shared_ptr<option> popt = std::make_shared<args_parser::option_scalar>(*this, s, get_arg_t<T>(), value(v));
    expected_args[current_group].push_back(popt);
    return *popt.get();
}

template <typename T>
args_parser::option &args_parser::add_vector(const char *s, char delim, int min, int max) {
    if (max > option_vector::MAX_VEC_SIZE)
        throw std::logic_error("args_parser: maximum allowed vector size for vector argument exceeded");
    std::shared_ptr<option> popt = std::make_shared<args_parser::option_vector>(*this, s, get_arg_t<T>(), delim, min, max);
    expected_args[current_group].push_back(popt);
    return *popt.get();
}

template <typename T>
args_parser::option &args_parser::add_vector(const char *s, const char *defaults, char delim, int min, int max) {
    if (max > option_vector::MAX_VEC_SIZE)
        throw std::logic_error("args_parser: maximum allowed vector size for vector argument exceeded");
    std::shared_ptr<option> popt = std::make_shared<args_parser::option_vector>(*this, s, get_arg_t<T>(), delim, min, max, defaults); 
    expected_args[current_group].push_back(popt);
    return *popt.get();
}
template <typename T>
void args_parser::get(const std::string &s, std::vector<T> &r) const {
    std::vector<value> v = get_result_value(s);
    vresult_to_vector<T>(v, r);
}

template <typename T>
T args_parser::get(const std::string &s) const {
    std::vector<T> r;
    get<T>(s, r);
    if (r.size() != 1)
        throw std::logic_error("args_parser: get_result can't get a result: zero-sized vector returned");
    return r[0];
}

template <typename T>
bool args_parser::parse_special(const std::string &s, T &r) const {
    option_scalar d("[FREE ARG]", get_arg_t<T>());
    bool res = d.do_parse(s.c_str());
    if (res) {
        r = get_val<T>(d.get_value_as_vector()[0]);
    }
    return res;
}

template <typename T>
bool args_parser::parse_special_vec(const std::string &s, std::vector<T> &r, char delim, int min, int max) const {
    option_vector d(*this, "[FREE ARG]", get_arg_t<T>(), delim, min, max);
    bool res = d.do_parse(s.c_str());
    if (res) {
        vresult_to_vector(d.get_value_as_vector(), r);
    }
    return res;
}

YAML::Emitter &operator<< (YAML::Emitter& out, const args_parser::value &v);
void operator>> (const YAML::Node& node, args_parser::value &v);
YAML::Emitter &operator<< (YAML::Emitter& out, const args_parser::option &opt);
void operator>> (const YAML::Node& node, args_parser::option &opt);
