/*
 * Copyright (c) 2020-2024 Alexey V. Medvedev
 * This code is licensed under 3-Clause BSD License.
 * See license.txt file for details.
 */

#pragma once

#include <sstream>

template <typename DETAILS>
std::string custom_print(const params::value &v) {
   std::string result = v.as_string();
   if (DETAILS::use_debug_print_tables) {
       return "<<<debug_print_tables>>>";
   } else {
       return result;
   }
}

struct utest_params_details {
	using my_dictionary = params::dictionary<utest_params_details>;
	using my_list = params::list<utest_params_details>;

    static bool use_debug_print_tables;
    
    static std::string get_family_key() { return "family"; }
    static std::string get_layer_prefix() { return "lev"; }
	static uint16_t get_nlayers() { return 100; }

    static void print_stream(const std::stringstream &ss) {
        std::cout << ss.str();
    }

    static void print_table(const my_dictionary &params) {
	    my_list::print_line_delimiter();
    	my_list::print_header("Unit test for params");
        auto size = params.size();
        for (size_t i = 0; i < size; i++) {
            auto &l = params.get(i);
            if (!std::regex_search(l, std::regex("_override")))
                params.print_list(l, l);
        }
    }


#define ALLFAMILIES {}
#define NOMINMAX {}
#define ALLALLOWED {}

	static const params::expected_params_t &get_expected_params() {
		static const params::expected_params_t expected_params = {
			{ "family", { params::value::S, false,	ALLFAMILIES, 		NOMINMAX, 		ALLALLOWED } },
			{ "aaa", 	{ params::value::I, true, 	ALLFAMILIES, 		NOMINMAX, 		ALLALLOWED } },
			{ "bbb", 	{ params::value::F, true, 	{ "xxx", "zzz" }, 	NOMINMAX, 		ALLALLOWED } },
			{ "eee", 	{ params::value::S, true, 	{ "xxx", "yyy" }, 	NOMINMAX, 		{ "test1", "test2", "test3" } } },
			{ "ccc", 	{ params::value::F, false, 	{ "yyy" }, 			NOMINMAX, 		ALLALLOWED } },
			{ "ddd", 	{ params::value::I, true, 	{ "yyy", "zzz" }, 	NOMINMAX, 		ALLALLOWED } },
			{ "fff", 	{ params::value::S, false, 	{ "yyy", "xxx" }, 	NOMINMAX, 		ALLALLOWED } },
			{ "hhh", 	{ params::value::I, false, 	{ "!yyy" }, 		{ "1", "3" }, 	ALLALLOWED } },
			{ "iii", 	{ params::value::I, false, 	ALLFAMILIES, 		NOMINMAX, 		{ "1", "2", "5" } } },
			{ "ivec", 	{ params::value::IV, true, 	ALLFAMILIES, 		NOMINMAX, 		ALLALLOWED } },
			{ "fvec", 	{ params::value::FV, true, 	ALLFAMILIES, 		NOMINMAX, 		ALLALLOWED } },
			{ "svec", 	{ params::value::SV, true, 	ALLFAMILIES, 		NOMINMAX, 		ALLALLOWED } },
			{ "bvec", 	{ params::value::BV, true, 	ALLFAMILIES, 		NOMINMAX, 		ALLALLOWED } },
		};
		return expected_params;
    }

#undef ALLFAMILIES 
#undef NOMINMAX 
#undef ALLALLOWED 

	static void set_dictionary_defaults(my_dictionary &params) {
		if (!params.find("foo")) {
			params.add("foo", my_list { "family", "xxx"});
		}
		if (!params.find("bar")) {
			params.add("bar", my_list { "family", "yyy"});
		}
		if (!params.find("baz")) {
			params.add("baz", my_list { "family", "zzz"});
		}
		if (!params.find("qux")) {
			params.add("qux", my_list { "family", "zzz"});
		}
	}

	static void set_family_defaults(my_list &list, const std::string &family, 
									const std::string &list_name) {
		(void)list_name;
		if (family == "xxx") {
			list.set_value_if_missing<uint32_t>("aaa", 56);
			list.set_value_if_missing<float64_t>("bbb", 1.234);
			list.set_value_if_missing<std::string>("fff", "test");
			list.set_value_if_missing<std::string>("eee", "test2");
			list.set_value_if_missing<uint32_t>("hhh", 2);
			list.set_value_if_missing<uint32_t>("iii", 5);
		} else if (family == "yyy") {
			list.set_value_if_missing<uint32_t>("ddd", 777);
			list.set_value_if_missing<float64_t>("ccc", 4.567);
			list.set_value_if_missing<std::string>("fff", "test");
			list.set_value_if_missing<std::string>("eee", "test1");
			list.set_value_if_missing<uint32_t>("iii", 5);
		} else if (family == "zzz") {
			list.set_value_if_missing<uint32_t>("aaa", 56);
			list.set_value_if_missing<float64_t>("bbb", 1.234);
			list.set_value_if_missing<uint32_t>("ddd", 777);
			list.set_value_if_missing<uint32_t>("hhh", 2);
			list.set_value_if_missing<uint32_t>("iii", 5);
		}
        list.add_print_converter("aaa", custom_print<utest_params_details>);
	}
};
