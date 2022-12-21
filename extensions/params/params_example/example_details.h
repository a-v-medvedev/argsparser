/*
 * Copyright (c) 2020-2022 Alexey V. Medvedev
 * This code is licensed under 3-Clause BSD License.
 * See license.txt file for details.
 */

#pragma once

#include <sstream>

namespace params {

struct example_params_details {
	using my_dictionary = params::dictionary<example_params_details>;
	using my_list = params::list<example_params_details>;

    static std::string get_family_key() { return "family"; }
    static std::string get_layer_prefix() { return "lev"; }
	static uint16_t get_nlayers() { return 100; }

    static void print_stream(const std::stringstream &ss) {
        std::cout << ss.str();
    }

    static void print_table(const my_dictionary &params) {
	    my_list::print_line_delimiter();
    	my_list::print_header("EXAMPLE set of params");
        auto size = params.size();
        for (size_t i = 0; i < size; i++) {
            auto &l = params.get(i);
            params.print_list(l, l);
        }
    }


#define ALLFAMILIES {}
#define NOMINMAX {}
#define ALLALLOWED {}

	static const params::expected_params_t &get_expected_params() {
		static const params::expected_params_t expected_params = {
			{ "family",     { params::value::S, false,	ALLFAMILIES, 		        NOMINMAX, 		ALLALLOWED } },
			{ "integer", 	{ params::value::I, true, 	ALLFAMILIES, 		        NOMINMAX, 		ALLALLOWED } },
			{ "float", 	    { params::value::F, true, 	{ "mammalia", "aves" }, 	NOMINMAX, 		ALLALLOWED } },
			{ "string", 	{ params::value::S, true, 	{ "mammalia", "reptilia" }, NOMINMAX, 		{ "test1", "test2", "test3" } } },
			{ "floatx", 	{ params::value::F, false, 	{ "reptilia" }, 			NOMINMAX, 		ALLALLOWED } },
			{ "integerx", 	{ params::value::I, true, 	{ "reptilia", "aves" }, 	NOMINMAX, 		ALLALLOWED } },
			{ "stringx", 	{ params::value::S, false, 	{ "reptilia", "mammalia" }, NOMINMAX, 		ALLALLOWED } },
			{ "integerX", 	{ params::value::I, false, 	{ "!reptilia" }, 		    { "1", "3" }, 	ALLALLOWED } },
			{ "integerXX", 	{ params::value::I, false, 	ALLFAMILIES, 		        NOMINMAX, 		{ "1", "2", "5" } } },
			{ "ivec", 	    { params::value::IV, true, 	ALLFAMILIES, 		        NOMINMAX, 		ALLALLOWED } },
			{ "fvec", 	    { params::value::FV, true, 	ALLFAMILIES, 		        NOMINMAX, 		ALLALLOWED } },
			{ "svec", 	    { params::value::SV, true, 	ALLFAMILIES, 		        NOMINMAX, 		ALLALLOWED } },
			{ "bvec", 	    { params::value::BV, true, 	ALLFAMILIES, 		        NOMINMAX, 		ALLALLOWED } },
		};
		return expected_params;
    }

#undef ALLFAMILIES 
#undef NOMINMAX 
#undef ALLALLOWED 

	static void set_family_defaults(my_list &list, const std::string &family, 
									const std::string &list_name) {
		(void)list_name;
		if (family == "mammalia") {
			list.set_value_if_missing<uint32_t>("integer", 56);
			list.set_value_if_missing<float64_t>("float", 1.234);
			list.set_value_if_missing<std::string>("stringx", "test");
			list.set_value_if_missing<std::string>("string", "test2");
			list.set_value_if_missing<uint32_t>("integerX", 2);
			list.set_value_if_missing<uint32_t>("integerXX", 5);
		} else if (family == "reptilia") {
			list.set_value_if_missing<uint32_t>("integerx", 777);
			list.set_value_if_missing<float64_t>("float", 4.567);
			list.set_value_if_missing<std::string>("stringx", "test");
			list.set_value_if_missing<std::string>("string", "test1");
			list.set_value_if_missing<uint32_t>("integerXX", 5);
		} else if (family == "aves") {
			list.set_value_if_missing<uint32_t>("integer", 56);
			list.set_value_if_missing<float64_t>("float", 1.234);
			list.set_value_if_missing<uint32_t>("integerx", 777);
			list.set_value_if_missing<uint32_t>("integerX", 2);
			list.set_value_if_missing<uint32_t>("integerXX", 5);
		}
	}

    static void set_dictionary_defaults(my_dictionary &params) {
		if (!params.find("foo")) {
			params.add("foo", my_list {"family", "mammalia"});
		}
		if (!params.find("bar")) {
			params.add("bar", my_list {"family", "reptilia"});
		}
		if (!params.find("baz")) {
			params.add("baz", my_list {"family", "aves"});
		}
		if (!params.find("qux")) {
			params.add("qux", my_list {"family", "aves"});
		}
	}
};

}
