/*
 * Copyright (c) 2018-2021 Alexey V. Medvedev
 * This code is an extension of the parts of Intel(R) MPI Benchmarks project.
 * It keeps the same Community Source License (CPL) license.
 */

#include "argsparser.h"
#include "argsparser_iface.h"

using namespace parser_iface;

namespace parser_iface {

    parser::parser()
    {
        ptr = (void *)(new args_parser);
    }

    parser::parser(int argc, char **argv)
    {
        ptr = (void *)(new args_parser(argc, argv, "--", '='));
    }

    parser::~parser()
    {
        delete (args_parser *)ptr;
    }

    std::shared_ptr<parser> parser_create()
    {   
        std::shared_ptr<parser> p(new parser());
        return p;
    }

    std::shared_ptr<parser> parser_create(int argc, char **argv)
    {   
        std::shared_ptr<parser> p(new parser(argc, argv));
        return p;
    }

    bool parser::parse() { return ((args_parser *)ptr)->parse(); }
    bool parser::is_option_defaulted(const std::string &s) { return ((args_parser *)ptr)->is_option_defaulted(s); }

    void parser::add_bool(const std::string &s) { ((args_parser *)ptr)->add<bool>(s.c_str()); }
    void parser::add_flag(const std::string &s) { ((args_parser *)ptr)->add_flag(s.c_str()); }
    void parser::add_bool(const std::string &s, bool v) { ((args_parser *)ptr)->add<bool>(s.c_str(), v); }
    bool parser::get_bool(const std::string &s) { return ((args_parser *)ptr)->get<bool>(s); }
    bool parser::get_flag(const std::string &s) { return ((args_parser *)ptr)->get<bool>(s); }
    void parser::add_string(const std::string &s) { ((args_parser *)ptr)->add<std::string>(s.c_str()); }
    void parser::add_string(const std::string &s, std::string v) { ((args_parser *)ptr)->add<std::string>(s.c_str(), v); }
    std::string parser::get_string(const std::string &s) { return ((args_parser *)ptr)->get<std::string>(s); }
    void parser::add_int(const std::string &s) { ((args_parser *)ptr)->add<int>(s.c_str()); }
    void parser::add_int(const std::string &s, int v) { ((args_parser *)ptr)->add<int>(s.c_str(), v); }
    int parser::get_int(const std::string &s) { return ((args_parser *)ptr)->get<int>(s); }
    void parser::add_float(const std::string &s) { ((args_parser *)ptr)->add<float>(s.c_str()); }
    void parser::add_float(const std::string &s, float v) { ((args_parser *)ptr)->add<float>(s.c_str(), v); }
    float parser::get_float(const std::string &s) { return ((args_parser *)ptr)->get<float>(s); }

    void parser::add_bool_vector(const std::string &s, char delim, int min, int max) { ((args_parser *)ptr)->add_vector<bool>(s.c_str(), delim, min, max); }
    std::vector<bool> parser::get_bool_vector(const std::string &s) { std::vector<bool> res; ((args_parser *)ptr)->get<bool>(s, res); return res; }
    void parser::add_string_vector(const std::string &s, char delim, int min, int max) { ((args_parser *)ptr)->add_vector<std::string>(s.c_str(), delim, min, max); }
    std::vector<std::string> parser::get_string_vector(const std::string &s) { std::vector<std::string> res; ((args_parser *)ptr)->get<std::string>(s, res); return res; }
    void parser::add_int_vector(const std::string &s, char delim, int min, int max) { ((args_parser *)ptr)->add_vector<int>(s.c_str(), delim, min, max); }
    std::vector<int> parser::get_int_vector(const std::string &s) { std::vector<int> res; ((args_parser *)ptr)->get<int>(s, res); return res; }
    void parser::add_float_vector(const std::string &s, char delim, int min, int max) { ((args_parser *)ptr)->add_vector<float>(s.c_str(), delim, min, max); }
    std::vector<float> parser::get_float_vector(const std::string &s) { std::vector<float> res; ((args_parser *)ptr)->get<float>(s, res); return res; }

    void parser::add_bool_vector(const std::string &s, const std::string &def, char delim, int min, int max) { ((args_parser *)ptr)->add_vector<bool>(s.c_str(), def.c_str(), delim, min, max); }
    void parser::add_string_vector(const std::string &s, const std::string &def, char delim, int min, int max) { ((args_parser *)ptr)->add_vector<std::string>(s.c_str(), def.c_str(), delim, min, max); }
    void parser::add_int_vector(const std::string &s, const std::string &def, char delim, int min, int max) { ((args_parser *)ptr)->add_vector<int>(s.c_str(), def.c_str(), delim, min, max); }
    void parser::add_float_vector(const std::string &s, const std::string &def, char delim, int min, int max) { ((args_parser *)ptr)->add_vector<float>(s.c_str(), def.c_str(), delim, min, max); }


    bool parser::load(std::istream &st) { return ((args_parser *)ptr)->load(st); }
    std::string parser::dump() { return ((args_parser *)ptr)->dump(); }

}

