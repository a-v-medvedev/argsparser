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
bool overrides_holder<details>::find(size_t level) 
{ 
    return (per_level_lists.find(level) != per_level_lists.end()); 
}

template <class details>
list<details> &overrides_holder<details>::get(size_t level) 
{ 
    assert(find(level)); 
    return per_level_lists[level]; 
}

template <class details>
void overrides_holder<details>::get_start_end_lev(const std::string &s, size_t &start, size_t &end) {
    // extract start and end level from s 
    // input form is: levX[-Y], where X is start level, Y is end level
    const std::string &layer_prefix = details::get_layer_prefix();
    std::regex lev(layer_prefix);
    std::stringstream ss;
    ss << std::regex_replace(s, lev, "");
    auto bounds = helpers::str_split(ss.str(), '-');
    if (bounds.size() == 1) {
        start = end = std::stoi(bounds[0]);
    } else {
        start = std::stoi(bounds[0]);
        if (bounds[1] == "E" || bounds[1] == "")
            end = maxlevel;
        else 
            end = std::stoi(bounds[1]);
    }
}

template <class details>
void overrides_holder<details>::fill_in(const list<details> &in) {
    // for each in "in" ->  { key, val }
    //     split val by @ -> { v0, v1 }
    //     extract start and end level from v1, check maxlev value for end
    //     for (lev=start..end)
    //         lists[lev].add(key, v0)
    for (auto &i : in.get_raw_list()) {
        auto &key = i.first;
        auto &value = i.second.template get<std::string>();
        auto parts = helpers::str_split(value, ';');
        for (const auto &part : parts) {
            auto v = helpers::str_split(part, '@');
            assert(v.size() == 2);
            size_t start, end;
            get_start_end_lev(v[1], start, end);
            end = std::min(end, maxlevel);
            start = std::min(start, maxlevel);
            assert(start <= end);
            for (size_t lev = start; lev <= end; lev++) {
                per_level_lists[lev].parse_and_set_value(key, v[0]);
            }
        }
    }
}

template <class details>
template <typename T>
void overrides_holder<details>::apply_for_each_level(const std::string &param_name, 
                                            std::function<void (T, uint8_t)> f) 
{
    for (uint8_t nl = 0; nl < maxlevel; nl++) {
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

