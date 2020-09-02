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
    assert(false);
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
    assert(false);
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
    assert(find(name));
    return m.find(name)->second;
}

template <class details>
const list<details> &dictionary<details>::get(const std::string &name) const {
    assert(find(name));
    return m.find(name)->second;
}

template <class details>
list<details> dictionary<details>::get(const std::string &name, int level) const {
    const auto &nlevels = details::get_nlevels();
    if (!find(name + "_override")) {
        return get(name);
    }
    overrides_holder<details> holder(nlevels);
    holder.fill_in(get(name + "_override"));
    list<details> l = get(name);
    if (holder.find(level)) {
        auto &overrides = holder.get(level);
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
void dictionary<details>::internal_change_value_lev(const std::string &list_name, const std::string &key, 
                                           const T &v, uint16_t level, bool forced) {
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
        auto newv = oldv + ";" + p.as_string() + "@" + layer_prefix + std::to_string(level);
        target.template change_value<std::string>(key, newv, forced);
    } else {
        value p;
        p.set<T>(v);
        target.template add_value<std::string>(key, p.as_string() + "@" + layer_prefix + std::to_string(level));
    }
} 

template <class details>
template<typename T>
void dictionary<details>::change_value_lev(const std::string &list_name, const std::string &key, 
                                  const T &v, uint16_t level) {
    internal_change_value_lev(list_name, key, v, level, false);
}

template <class details>
template<typename T>
void dictionary<details>::forced_change_value_lev(const std::string &list_name, const std::string &key, 
                                         const T &v, uint16_t level) {
    internal_change_value_lev(list_name, key, v, level, true);
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
void dictionary<details>::print_list(const std::string &list_name, const std::string &header_name) const {
    const auto &family_key = details::get_family_key();
    const auto &nlevels = details::get_nlevels();
    const auto &expected_params = details::get_expected_params();
    assert(find(list_name));
    auto &l = get(list_name);
    list<details>::print_line_delimiter();
    list<details>::print_header(header_name);
    list<details>::print_line_delimiter();
    overrides_holder<details> holder(nlevels);
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
            l.print_line(e.first);
            for (int lev = 0; lev < nlevels; lev++) {
                if (holder.find(lev)) {
                    auto &per_level_list = holder.get(lev);
                    std::string ignored;
                    if (per_level_list.get_value_as_string(e.first, ignored)) {
                        per_level_list.print_line(e.first, e.first + " (" + layer_prefix + "." + std::to_string(lev) + ")");
                    }
                }
            }
        }
    }
    list<details>::print_line_delimiter();
}

}

