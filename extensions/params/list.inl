/*
 * Copyright (c) 2020-2021 Alexey V. Medvedev, Boris Krasnopolsky
 * This code is licensed under 3-Clause BSD license.
 * See license.txt file for details.
 */

#pragma once

#ifndef ARGPARSER_PARAMS_TABLEWIDTH
#define ARGPARSER_PARAMS_TABLEWIDTH 69
#define ARGPARSER_PARAMS_TABLEWIDTH_1STCOLUMN 36
#endif

#include <iostream>
#include <iomanip>
#include <stdexcept>

namespace params {

namespace helpers {

static inline std::vector<std::string> str_split(const std::string &s, char delimiter)
{
   std::vector<std::string> result;
   std::string token;
   std::istringstream token_stream(s);
   while (std::getline(token_stream, token, delimiter)) {
      result.push_back(token);
   }
   return result;
}

}
    
}

namespace params {

template <class details>
void list<details>::init(const std::string &key, const std::string &value) {
    const auto &expected_params = details::get_expected_params();
	auto item_it = is_in_expected_params(key);
	if (item_it == expected_params.end()) {
		throw std::runtime_error(std::string("params: list: unknown parameter: ") + key);
	}
    if (item_it->second.is_vector()) {
        auto vvalues = helpers::str_split(value, ',');
		parse_and_set_value(key, vvalues);
    } else {
        parse_and_set_value(key, value);
    }
}

template <class details>
typename expected_params_t::const_iterator 
list<details>::is_in_expected_params(const std::string &key) const {
    const auto &expected_params = details::get_expected_params();
    return std::find_if(expected_params.cbegin(), 
                        expected_params.cend(), 
                        [key](const expected_params_t::value_type &i) { return i.first == key; } );
}

template <class details>
void list<details>::parse_and_set_value(const std::string &key, const std::string &v) {
    const auto &expected_params = details::get_expected_params();
    if (is_in_expected_params(key) == expected_params.end()) {
        throw std::runtime_error(std::string("params: parse_and_set_value: unknown parameter: ") + key);
    }
    value::type_t t;
    if (omit_value_coversions_and_checks) {
        t = value::type_t::S;
    } else {
        t = (is_in_expected_params(key)->second).type;
    }
    value obj;
    obj.parse_and_set(t, v);
    set(key, obj);
}

template <class details>
void list<details>::parse_and_set_value(const std::string &key, const std::vector<std::string> &vec) {
    const auto &expected_params = details::get_expected_params();
    if (is_in_expected_params(key) == expected_params.end()) {
        throw std::runtime_error(std::string("params: parse_and_set_value: unknown parameter: ") + key);
    }
    value::type_t t;
    if (omit_value_coversions_and_checks) {
        t = value::type_t::S;
    } else {
        t = (is_in_expected_params(key)->second).type;
    }
    value obj;
    obj.parse_and_set(t, vec);
    set(key, obj);
}

template <class details>
value::type_t list<details>::get_type(const std::string &key) const {
    auto elem = l.find(key);
    if (elem == l.end())
        return value::type_t::NUL;
    else
        return elem->second.type;
}

template <class details>
void list<details>::set_unsafe(const std::string &key, const value &p) {
    auto elem = l.find(key);
    if (elem != l.end())
        elem->second = p;
    else
        l.insert(std::pair<std::string, value>(key, p));
}

template <class details>
void list<details>::set(const std::string &key, const value &p) {
    const auto &expected_params = details::get_expected_params();
    if (is_in_expected_params(key) == expected_params.end()) {
        throw std::runtime_error(std::string("params: set: unknown parameter: ") + key);
    }
    if (!omit_value_coversions_and_checks) {
        if (!is_value_allowed(key, p)) {
            throw std::runtime_error(std::string("params: set_value: not allowed parameter value for key: ") + key);
        }
    }
    set_unsafe(key, p);
}

template <class details>
template<typename T>
void list<details>::set_value(const std::string &key, const T &v) {
    const auto &expected_params = details::get_expected_params();
    if (is_in_expected_params(key) == expected_params.end()) {
        throw std::runtime_error(std::string("params: set_value: unknown parameter: ") + key);
    }
    value obj;
    if (!omit_value_coversions_and_checks) {
        if ((is_in_expected_params(key)->second).type != obj.get_type<T>()) {
            throw std::runtime_error(std::string("params: set_value: type mismatch on a parameter: ") + key);
        } 
        if (!is_value_allowed(key, v)) {
            throw std::runtime_error(std::string("params: set_value: not allowed parameter value for key: ") + key);
        }
    }
    obj.set<T>(v);
    set_unsafe(key, obj);
}

template <class details>
template<typename T>
void list<details>::set_value_if_missing(const std::string &key, const T &v) {
    const auto &expected_params = details::get_expected_params();
    if (is_in_expected_params(key) == expected_params.end()) {
        throw std::runtime_error(std::string("params: set_value_if_missing: unknown parameter: ") + key);
    }
    auto elem = l.find(key);
    if (elem == l.end()) {
        value obj;
        if (!omit_value_coversions_and_checks) {
            if ((is_in_expected_params(key)->second).type != obj.get_type<T>()) {
                throw std::runtime_error(std::string("params: set_value: type mismatch on a parameter: ") + key);
            }
            if (!is_value_allowed(key, v)) {
                throw std::runtime_error(std::string("params: set_value: not allowed value for parameter: ") + key);
            }
        }
        obj.set<T>(v);
        l.insert(std::pair<std::string, value>(key, obj));
    }
}

template <class details>
void list<details>::override_param(const std::string &key, const value &p) {
    set_unsafe(key, p); // NOTE: we assume that all value checks are done before
}

template <class details>
template<typename T>
void list<details>::change_value(const std::string &key, const T &v, bool forced) {
    const auto &expected_params = details::get_expected_params();
    if (is_in_expected_params(key) == expected_params.end()) {
        throw std::runtime_error(std::string("params: set_value_if_missing: unknown parameter: ") + key);
    }
    if (!(is_in_expected_params(key)->second).changeable) {
        if (!forced) {
            throw std::runtime_error(std::string("params: change_value: parameter cannot be changed: ") + key);
        }
    }
    set_value(key, v);
}

template <class details>
void list<details>::override_params(const list &other) {
    for (auto &i : other.l) {
        override_param(i.first, i.second);
    }
}

template <class details>
template<typename T>
void list<details>::add_value(const std::string &key, const T &v) {
    const auto &expected_params = details::get_expected_params();
    if (is_in_expected_params(key) == expected_params.end()) {
        throw std::runtime_error(std::string("params: add_value: unknown parameter: ") + key);
    }
    if (l.find(key) != l.end()) {
        throw std::runtime_error(std::string("params: add_value: parameter is already set: ") + key);
    }
    set_value(key, v);
}

template <class details>
template<typename T>
bool list<details>::get_value(const std::string &key, T &v) const {
    auto elem = l.find(key);
    if (elem == l.end())
        return false;
    else
        v = elem->second.template get<T>();
    return true;
}

template <class details>
template<typename T>
T list<details>::get_value(const std::string &key) const {
    T val;
    bool r = get_value<T>(key, val);
    if (!r)
        throw std::runtime_error(std::string("params: get_value: unknown parameter or parameter is not set: ") + key);
    return val;
}

template <class details>
bool list<details>::is_value_set(const std::string &key) const {
    return l.find(key) != l.end();
}

template <class details>
std::string list<details>::get_value_as_string(const std::string &key) const {
    auto elem = l.find(key);
    if (elem == l.end()) {
        throw std::runtime_error(std::string("params: get_value_as_string: unknown parameter or parameter is not set: ") + key);
    }
	auto it = print_converters.find(key);
	if (it != print_converters.end()) {
		if (print_converters.at(key)) {
			return print_converters.at(key)(elem->second);
		}
	}
    return elem->second.as_string();
}

template <class details>
bool list<details>::get_value_as_string(const std::string &key, std::string &result) const {
    auto elem = l.find(key);
    if (elem != l.end()) {
        auto it = print_converters.find(key);
        if (it != print_converters.end()) {
            if (print_converters.at(key)) {
                result = print_converters.at(key)(elem->second);
                return true;
            }
        }
        result = elem->second.as_string();
        return true;
    }
    return false;
}

template <class details>
template<typename T>
bool list<details>::get_minmax(const std::string &key, std::pair<T, T> &output) const {
    const auto &expected_params = details::get_expected_params();
    auto it = is_in_expected_params(key);
    if (it == expected_params.end()) {
        throw std::runtime_error(std::string("params: get_minmax: unknown parameter: ") + key);
    }
    if ((it->second).minmax.size() != 2)
        return false;
    const std::string &s1 = (it->second).minmax[0];
    const std::string &s2 = (it->second).minmax[1];
    if (s1 == "" || s2 == "") {
        return false;
    }
    value p1, p2;
    if (p1.get_type<T>() != (it->second).type) {
        throw std::runtime_error(std::string("params: get_minmax: type mismatch in min/max values: ") + key);
    }
    p1.parse_and_set((it->second).type, s1); 
    p2.parse_and_set((it->second).type, s2); 
    output = std::pair<T, T>(p1.get<T>(), p2.get<T>());
    return true;
}

template <class details>
template<typename T>
bool list<details>::get_allowed_values(const std::string &key, std::vector<T> &output) const {
    const auto &expected_params = details::get_expected_params();
    auto it = is_in_expected_params(key);
    if (it == expected_params.end()) {
        throw std::runtime_error(std::string("params: get_allowed_values: unknown parameter: ") + key);
    }
    if (!(it->second).allowed_values.size())
        return false;
    for (auto strval : (it->second).allowed_values) {
        value p;
        p.parse_and_set((it->second).type, strval);
        output.push_back(p.get<T>());
    }
    return true;
}

template <class details>
template <typename T>
bool list<details>::is_value_allowed(const std::string &key, T val) {
    std::vector<T> allowed;
    if (get_allowed_values(key, allowed)) {
        return std::find(allowed.begin(), allowed.end(), val) != allowed.end();
    }
    std::pair<T, T> minmax;
    if (get_minmax(key, minmax)) {
        return val >= minmax.first && val <= minmax.second;
    }
    return true;
}

template <class details>
template <typename T>
bool list<details>::is_allowed_vec(const std::string &key, const value &p) {
  const auto &v = p.get<std::vector<T>>(); 
  bool res = true; 
  for (const auto &x : v) { 
      res = res && is_value_allowed(key, x); 
  } 
  return res;
}

template <class details>
bool list<details>::is_value_allowed(const std::string &key, const value &p) {
    auto t = p.type;
    switch (t) {
        case value::I: return is_value_allowed(key, p.get<uint32_t>());
        case value::F: return is_value_allowed(key, p.get<float64_t>());
        case value::B: return true;
        case value::NUL: return false;
        case value::S: return is_value_allowed(key, p.get<std::string>());
        case value::IV: return is_allowed_vec<uint32_t>(key, p);
        case value::FV: return is_allowed_vec<float64_t>(key, p);
        case value::BV: return true;
        case value::SV: return is_allowed_vec<std::string>(key, p);
    }
    throw std::runtime_error(std::string("params: is_value_allowed: unknown value type: ") + key);
    return false;
}

template <class details>
bool list<details>::erase() {
    l.erase(l.begin(), l.end());
    return true;
}

template <class details>
void list<details>::print(const std::string &header) {
    if (header != "") {
        print_line_delimiter();
        print_header(header);
        print_line_delimiter();
    }
    for (auto &i : l) {
        print_line(i.first);
    }
}

template <class details>
void list<details>::add_print_converter(const std::string &key, std::function<std::string(const value &)> func) {
    print_converters[key] = func;
}

template <class details>
void list<details>::remove_print_converter(const std::string &key) {
    print_converters[key] = nullptr;
}

template <class details>
void list<details>::print_line(const std::string &key, const std::string &out, bool omit_undefined) const {
    const auto &family_key = details::get_family_key();
    if (key == family_key && key == "-")
        return;
    std::string str_key = (out == "" ? key : out);
    std::string str_value = "[UNDEFINED]";
    get_value_as_string(key, str_value);
    if (omit_undefined && str_value == "[UNDEFINED]")
        return;
    std::stringstream ss;
    ss << std::setfill(' ') << "| " << std::left
       << std::setw(ARGPARSER_PARAMS_TABLEWIDTH_1STCOLUMN) 
       << str_key << " | " 
       << std::right << std::setw(ARGPARSER_PARAMS_TABLEWIDTH - ARGPARSER_PARAMS_TABLEWIDTH_1STCOLUMN - 8)
       << str_value << " |" << std::endl;
    details::print_stream(ss);
}

template <class details>
void list<details>::print_header(const std::string str, uint16_t offset) {
    std::stringstream ss;
    ss << std::setfill(' ') << std::left << "| "
       << std::setw(offset) << "" << std::setw(ARGPARSER_PARAMS_TABLEWIDTH - 5 - offset) << str << " |"
       << std::endl;
    details::print_stream(ss);
}

template <class details>
void list<details>::print_line_delimiter() {
    std::stringstream ss;
    ss << std::right << std::setfill('-') << "|" 
       << std::setw(ARGPARSER_PARAMS_TABLEWIDTH - 2) << "|" << std::endl;
    details::print_stream(ss);
}

template <class details>
void list<details>::set_default(const std::string &list_name) {
    const auto &family_key = details::get_family_key();
    if (omit_value_coversions_and_checks)
        return;
    details::set_family_defaults(*this, get_value<std::string>(family_key), list_name);
}


}

