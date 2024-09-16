/*
 * Copyright (c) 2020-2024 Alexey V. Medvedev
 * This code is licensed under 3-Clause BSD License.
 * See license.txt file for details.
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <vector>
#include <string>
#include <functional>

#include <yaml-cpp/yaml.h>

#include <params.h>
#include <params.inl>
#include <yamlassist.inl>
#include "example_details.h"


//int main(int argc, char **argv)
int main()
{
    //args_parser parser(argc, argv);
    //parser.add_map(...);
    try {
        std::string file = "example.yaml";
        std::ifstream in_stream;
        in_stream.open(file);
        if (in_stream.fail()) {
            throw std::runtime_error(std::string("can not open yaml file: ") + file);
        }
        YAML::Node stream = YAML::Load(in_stream);
        params::dictionary<params::example_params_details> example_dict;
        params::yaml_read_assistant<params::example_params_details> yaml_reader(stream);
        const std::string dict_name = "common/examples/example_dictionary/";
        if (!yaml_reader.get_all_lists(dict_name, example_dict)) {
            std::cout << "No expected dictionary \"" << dict_name << "\"in the yaml file: " << file << std::endl;
            return 1;
        }
        example_dict.set_defaults();
        std::cout << "Expected: dos" << std::endl;
        std::cout << example_dict.get("foo").get_vstring("svec")[1] << std::endl;
    }
    catch(std::exception &ex) {
        std::cerr << "EXCEPTION: " << ex.what() << std::endl;
        return 1;
    }
    catch(...) {
        return 1;
    }
    return 0;
}
