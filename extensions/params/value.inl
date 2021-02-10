/*
 * Copyright (c) 2020 Alexey V. Medvedev, Boris Krasnopolsky
 * This code is licensed under Community Source License v.1 (CPL v.1) license.
 * See license.txt file for details.
 */

#pragma once

namespace params {

template<> value::type_t value::get_type<uint16_t>() const { return value::type_t::I; }
template<> value::type_t value::get_type<float32_t>() const { return value::type_t::F; }
template<> value::type_t value::get_type<bool>() const { return value::type_t::B; }
template<> value::type_t value::get_type<std::string>() const { return value::type_t::S; }
template<> value::type_t value::get_type<std::vector<uint16_t>>() const { return value::type_t::IV; }
template<> value::type_t value::get_type<std::vector<float32_t>>() const { return value::type_t::FV; }
template<> value::type_t value::get_type<std::vector<bool>>() const { return value::type_t::BV; }
template<> value::type_t value::get_type<std::vector<std::string>>() const { return value::type_t::SV; }


template<> void value::autoconv<value::I>() {
    switch (type) {
        case I: break;
        case F: i = (uint16_t)f; type = I; break;
        case S: assert(0 && "Autoconvertion of a parameter from string requested"); break;
        case B: i = (b ? 1 : 0); break;
        case NUL: break;
        case IV: assert(0); break;
        case FV: assert(0); break;
        case SV: assert(0); break;
        case BV: assert(0); break;
        // TODO all vectors
    }
}

template<> void value::autoconv<value::F>() {
    switch (type) {
        case I: f = (float32_t)i; type = F; break;
        case F: break;
        case S: assert(0 && "Autoconvertion of a parameter from string requested"); break;
        case B: f = (b ? 1.0f : 0.0f); break;
        case NUL: break;
        case IV: assert(0); break;
        case FV: assert(0); break;
        case SV: assert(0); break;
        case BV: assert(0); break;
        // TODO all vectors
    }
}

static inline std::string float_to_string(float32_t x) {
    std::ostringstream o;
    if (!(o << x))
        return "???";
    return o.str();
}



template <typename T>
void vec2str(std::string &str, const std::vector<T> &v, std::function<std::string(T)> fn) {
    str = "[ ";
    for (const auto &x : v) {
        str += fn(x) + ", ";
    }
    str = str.substr(0, str.size() - 2);
    str += " ]";
    if (str == " ]")
        str = "[]";
}

template<> void value::autoconv<value::S>() {
    switch (type) {
        case I: s = std::to_string(i); type = S; break; 
        case F: s = float_to_string(f); type = S; break;
        case S: break;
        case B: s = (b ? "true" : "false"); type = S; break;
        case NUL: break;
        case IV: vec2str<uint16_t>(s, iv, [](uint16_t x) -> std::string {return std::to_string(x);}); type = S; break;
        case FV: vec2str<float32_t>(s, fv, [](float32_t x) -> std::string {return float_to_string(x);}); type = S; break;
        case SV: vec2str<std::string>(s, sv, [](std::string x) -> std::string {return x;}); type = S; break;
        case BV: vec2str<bool>(s, bv, [](bool x) -> std::string {return (x ? "true" : "false");}); type = S; break;
    }
}

template<> void value::set(uint16_t val) { i = val; type = I; }
template<> void value::set(float32_t val) { f = val; type = F; }
template<> void value::set(bool val) { b = val; type = B; }
template<> void value::set(std::string val) { s = val; type = S; }
template<> void value::set(std::vector<uint16_t> val) { iv = val; type = I; }
template<> void value::set(std::vector<float32_t> val) { fv = val; type = F; }
template<> void value::set(std::vector<bool> val) { bv = val; type = B; }
template<> void value::set(std::vector<std::string> val) { sv = val; type = S; }
template<> void value::set(value other) { *this = other; }

// scalars
template<> uint16_t &value::get<uint16_t>() { assert(type == I); return i; }
template<> float32_t &value::get<float32_t>() { assert(type == F); return f; }
template<> bool &value::get<bool>() { assert(type == B); return b; }
template<> std::string &value::get<std::string>() { assert(type == S); return s; }
template<> const uint16_t &value::get<uint16_t>() const { assert(type == I); return i; }
template<> const float32_t &value::get<float32_t>() const { assert(type == F); return f; }
template<> const bool &value::get<bool>() const { assert(type == B); return b; }
template<> const std::string &value::get<std::string>() const { assert(type == S); return s; }

//vectors
template<> std::vector<uint16_t> &value::get<std::vector<uint16_t>>() { assert(type == IV); return iv; }
template<> std::vector<float32_t> &value::get<std::vector<float32_t>>() { assert(type == FV); return fv; }
template<> std::vector<bool> &value::get<std::vector<bool>>() { assert(type == BV); return bv; }
template<> std::vector<std::string> &value::get<std::vector<std::string>>() { assert(type == SV); return sv; }
template<> const std::vector<uint16_t> &value::get<std::vector<uint16_t>>() const { assert(type == IV); return iv; }
template<> const std::vector<float32_t> &value::get<std::vector<float32_t>>() const { assert(type == FV); return fv; }
template<> const std::vector<bool> &value::get<std::vector<bool>>() const { assert(type == BV); return bv; }
template<> const std::vector<std::string> &value::get<std::vector<std::string>>() const { assert(type == SV); return sv; }

template<> uint16_t value::get_max_possible_value() { return I_MAX; }
template<> float32_t value::get_max_possible_value() { return F_MAX; }
template<> bool value::get_max_possible_value() { return true; }
template<> std::string value::get_max_possible_value() { return ""; }

std::string value::get_max_possible_value(type_t t) {
    switch (t) {
        case value::IV:
        case value::I: { value p; p.set(value::I_MAX); return p.as_string(); }
        case value::FV:
        case value::F: { value p; p.set(value::F_MAX); return p.as_string(); }
        case value::BV:
        case value::B: return "true";
        case value::NUL: return "";
        case value::SV:
        case value::S: return "";
    }
    return "";
}

void value::parse_and_set(value::type_t t, const std::string &value) {
    assert(t != value::IV && t != value::FV && t != value::SV && t != value::BV);
	std::regex unsigned_integer_number("^[0-9]*$", std::regex_constants::ECMAScript);
	std::regex fp_number("^[+-]?([0-9]+([.][0-9]*)?([eE][+-]?[0-9]+)?|[.][0-9]+([eE][+-]?[0-9]+)?)$", 
                         std::regex_constants::ECMAScript);
	if (t == value::I) {
        if (value == "inf") {
            set<uint16_t>(value::get_max_possible_value<uint16_t>());
            return;
        }
		if (!std::regex_search(value, unsigned_integer_number)) {
			assert(0 && "Parse error on integer value");
		}
		set<uint16_t>((uint16_t)std::stoi(value));
	} else if (t == value::F) {
        if (value == "inf") {
            set<float32_t>(value::get_max_possible_value<float32_t>());
            return;
        }
		if (!std::regex_search(value, fp_number)) {
			assert(0 && "Parse error on floating point value");
		}
		set<float32_t>((float32_t)std::stof(value));
    } else if (t == value::B) {
        if (value == "true" || value == "TRUE" || value == "on" || value == "ON" || 
            value == "yes" || value == "YES" || value == "enable" || value == "ENABLE" ||
            value == "1") {
            set<bool>(true);
        } else if (value == "false" || value == "FALSE" || value == "off" || value == "OFF" || 
            value == "no" || value == "NO" || value == "disable" || value == "DISABLE" ||
            value == "0") {
            set<bool>(false);
        } else {
            assert(0 && "Parse error on bool value");
        }
	} else {
		set<std::string>(value);
	}
}

void value::parse_and_set(value::type_t t, const std::vector<std::string> &vec) {
    assert(t == value::IV || t == value::FV || t == value::SV || t == value::BV);
    if (t == value::IV) {
        value v;
        iv.resize(0);
        for (const auto &s : vec) {
            v.parse_and_set(value::I, s);
            iv.push_back(v.i);
        }
    } else if (t == value::FV) {
        value v;
        fv.resize(0);
        for (const auto &s : vec) {
            v.parse_and_set(value::F, s);
            fv.push_back(v.f);
        }
    } else if (t == value::SV) {
        value v;
        sv.resize(0);
        for (const auto &str : vec) {
            v.parse_and_set(value::S, str);
            sv.push_back(v.s);
        }
    } else if (t == value::BV) {
        value v;
        bv.resize(0);
        for (const auto &s : vec) {
            v.parse_and_set(value::B, s);
            bv.push_back(v.b);
        }
    }
    type = t;
}

std::string value::as_string() const { 
    value copy = *this; 
    copy.autoconv<value::S>(); 
    return copy.get<std::string>(); 
}


}

