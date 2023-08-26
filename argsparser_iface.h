/*
 * Copyright (c) 2018-2023 Alexey V. Medvedev
 * This code is an extension of the parts of Intel(R) MPI Benchmarks project.
 * It keeps the same 3-Clause BSD License.
 */

#pragma once
#include <string>
#include <vector>
#include <iostream>
#include <memory>


namespace parser_iface {
    struct parser
    {
        enum { MAX_VEC_SIZE = 1024 };
        void *ptr;
        parser();
        parser(int argc, char **argv);
        ~parser();
        bool parse();
        void add_bool(const std::string &s);
        bool get_bool(const std::string &s);
        bool get_flag(const std::string &s);
        void add_string(const std::string &s);
        std::string get_string(const std::string &s);
        void add_int(const std::string &s);
        int get_int(const std::string &s);
        void add_float(const std::string &s);
        float get_float(const std::string &s);
        void add_bool(const std::string &s, bool v);
        void add_string(const std::string &s, std::string v);
        void add_int(const std::string &s, int v);
        void add_float(const std::string &s, float v);

        void add_flag(const std::string &s);

        void add_bool_vector(const std::string &s, char delim = ',', int min = 0, int max = MAX_VEC_SIZE);
        std::vector<bool> get_bool_vector(const std::string &s);
        void add_string_vector(const std::string &s, char delim = ',', int min = 0, int max = MAX_VEC_SIZE);
        std::vector<std::string> get_string_vector(const std::string &s);
        void add_int_vector(const std::string &s, char delim = ',', int min = 0, int max = MAX_VEC_SIZE);
        std::vector<int> get_int_vector(const std::string &s);
        void add_float_vector(const std::string &s, char delim = ',', int min = 0, int max = MAX_VEC_SIZE);
        std::vector<float> get_float_vector(const std::string &s);

        void add_bool_vector(const std::string &s, const std::string &def, char delim = ',', int min = 0, int max = MAX_VEC_SIZE);
        void add_string_vector(const std::string &s, const std::string &def, char delim = ',', int min = 0, int max = MAX_VEC_SIZE);
        void add_int_vector(const std::string &s, const std::string &def, char delim = ',', int min = 0, int max = MAX_VEC_SIZE);
        void add_float_vector(const std::string &s, const std::string &def, char delim = ',', int min = 0, int max = MAX_VEC_SIZE);

        bool load(std::istream &st); 
        std::string dump(); 
        bool is_option_defaulted(const std::string &s);
    };

    std::shared_ptr<parser> parser_create();
    std::shared_ptr<parser> parser_create(int argc, char **argv);
}
