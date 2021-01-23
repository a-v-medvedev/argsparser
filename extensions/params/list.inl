/*
 * Copyright (c) 2020 Alexey V. Medvedev, Boris Krasnopolsky
 * This code is licensed under Community Source License v.1 (CPL v.1) license.
 * See license.txt file for details.
 */

#pragma once

#include <iostream>
#include <iomanip>

namespace params {

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
        assert(0 && "Unknown parameter");
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
        assert(0 && "Unknown parameter");
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
void list<details>::set(const std::string &key, const value &p) {
    const auto &expected_params = details::get_expected_params();
    if (is_in_expected_params(key) == expected_params.end()) {
        assert(0 && "Unknown parameter");
    }
    if (!omit_value_coversions_and_checks) {
        if (!is_value_allowed(key, p)) {
            assert(0 && "Not allowed parameter value");
        }
    }
    auto elem = l.find(key);
    if (elem != l.end())
        elem->second = p;
    else
        l.insert(std::pair<std::string, value>(key, p));
}

template <class details>
template<typename T>
void list<details>::set_value(const std::string &key, const T &v) {
    const auto &expected_params = details::get_expected_params();
    if (is_in_expected_params(key) == expected_params.end()) {
        assert(0 && "Unknown parameter");
    }
    value obj;
    if (!omit_value_coversions_and_checks) {
        if ((is_in_expected_params(key)->second).type != obj.get_type<T>()) {
            assert(0 && "Type mismatch on a parameter");
        } 
        if (!is_value_allowed(key, v)) {
            assert(0 && "Not allowed parameter value");
        }
    }
    obj.set<T>(v);
    auto elem = l.find(key);
    if (elem != l.end())
        elem->second = obj;
    else
        l.insert(std::pair<std::string, value>(key, obj));
}

template <class details>
template<typename T>
void list<details>::set_value_if_missing(const std::string &key, const T &v) {
    const auto &expected_params = details::get_expected_params();
    if (is_in_expected_params(key) == expected_params.end()) {
        assert(0 && "Unknown parameter");
    }
    auto elem = l.find(key);
    if (elem == l.end()) {
        value obj;
        if (!omit_value_coversions_and_checks) {
            if ((is_in_expected_params(key)->second).type != obj.get_type<T>()) {
                assert(0 && "Type mismatch on a parameter");
            }
            if (!is_value_allowed(key, v)) {
                assert(0 && "Not allowed parameter value");
            }
        }
        obj.set<T>(v);
        l.insert(std::pair<std::string, value>(key, obj));
    }
}

template <class details>
void list<details>::override_param(const std::string &key, const value &p) {
    set(key, p);
}

template <class details>
template<typename T>
void list<details>::change_value(const std::string &key, const T &v, bool forced) {
    const auto &expected_params = details::get_expected_params();
    if (is_in_expected_params(key) == expected_params.end()) {
        assert(0 && "Unknown parameter");
    }
    if (!(is_in_expected_params(key)->second).changeable) {
        if (!forced) {
            assert(0 && "Parameter cannot be changed");
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
        assert(0 && "Unknown parameter");
    }
    if (l.find(key) != l.end()) {
        assert(0 && "Parameter is already set");
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
        v = elem->second.get<T>();
    return true;
}

template <class details>
template<typename T>
T list<details>::get_value(const std::string &key) const {
    T val;
    bool r = get_value<T>(key, val);
    assert(r == true);
    return val;
}

template <class details>
bool list<details>::is_value_set(const std::string &key) const {
    return l.find(key) != l.end();
}

template <class details>
std::string list<details>::get_value_as_string(const std::string &key) const {
    auto elem = l.find(key);
    assert(elem != l.end());
    return elem->second.as_string();
}

template <class details>
bool list<details>::get_value_as_string(const std::string &key, std::string &result) const {
    auto elem = l.find(key);
    if (elem != l.end()) {
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
        assert(0 && "Unknown parameter");
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
        assert(0 && "Type mismatch in min/max values");
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
        assert(0 && "Unknown parameter");
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
        case value::I: return is_value_allowed(key, p.get<uint16_t>());
        case value::F: return is_value_allowed(key, p.get<float32_t>());
        case value::B: return true;
        case value::NUL: return false;
        case value::S: return is_value_allowed(key, p.get<std::string>());
        case value::IV: return is_allowed_vec<uint16_t>(key, p);
        case value::FV: return is_allowed_vec<float32_t>(key, p);
        case value::BV: return true;
        case value::SV: return is_allowed_vec<std::string>(key, p);
    }
    assert(0 && "unknown value type");
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
void list<details>::print_line(const std::string &key, const std::string &out) const {
    const auto &family_key = details::get_family_key();
    if (key == family_key && key == "-")
        return;
    std::string str_key = (out == "" ? key : out);
    std::string str_value = "[UNDEFINED]";
    get_value_as_string(key, str_value);
    std::stringstream ss;
    ss << std::setfill(' ') << "| " << std::left
       << std::setw(36) << str_key << " | " << std::right << std::setw(25)
       << str_value << " |" << std::endl;
    details::print_stream(ss);
}

template <class details>
void list<details>::print_header(const std::string str, uint16_t offset) {
    std::stringstream ss;
    ss << std::setfill(' ') << std::left << "| "
       << std::setw(offset) << "" << std::setw(64-offset) << str << " |"
       << std::endl;
    details::print_stream(ss);
}

template <class details>
void list<details>::print_line_delimiter() {
    std::stringstream ss;
    ss << std::right << std::setfill('-') << "|" 
       << std::setw(67) << "|" << std::endl;
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

