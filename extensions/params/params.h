/*
 * Copyright (c) 2020-2021 Alexey V. Medvedev, Boris Krasnopolsky
 * This code is licensed under Community Source License v.1 (CPL v.1) license.
 * See license.txt file for details.
 */

#pragma once

#include <limits>
#include <string>
#include <vector>
#include <map>
#include <assert.h>

typedef double float64_t; // FIXME this is actually system-dependent 

namespace params {

template <class details> class dictionary;
template <class details> class list;
template <class details> class overrides_holder;

struct value {
    enum type_t { I, F, S, B, IV, FV, SV, BV, NUL } type = NUL;
    static constexpr auto F_MAX = std::numeric_limits<float64_t>::max();
    static constexpr auto I_MAX = std::numeric_limits<uint32_t>::max();
    template <typename T> type_t get_type() const;
    static std::string get_max_possible_value(type_t);
    template <typename T> T get_max_possible_value();
    uint32_t i;
    float64_t f;
    bool b;
    std::string s;
    std::vector<uint32_t> iv;
    std::vector<float64_t> fv;
    std::vector<bool> bv;
    std::vector<std::string> sv;
    template<typename T> void set(T val);
    
    template<type_t to> void autoconv();
    void parse_and_set(value::type_t t, const std::string &value);
    void parse_and_set(value::type_t t, const std::vector<std::string> &vec);
    std::string as_string() const;
protected:
    template<typename T> T &get();
    template<typename T> const T &get() const;
    template<typename U> friend class list;
    template<typename U> friend class overrides_holder;
};

struct param_traits {
    value::type_t type;
    bool changeable;
    std::vector<std::string> matching_families;
    std::vector<std::string> minmax;
    std::vector<std::string> allowed_values;
};

using expected_params_t = std::vector<std::pair<std::string, param_traits>>;

template <class details> class dictionary;
                                       
template <class details>
class list {
    friend details;
public:
    bool omit_value_coversions_and_checks = false;
    list() {}
    list(std::initializer_list<std::string> l) {
        assert(l.size() == 2);
        set_value(*(l.begin()), *(l.begin() + 1));
    }
protected:
    expected_params_t::const_iterator is_in_expected_params(const std::string &key) const;
    std::map<std::string, value> l;
public:
    void parse_and_set_value(const std::string &key, const std::string &value);
    void parse_and_set_value(const std::string &key, const std::vector<std::string> &vec);
protected:
    value::type_t get_type(const std::string &key) const;
    template<typename T>
    void set_value(const std::string &key, const T &value);
    template<typename T>
    void set_value_if_missing(const std::string &key, const T &value);
    void override_param(const std::string &key, const value &p);
    template<typename T>
    void change_value(const std::string &key, const T &value, bool forced = false);
    template<typename T>
    bool get_value(const std::string &key, T &value) const;
    template<typename T>
    T get_value(const std::string &key) const;
public:
    void set(const std::string &key, const value &p);
    void override_params(const list<details> &other);
    template<typename T>
    void add_value(const std::string &key, const T &value);
    uint32_t get_int(const std::string &key) const { return get_value<uint32_t>(key); }
    float64_t get_float(const std::string &key) const { return get_value<float64_t>(key); }
    std::string get_string(const std::string &key) const { return get_value<std::string>(key); }
    bool get_bool(const std::string &key) const { return get_value<bool>(key); }
    std::vector<uint32_t> get_vint(const std::string &key) const { return get_value<std::vector<uint32_t>>(key); }
    std::vector<float64_t> get_vfloat(const std::string &key) const { return get_value<std::vector<float64_t>>(key); }
    std::vector<std::string> get_vstring(const std::string &key) const { return get_value<std::vector<std::string>>(key); }
    std::vector<bool> get_vbool(const std::string &key) const { return get_value<std::vector<bool>>(key); }

    bool is_value_set(const std::string &key) const;
    std::string get_value_as_string(const std::string &key) const;
    bool get_value_as_string(const std::string &key, std::string &result) const;
    template<typename T>
    bool get_minmax(const std::string &key, std::pair<T, T> &output) const;
    template<typename T>
    bool get_allowed_values(const std::string &key, std::vector<T> &output) const;
	template <typename T>
	bool is_value_allowed(const std::string &key, T val);
	bool is_value_allowed(const std::string &key, const value &p);
    const std::map<std::string, value> &get_raw_list() const { return l; }
    bool erase();
    void print(const std::string &header = "");
protected:
	template <typename T>
	bool is_allowed_vec(const std::string &key, const value &p);
    void print_line(const std::string &key, const std::string &out = "", bool omit_undefined = false) const;
	static void print_header(const std::string str, uint16_t offset = 8);
	static void print_line_delimiter();
   
public:
    friend class dictionary<details>;
    void set_default(const std::string &list_name);
};

}

#include "params_override.h"

namespace params {    

template <class details>
struct dictionary {
    std::map<std::string, list<details>> m;
    void add(const std::string &name, const list<details> &list);
    void add_map(const std::string &name, const std::map<std::string, std::string> &kvmap);
    void add_override(const std::string &name, const std::map<std::string, std::string> &kvmap);
    bool find(const std::string &name) const;
    template <typename T>
    bool find_if(const std::vector<std::pair<std::string, T>> &conditions, 
                 std::string &found_list_name) const;
    template <typename T>
    bool find_if(const std::vector<std::pair<std::string, T>> &conditions) const;
    template <typename T>
    const list<details> &get_if(const std::vector<std::pair<std::string, T>> &conditions, 
                                          std::string &found_list_name) const;
    template <typename T>
    list<details> &get_if(const std::vector<std::pair<std::string, T>> &conditions, 
                                    std::string &found_list_name);
    template <typename T>
    const list<details> &get_if(const std::vector<std::pair<std::string, T>> &conditions) const;
    template <typename T>
    list<details> &get_if(const std::vector<std::pair<std::string, T>> &conditions);
    list<details> &get(const std::string &name);
    const list<details> &get(const std::string &name) const;
    const std::string &get(size_t num) const;
    list<details> get(const std::string &name, int layer) const;
    template<typename T>
    void change_value(const std::string &list_name, const std::string &key, const T &value);
    template<typename T>
    void change_value_onlayer(const std::string &list_name, const std::string &key, const T &value, uint16_t layer);
    template<typename T>
    void forced_change_value(const std::string &list_name, const std::string &key, const T &value);
    template<typename T>
    void forced_change_value_onlayer(const std::string &list_name, const std::string &key, const T &value, 
                                     uint16_t layer);
    protected:    
    template<typename T>
    void internal_change_value_onlayer(const std::string &list_name, const std::string &key, 
                                       const T &value, uint16_t layer, bool forced);
    
    public:
    void print() const;
    void set_defaults();
    void print_list(const std::string &list_name, const std::string &header_name, bool omit_undefined = false) const;
    size_t size() const;
};

}

#include "list.inl"
#include "dict.inl" 
#include "override.inl"

