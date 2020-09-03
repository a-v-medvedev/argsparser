#include <iostream>
#include <map>
#include <vector>
#include <string>

#include "params.h"
#include "params.inl"
#include "utest_details.h"

#include <argsparser.h>

using utest_dictionary = params::dictionary<utest_params_details>;
using utest_list = params::list<utest_params_details>;
using utest_overrides_holder = params::overrides_holder<utest_params_details>;

static inline bool parse_params(const std::map<std::string, std::string> kvmap, utest_list &list) {
    for (auto &kv : kvmap) {
        list.parse_and_set_value(kv.first, kv.second);
    }
    return true;
}

utest_list get_list_for_layer(const utest_dictionary &global, 
        const std::string &base, const std::string &overrides, uint16_t layers, uint16_t layer) {
    utest_overrides_holder holder(layers);
    holder.fill_in(global.get(overrides));
    utest_list list = global.get(base);
    if (holder.find(layer)) {
        auto &overrides = holder.get(layer);
        list.override_params(overrides);
    }
    return list;
}

void testsuite_0(int argc, char **argv) 
{
    utest_dictionary global;
    args_parser parser(argc, argv, "-", ' ');
    parser.add_map("foo", "family=xxx");
    parser.add_map("foo_override", "aaa=99@lev2-4").
        set_mode(args_parser::option::APPLY_DEFAULTS_ONLY_WHEN_MISSING);
    parser.add_map("bar", "");
    parser.add_map("bar_override", "");
    if (!parser.parse()) {
        return;
    }
    for (auto list : std::vector<std::string>({ "foo", "bar", "foo_override", "bar_override"})) {
        std::map<std::string, std::string> m;
        parser.get(list, m);
        if (m.empty())
            continue;
        utest_list mlist;
        if (list == "foo_override")
            mlist.omit_value_coversions_and_checks = true;
        if (!parse_params(m, mlist)) {
            std::cerr << "Parse error on parameters; list: " << list << std::endl;
            return;
        }
        global.add(list, mlist);
    }
    global.set_defaults();
    global.print();
    for (int i = 0; i < 10; i++) {
        auto list = get_list_for_layer(global, "foo", "foo_override", 10, i);
        std::cout << "------- Level: " << i << std::endl;
        list.print();
    }
}

void testsuite_1(int argc, char **argv)
{
    (void)argc; (void)argv;
    utest_dictionary global;
    utest_list list;
    list.add_value<std::string>("family", "xxx");
    global.add("foo", list);
    global.set_defaults();
}

void testsuite_2(int argc, char **argv)
{
    (void)argc; (void)argv;
    utest_dictionary params;
    params.add("foo", {"family", "xxx"});
    params.set_defaults();
}

void testsuite_3(int argc, char **argv)
{
    (void)argc; (void)argv;
    utest_dictionary params;
    params.add_map("foo", {{"family", "yyy"}, {"aaa", "50"}});
    std::map<std::string, std::string> baz_params_dict;
    baz_params_dict["family"] = "zzz";
    baz_params_dict["aaa"] = "3";
    params.add_map("baz", baz_params_dict);
    params.set_defaults();
}

void testsuite_4(int argc, char **argv)
{
    (void)argc; (void)argv;
    utest_dictionary params;
    params.add_map("foo", {{"family", "xxx"}, {"aaa", "50"}});
    params.add("bar", {"family", "yyy"});
    params.add("baz", {"family", "zzz"});
    params.add("qux", {"family", "zzz"});
    std::map<std::string, std::string> per_level_over {{"aaa", "5@lev3"}};
    params.add_override("baz", per_level_over);
    params.add_override("qux", per_level_over);
    if (params.find("baz_override")) {
        auto s = params.get("baz_override").
                        get_value_as_string("aaa");
        // Should print: “Override: aaa: 5@lev3”
        std::cout << "Override: " << "aaa" << ": " << s << std::endl;
    }
    params.set_defaults();
}

void testsuite_5(int argc, char **argv)
{
    (void)argc; (void)argv;
    utest_dictionary params;
	params.add("bar", {"family", "yyy"});
    if (params.find_if<std::string>({{"family", "yyy"}})) {
        std::cout << "We have a list with family yyy" << std::endl;
		std::string list_name;
		params.find_if<std::string>({{"family", "yyy"}}, list_name);
		std::cout << "Family yyy found in list: " << list_name << std::endl;
		params.get(list_name).print();
    }
    params.set_defaults();
}

void testsuite_6(int argc, char **argv)
{
    (void)argc; (void)argv;
    utest_dictionary params;
    params.add_map("foo", {{"family", "yyy"}, {"aaa", "50"}});
    utest_list &list = params.get_if<std::string>({{"family", "yyy"}});
    assert(list.is_value_set("aaa"));
    assert(list.get_value_as_string("aaa") == "50");
    assert(list.get_value<uint16_t>("aaa") == 50);
    params.set_defaults();
}

void testsuite_7(int argc, char **argv)
{
    (void)argc; (void)argv;
    utest_dictionary params;
    params.set_defaults();
	params.change_value<float32_t>("baz", "bbb", 1e-12);
	params.forced_change_value<std::string>("baz", "fff", "qwerty");
    params.print();
}

void testsuite_8(int argc, char **argv)
{
    (void)argc; (void)argv;
    utest_dictionary params;
    params.set_defaults();
	utest_list over;
	over.add_value<uint16_t>("aaa", 2);
    over.add_value<std::string>("eee", "test2");
	params.get("baz").override_params(over);
}

void testsuite_9(int argc, char **argv)
{
    (void)argc; (void)argv;
    utest_dictionary params;
    params.add_override("qux", {});
    params.add_override("baz", {});
    params.set_defaults();
	for (int level = 2; level < 5; level++) {
		params.change_value_onlayer<uint16_t>("qux", "ddd", 5555, level);
	}
	auto on_3rd_level = params.get("baz", 3);
    if (on_3rd_level.is_value_set("ddd")) {
        auto value = on_3rd_level.get_value_as_string("ddd");
        std::cout << "There is overriden value of " 
                  << "ddd " << "on 3rd level: " << value << std::endl;
    }   
}

void testsuite_10(int argc, char **argv)
{
    (void)argc; (void)argv;
    utest_dictionary params;
    params.set_defaults();
	auto &list = params.get("foo");
	std::string key, svalue;
    uint16_t ivalue;
	if (!list.is_value_allowed<uint16_t>(key="hhh", ivalue=4)) {
		std::cout << "(key=" << key << " value=" << ivalue << ") Value is not allowed, OK" << std::endl;
	}
	if (!list.is_value_allowed<std::string>(key="eee", svalue="sdfsdf")) {
		std::cout << "(key=" << key << " value=" << svalue << ") Value is not allowed, OK" << std::endl;
	}
	if (list.is_value_allowed<std::string>(key="eee", svalue="test3")) {
		std::cout << "(key=" << key << " value=" << svalue << ") Value is allowed, OK" << std::endl;
	}
}

int main(int argc, char **argv)
{
    testsuite_0(argc, argv);
    testsuite_1(argc, argv);
	testsuite_2(argc, argv);
	testsuite_3(argc, argv);
	testsuite_4(argc, argv);
	testsuite_5(argc, argv);
	testsuite_6(argc, argv);
	testsuite_7(argc, argv);
	testsuite_8(argc, argv);
	testsuite_9(argc, argv);
	testsuite_10(argc, argv);
    return 0;
}
