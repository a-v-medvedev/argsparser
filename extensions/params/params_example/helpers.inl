/*
 * Copyright (c) 2020-2022 Alexey V. Medvedev
 * This code is licensed under 3-Clause BSD License.
 * See license.txt file for details.
 */
#pragma once

namespace params {

namespace helpers {

template <typename T>
struct yaml_read_assistant {
    const YAML::Node &stream;
    yaml_read_assistant(const YAML::Node &stream_) : stream(stream_) {}
    void get_map_keys(const std::string &entry, std::vector<std::string> &keys) {
        YAML::Node node = stream;
        auto node_names = str_split(entry, '/');
        for (const auto &name : node_names) {
            if (name.empty())
                break;
            if (!node.IsMap()) {
                throw std::runtime_error("yaml_read_assistant::get_map_keys: entry is not a map");
            }
            if (!node[name]) {
                throw std::runtime_error("yaml_read_assistant::get_map_keys: entry does not exist in yaml stream");
            }
            node.reset(node[name].as<YAML::Node>());
        }
        if (!node.IsMap()) {
            throw std::runtime_error("yaml_read_assistant::get_map_keys: entry is not a map");
        }
        for(auto it = node.begin(); it != node.end(); ++it) {
            std::string k = it->first.as<std::string>();
            keys.push_back(k);
        }
    }
    
    void iterate_over_map(const std::string &entry, std::string &short_entry_name,
                          std::function<void(const std::string &k, const std::string &v)> func,
                          std::function<void(const std::string &k, const std::vector<std::string> &vec)> funcv) {
        if (entry.back() == '/') {
            throw std::runtime_error("yaml_read_assistant::iterate_over_map: yaml entry name must not end with '/'");
        }
        YAML::Node node = stream;
        auto node_names = str_split(entry, '/');
        for (const auto &name : node_names) {
            if (!node[name]) {
                throw std::runtime_error("yaml_read_assistant::iterate_over_map: entry does not exist in yaml stream");
            }
            node.reset(node[name].as<YAML::Node>());
        }
        if (!node.IsMap()) {
            throw std::runtime_error("yaml_read_assistant::iterate_over_map: entry is not a map");
        }
        for (auto it = node.begin(); it != node.end(); ++it) {
            std::string k = it->first.as<std::string>();
            if (it->second.IsSequence()) {
                auto vec = it->second.as<std::vector<std::string>>();
                funcv(k, vec);
            } else if (it->second.IsScalar()) {
                std::string v = it->second.as<std::string>();
                func(k, v);
            } else {
                throw std::runtime_error("yaml_read_assistant::iterate_over_map: wrong map structure: it may contain only scalars or sequences");
            }
        }
        short_entry_name = node_names.back();
    }
    void get_list(const std::string &entry, params::dictionary<T> &dict) {
        if (entry.back() == '/') {
            throw std::runtime_error("yaml_read_assistant::get_list: yaml entry name must not end with '/'");
        }
        params::list<T> list;
        if ("-" == T::get_family_key()) {
            list.parse_and_set_value("-", "-");
        }
        std::string short_entry_name;
        iterate_over_map(entry, short_entry_name,
            [&](const std::string &k, const std::string &v) { list.parse_and_set_value(k, v); },
            [&](const std::string &k, const std::vector<std::string> &vec) { list.parse_and_set_value(k, vec); });
        dict.add(short_entry_name, list);
    }
    void get_all_lists(const std::string &dictionary_entry, params::dictionary<T> &dict) {
        if (dictionary_entry.back() != '/') {
            throw std::runtime_error("yaml_read_assistant::get_all_lists: yaml dictionary entry name must end with '/'");
        }
        std::vector<std::string> keys;
        get_map_keys(dictionary_entry, keys);
        for (const auto &key : keys) {
            auto list_entry = dictionary_entry + key;
            get_list(list_entry, dict);
        }
    }
};

}

}
