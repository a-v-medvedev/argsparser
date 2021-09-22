/*
 * Copyright (c) 2020-2021 Alexey V. Medvedev, Boris Krasnopolsky
 * This code is licensed under Community Source License v.1 (CPL v.1) license.
 * See license.txt file for details.
 */

#pragma once

namespace params {

template <class details>
void dictionary<details>::add(const std::string &name, const list<details> &l) {
    typedef std::pair<std::string, list<details>> item;
    m.insert(item(name, l));
}

template <class details>
void dictionary<details>::add_map(const std::string &name, const std::map<std::string, std::string> &kvmap) {
    list<details> l;
    for (auto &kv : kvmap) {
        l.parse_and_set_value(kv.first, kv.second);
    }
    typedef std::pair<std::string, list<details>> item;
    m.insert(item(name, l));
}

template <class details>
void dictionary<details>::add_override(const std::string &name, const std::map<std::string, std::string> &kvmap) {
    list<details> l;
    l.omit_value_coversions_and_checks = true;
    for (auto &kv : kvmap) {
        l.set_value(kv.first, kv.second);
    }
    typedef std::pair<std::string, list<details>> item;
    m.insert(item(name + "_override", l));
}

template <class details>
bool dictionary<details>::find(const std::string &name) const {
    return m.find(name) != m.end();
}

template <class details>
template <typename T>
bool dictionary<details>::find_if(const std::vector<std::pair<std::string, T>> &conditions, 
                                  std::string &found_list_name) const {
    for (auto &i : m) {
        bool match = false;
        for (auto &c : conditions) {
            T val;
            if (i.second.template get_value<T>(c.first, val))
                if (val == c.second) {
                    match = true;
                    found_list_name = i.first;
                }
        }
        if (match)
            return true;
    }
    return false;
}

template <class details>
template <typename T>
bool dictionary<details>::find_if(const std::vector<std::pair<std::string, T>> &conditions) const {
    std::string ignored_value;
    return find_if(conditions, ignored_value);
}

template <class details>
template <typename T>
const list<details> &dictionary<details>::get_if(const std::vector<std::pair<std::string, T>> &conditions, 
                               std::string &found_list_name) const {
    for (auto &i : m) {
        // In fact, we do logical OR for all conditions. For a more complex logic, 
        // lambda arg can be implemented
        for (auto &c : conditions) {
            T val;
            if (i.second.template get_value<T>(c.first, val))
                if (val == c.second) { 
                    found_list_name = i.first;
                    return i.second;
            }
        }
    }
    throw std::runtime_error("params: get_if: list not found for given conditions");
    return *(new list<details>); // never can be reached
}

template <class details>
template <typename T>
list<details> &dictionary<details>::get_if(const std::vector<std::pair<std::string, T>> &conditions, 
                         std::string &found_list_name) {
    for (auto &i : m) {
        // In fact, we do logical OR for all conditions. For a more complex logic, 
        // lambda arg can be implemented
        for (auto &c : conditions) {
            T val;
            if (i.second.template get_value<T>(c.first, val))
                if (val == c.second) { 
                    found_list_name = i.first;
                    return i.second;
            }
        }
    }
    throw std::runtime_error("params: get_if: list not found for given conditions");
    return *(new list<details>); // never can be reached
}

template <class details>
template <typename T>
const list<details> &dictionary<details>::get_if(const std::vector<std::pair<std::string, T>> &conditions) const {
    std::string ignored_value;
    return get_if(conditions, ignored_value);
}

template <class details>
template <typename T>
list<details> &dictionary<details>::get_if(const std::vector<std::pair<std::string, T>> &conditions) {
    std::string ignored_value;
    return get_if(conditions, ignored_value);
}

template <class details>
list<details> &dictionary<details>::get(const std::string &name) {
    if (!find(name)) {
        throw std::runtime_error(std::string("params: get: list not found: ") + name);
    }
    return m.find(name)->second;
}

template <class details>
const list<details> &dictionary<details>::get(const std::string &name) const {
    if (!find(name)) {
        throw std::runtime_error(std::string("params: get: list not found: ") + name);
    }
    return m.find(name)->second;
}

template <class details>
const std::string &dictionary<details>::get(size_t num) const {
    if (num >= size()) {
        throw std::runtime_error(std::string("params: get: index out of bounds: ") + std::to_string(num));
    }
    auto it = m.begin();
    std::advance(it, num);
    return it->first;
}

template <class details>
list<details> dictionary<details>::get(const std::string &name, int layer) const {
    const auto &nlayers = details::get_nlayers();
    if (!find(name + "_override")) {
        return get(name);
    }
    overrides_holder<details> holder(nlayers);
    holder.fill_in(get(name + "_override"));
    list<details> l = get(name);
    if (holder.find(layer)) {
        auto &overrides = holder.get(layer);
        l.override_params(overrides);
    }
    return l;
}

template <class details>
template<typename T>
void dictionary<details>::change_value(const std::string &list_name, const std::string &key, const T &value) {
    auto &l = get(list_name);
    l.change_value(key, value, false);
} 

template <class details>
template<typename T>
void dictionary<details>::forced_change_value(const std::string &list_name, const std::string &key, 
                                     const T &value) {
    auto &l = get(list_name);
    l.change_value(key, value, true);
} 

template <class details>
template<typename T>
void dictionary<details>::internal_change_value_onlayer(const std::string &list_name, const std::string &key, 
                                           const T &v, uint16_t layer, bool forced) {
    const std::string &layer_prefix = details::get_layer_prefix();
    std::string ol = list_name + "_override";
    if (!find(ol)) {
        add(ol, list<details> {});
    }
    auto &target = get(ol);
    std::string oldv;
    if (target.get_value(key, oldv)) {
        value p;
        p.set<T>(v);
        auto newv = oldv + ";" + p.as_string() + "@" + layer_prefix + std::to_string(layer);
        target.template change_value<std::string>(key, newv, forced);
    } else {
        value p;
        p.set<T>(v);
        target.template add_value<std::string>(key, p.as_string() + "@" + layer_prefix + std::to_string(layer));
    }
} 

template <class details>
template<typename T>
void dictionary<details>::change_value_onlayer(const std::string &list_name, const std::string &key, 
                                  const T &v, uint16_t layer) {
    internal_change_value_onlayer(list_name, key, v, layer, false);
}

template <class details>
template<typename T>
void dictionary<details>::forced_change_value_onlayer(const std::string &list_name, const std::string &key, 
                                         const T &v, uint16_t layer) {
    internal_change_value_onlayer(list_name, key, v, layer, true);
}

template <class details>
void dictionary<details>::set_defaults() {
    details::set_dictionary_defaults(*this);
    for (auto &i : m) {
        i.second.set_default(i.first);
    }
}

template <class details>
void dictionary<details>::print() const {
    details::print_table(*this);
}

template <class details>
void dictionary<details>::print_list(const std::string &list_name, const std::string &header_name = "", bool omit_undefined) const {
    const auto &family_key = details::get_family_key();
    const auto &nlayers = details::get_nlayers();
    const auto &expected_params = details::get_expected_params();
    if (!find(list_name)) {
        throw std::runtime_error(std::string("params: print_list: not found list: ") + list_name);
    }
    auto &l = get(list_name);
    if (header_name.size()) {
        list<details>::print_line_delimiter();
        list<details>::print_header(header_name);
    }
    list<details>::print_line_delimiter();
    overrides_holder<details> holder(nlayers);
    if (find(list_name + "_override")) {
        holder.fill_in(get(list_name + "_override"));
    }
    const std::string &layer_prefix = details::get_layer_prefix();    
    auto this_family = l.template get_value<std::string>(family_key);
    for (auto &e : expected_params) {
        auto &m = e.second.matching_families;
        bool match = true;
        if (m.size()) {
            for (auto &family : m) {
                if (std::regex_match(family, std::regex("^!.*"))) {
                    if (family == "!" + this_family) {
                        match = false;
                        break;
                    }
                } else if (family == this_family) {
                    match = true;
                    break;
                } else {
                    match = false;
                }
            }
        }
        if (match) {
            l.print_line(e.first, "", omit_undefined);
            for (int layer = 0; layer < nlayers; layer++) {
                if (holder.find(layer)) {
                    auto &per_layer_list = holder.get(layer);
                    std::string ignored;
                    if (per_layer_list.get_value_as_string(e.first, ignored)) {
                        per_layer_list.print_line(e.first, e.first + " (" + layer_prefix + "." + std::to_string(layer) + ")");
                    }
                }
            }
        }
    }
    list<details>::print_line_delimiter();
}

template <typename details>
size_t dictionary<details>::size() const {
    return m.size();
}

}

