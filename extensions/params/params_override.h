#pragma once

#include <functional>
#include <regex>

namespace params {

template <class details> class list;

template <class details>
struct overrides_holder {
    std::map<size_t, list<details>> per_level_lists;
    const size_t maxlevel;
    void get_start_end_lev(const std::string &s, size_t &start, size_t &end);
    overrides_holder(size_t _maxlevel) : maxlevel(_maxlevel) {}
    void fill_in(const list<details> &in);
    bool find(size_t level);
    list<details> &get(size_t level);
    template <typename T>
    void apply_for_each_level(const std::string &param_name, std::function<void (T, uint8_t)> f);
};

}

