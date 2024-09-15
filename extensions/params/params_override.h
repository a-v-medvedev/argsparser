/*
 * Copyright (c) 2020-2024 Alexey V. Medvedev, Boris Krasnopolsky
 * This code is licensed under 3-Clause BSD license.
 * See license.txt file for details.
 */

#pragma once

#include <functional>
#include <regex>

namespace params {

template <class details> class list;

template <class details>
struct overrides_holder {
    std::map<size_t, list<details>> per_layer_lists;
    const size_t nlayers;
    void get_start_end_layer(const std::string &s, size_t &start, size_t &end);
    overrides_holder(size_t _nlayers) : nlayers(_nlayers) {}
    void fill_in(const list<details> &in);
    bool find(size_t layer);
    list<details> &get(size_t layer);
    template <typename T>
    void apply_for_each_layer(const std::string &param_name, std::function<void (T, uint8_t)> f);
};

}

