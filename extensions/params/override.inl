/*
 * Copyright (c) 2020-2021 Alexey V. Medvedev, Boris Krasnopolsky
 * This code is licensed under Community Source License v.1 (CPL v.1) license.
 * See license.txt file for details.
 */

#pragma once

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

template <class details>
bool overrides_holder<details>::find(size_t layer) 
{ 
    return (per_layer_lists.find(layer) != per_layer_lists.end()); 
}

template <class details>
list<details> &overrides_holder<details>::get(size_t layer) 
{ 
    if (!find(layer)) {
        throw std::runtime_error(std::string("params: overrides_holder: find: not found layer: ") + std::to_string(layer));
    }
    return per_layer_lists[layer]; 
}

template <class details>
void overrides_holder<details>::get_start_end_layer(const std::string &s, size_t &start, size_t &end) {
    // extract start and end layer from s 
    // input form is: layerX[-Y], where X is start layer, Y is end layer
    const std::string &layer_prefix = details::get_layer_prefix();
    std::regex layer(layer_prefix);
    std::stringstream ss;
    ss << std::regex_replace(s, layer, "");
    auto bounds = helpers::str_split(ss.str(), '-');
    if (bounds.size() == 1) {
        start = end = std::stoi(bounds[0]);
    } else {
        start = std::stoi(bounds[0]);
        if (bounds[1] == "E" || bounds[1] == "")
            end = nlayers;
        else 
            end = std::stoi(bounds[1]);
    }
}

template <class details>
void overrides_holder<details>::fill_in(const list<details> &in) {
    // for each in "in" ->  { key, val }
    //     split val by @ -> { v0, v1 }
    //     extract start and end layer from v1, check maxlayer value for end
    //     for (layer=start..end)
    //         lists[layer].add(key, v0)
    for (auto &i : in.get_raw_list()) {
        auto &key = i.first;
        auto &value = i.second.template get<std::string>();
        auto parts = helpers::str_split(value, ';');
        for (const auto &part : parts) {
            auto v = helpers::str_split(part, '@');
            if (v.size() != 2) {
                throw std::runtime_error(std::string("params: overrides_holder: fill_in: syntax error in part: ") + part);
            }
            size_t start, end;
            get_start_end_layer(v[1], start, end);
            end = std::min(end, nlayers);
            start = std::min(start, nlayers);
            if (start > end) {
                throw std::runtime_error(std::string("params: overrides_holder: fill_in: syntax error in part: ") + part);
            }
            for (size_t layer = start; layer <= end; layer++) {
                per_layer_lists[layer].parse_and_set_value(key, v[0]);
            }
        }
    }
}

template <class details>
template <typename T>
void overrides_holder<details>::apply_for_each_layer(const std::string &param_name, 
                                            std::function<void (T, uint8_t)> f) 
{
    for (uint8_t nl = 0; nl < nlayers; nl++) {
        if (find(nl)) {
            auto &over_list = get(nl);
            T value;
            if (over_list.template get_value<T>(param_name, value)) {
                f(value, nl);
            }
        }
    }
}

}

