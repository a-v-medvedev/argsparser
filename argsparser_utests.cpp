/*
 * Copyright (c) 2018-2020 Alexey V. Medvedev
 * This code is an extension of the parts of Intel(R) MPI Benchmarks project.
 * It keeps the same Community Source License (CPL) license.
 */

/*****************************************************************************
 *                                                                           *
 * Copyright (c) 2016-2018 Intel Corporation.                                *
 * All rights reserved.                                                      *
 *                                                                           *
 *****************************************************************************

This code is covered by the Community Source License (CPL), version
1.0 as published by IBM and reproduced in the file "license.txt". 
Redistribution in source and binary form, with or without modification, 
is permitted ONLY within the regulations contained in above mentioned license.

THE PROGRAM IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OR
CONDITIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED INCLUDING, WITHOUT
LIMITATION, ANY WARRANTIES OR CONDITIONS OF TITLE, NON-INFRINGEMENT,
MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE. Each Recipient is
solely responsible for determining the appropriateness of using and
distributing the Program and assumes all risks associated with its
exercise of rights under this Agreement, including but not limited to
the risks and costs of program errors, compliance with applicable
laws, damage to or loss of data, programs or equipment, and
unavailability or interruption of operations.

EXCEPT AS EXPRESSLY SET FORTH IN THIS AGREEMENT, NEITHER RECIPIENT NOR
ANY CONTRIBUTORS SHALL HAVE ANY LIABILITY FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING
WITHOUT LIMITATION LOST PROFITS), HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OR
DISTRIBUTION OF THE PROGRAM OR THE EXERCISE OF ANY RIGHTS GRANTED
HEREUNDER, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.

EXPORT LAWS: THIS LICENSE ADDS NO RESTRICTIONS TO THE EXPORT LAWS OF
YOUR JURISDICTION. It is licensee's responsibility to comply with any
export regulations applicable in licensee's jurisdiction. Under
CURRENT U.S. export regulations this software is eligible for export
from the U.S. and can be downloaded by or otherwise exported or
reexported worldwide EXCEPT to U.S. embargoed destinations which
include Cuba, Iraq, Libya, North Korea, Iran, Syria, Sudan,
Afghanistan and any other country to which the U.S. has embargoed
goods and services.
*/

#include "argsparser.h"

#ifdef WITH_YAML_CPP
#include "yaml-cpp/yaml.h"
#endif

#include <stdexcept>
#include <assert.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <stdio.h>
#include <sstream>
#include <algorithm>
#include <functional>
#include <numeric>

using namespace std;

//-- UNIT TESTS ----------------------------------------------------------------------------------

    // Current coverage: 79.3% argsparser.cpp, 82.1% argsparser.h
    // 
    // Script:
    //
    // # CFLAGS_OPT="-g -O0 --coverage" LDFLAGS="-lgcov" make all argsparser_utests
    // # ./argsparser_utests
    // # lcov -t "argsparser_utests_coverage" -o argsparser_utests_coverage.info -c -d .
    // # genhtml -o coverage_report argsparser_utests_coverage.info
    //
    // TODO check set_mode ALLOW_UNEXPECTED_ARGS flag_t
    // TODO argsparser.cpp:968-982
    // TODO load()/dump() with extra args
    // TODO groups other than SYS & EXTRA_ARGS
    // TODO NODUPLICATE flag_t
    // TODO NOHELP flag_t
    // TODO SILENT flag_t
    // TODO float type in get_type_str()
    // TODO get_command_line(), is_option(), is_option_defaulted(), print()
    // TODO various combinations of different options in the same cmdline, check options with similar names
    // TODO all error cases
//------------------------------------------------------------------------------------------------

void check_parser();

int main()
{
    check_parser();
    return 0;
}

void print_args(int nargs, const char * const *argv) {
    std::cout << ":: ";
    for (int i = 0; i < nargs; i++) {
        std::cout << argv[i];
        if (i != nargs - 1) std::cout << " ";
    }
    std::cout << std::endl;
}

struct CheckParser {
    bool result;
    bool except;
    shared_ptr<args_parser> pparser;
    ostringstream streamout;
    std::string strexception;
    CheckParser() : result(false), except(false) {}
    args_parser &parser() { return *pparser; }
    args_parser &init(int argc, char ** argv, int mode = 1) {
        print_args(argc, argv);
        switch (mode) {
            case 1: pparser = std::make_shared<args_parser>(argc, argv, "-", ' ', streamout); break;
            case 2: pparser = std::make_shared<args_parser>(argc, argv, "--", '=', streamout); break;
            case 3: pparser = std::make_shared<args_parser>(argc, argv, "/", ':', streamout); break;
            default: assert(false);
        }
        return *pparser;
    }
    args_parser &run() {
        try {
            result = pparser->parse();
        }
        catch(exception &ex) {
            strexception = ex.what();
            std::cout << "EXCEPTION: " << ex.what() << std::endl;
            except = true;
        }
        return *pparser;
    }
    args_parser &dump(std::string &dumped) {
        try {
            dumped = pparser->dump();
            return *pparser;
        }
        catch(exception &ex) {
            strexception = ex.what();
            std::cout << "EXCEPTION: " << ex.what() << std::endl;
            except = true;
        }
        return *pparser;
    }
    args_parser &load(const std::string &dumped) {
        try {
            result = pparser->load(dumped);
            return *pparser;
        }
        catch(exception &ex) {
            strexception = ex.what();
            std::cout << "EXCEPTION: " << ex.what() << std::endl;
            except = true;
        }
        return *pparser;
    }
    size_t lines_in_output() {
        const auto &s = streamout.str();
        size_t cnt = std::count(s.begin(), s.end(), '\n');
        return cnt;
    }
    bool output_contains(const std::string &what) {
        const std::string &str = streamout.str();
        return (streamout.str().find(what) != std::string::npos);
    }
};

enum function_t { PARSE, DUMP, LOAD };
enum delimiter_t { RAW, WITH_SPACE, WITH_EQUAL, WITH_SLASH };

template <typename T> 
void val2str(T val, std::string &str)
{
    ostringstream os;
    os << val;
    str = os.str();
}
    
template <> 
void val2str<bool>(bool val, std::string &str)
{
    str = (val ? "true" : "false");
}

template <typename T> 
void vals2str(vector<T> vals, std::string delim, std::string &str)
{
    for (size_t i = 0; i < vals.size(); i++) {
        std::string tmp;
        val2str<T>(vals[i], tmp);
        str += tmp;
        if (i != vals.size() - 1) {
            str += delim;
        }
    }
}

template <typename T>
int make_arg(int start, const char *(&argv)[1024], std::string opt, delimiter_t mode)
{
    switch (mode) {
        case RAW: { argv[start] = strdup(opt.c_str()); return 1; break; }
        case WITH_SPACE: { 
                    std::string a1 = "-" + opt; 
                    argv[start] = strdup(a1.c_str()); 
                    return 1; 
                    break; 
                }
        case WITH_EQUAL: { std::string a1 = "--" + opt; argv[start] = strdup(a1.c_str()); return 1; break; }
        case WITH_SLASH: { std::string a1 = "/" + opt; argv[start] = strdup(a1.c_str()); return 1; break; }
        default: assert(false);
    }
    return 0;
}

template <typename T>
int make_args(int start, const char *(&argv)[1024], std::string opt, const std::string &sval, delimiter_t mode)
{
    switch (mode) {
        case WITH_SPACE: { 
                    std::string a1 = "-" + opt; 
                    argv[start] = strdup(a1.c_str()); 
                    argv[start+1] = strdup(sval.c_str()); 
                    return 2; 
                    break; 
                }
        case WITH_EQUAL: { std::string a1 = "--" + opt + "=" + sval; argv[start] = strdup(a1.c_str()); return 1; break; }
        case WITH_SLASH: { std::string a1 = "/" + opt + ":" + sval; argv[start] = strdup(a1.c_str()); return 1; break; }
        default: assert(false);
    }
    return 0;
}

int make_single_arg(const char *(&argv)[1024], std::string opt, delimiter_t mode)
{
    argv[0] = strdup("check");
    return make_arg<std::string>(1, argv, opt, mode) + 1;
}

template <typename T>
int make_keyvalue_arg_scalar(const char *(&argv)[1024], std::string opt, delimiter_t mode, T val)
{
    std::string sval;
    val2str<T>(val, sval);
    argv[0] = strdup("check");
    return make_args<T>(1, argv, opt, sval, mode) + 1;
}
 
template <typename T>
int make_keyvalue_arg_vector(const char *(&argv)[1024], std::string opt, delimiter_t mode, T val1, T val2)
{
    std::string sval;
    vector<T> vals;
    vals.push_back(val1);
    vals.push_back(val2);
    vals2str<T>(vals, ",", sval);
    argv[0] = strdup("check");
    return make_args<T>(1, argv, opt, sval, mode) + 1;
}

template <typename T>
void basic_scalar_check(T val) {
    const char * argv[1024]; 
    for (auto mode : std::vector<delimiter_t> { WITH_SPACE, WITH_EQUAL, WITH_SLASH }) {
        int nargs = make_keyvalue_arg_scalar<T>(argv, "aaa", mode, val);
        CheckParser p;
        p.init(nargs, (char **)argv, mode).add<T>("aaa").set_caption("bbb");
        T result = p.run().get<T>("aaa");
        assert(result == val && p.result && !p.except);
    }
}
 

template <typename T>
void err_scalar_check(T val) {
    const char * argv[1024]; 
    for (auto mode : std::vector<delimiter_t> { WITH_SPACE, WITH_EQUAL, WITH_SLASH }) {
        std::string opt, ext;
        int nargs = make_keyvalue_arg_scalar<T>(argv, "aaaa", mode, val);
        CheckParser p;
        p.init(nargs, (char **)argv, mode).add<T>("aaa").set_caption("bbb");
        args_parser::error_t err = p.run().get_last_error(opt, ext);
        assert(!p.result && !p.except && err == args_parser::NO_REQUIRED_OPTION &&
                opt == "aaa" && (p.lines_in_output() == 2 || p.lines_in_output() == 3) && 
                p.output_contains("ERROR:") && p.output_contains("usage information"));
    }
}

template <typename T>
void basic_vector_check(T val1, T val2) {
    const char *argv[1024]; 
    for (auto mode : std::vector<delimiter_t> { WITH_SPACE, WITH_EQUAL, WITH_SLASH }) {    
        std::string dumped;
        for (auto dump_load_or_parse : std::vector<function_t> { PARSE, DUMP, LOAD }) {
            int nargs = make_keyvalue_arg_vector<T>(argv, "aaa", mode, val1, val2);
            CheckParser p;
            p.init(nargs, (char **)argv, mode).add_vector<T>("aaa", ',').set_caption("bbb");
            vector<T> result;
            switch (dump_load_or_parse) {
                case PARSE: p.run().get<T>("aaa", result); break;
                case DUMP: p.run(); p.dump(dumped).get<T>("aaa", result); break;
                case LOAD: p.load(dumped); p.run().get<T>("aaa", result); break;
                default: assert(0);
            } 
            assert(result.size() == 2 && result[0] == val1 && result[1] == val2 && p.result && !p.except);
        }
    }
}

template <typename T>
void default_scalar_check(T def) {
    const char *argv[1024]; 
    for (auto mode : std::vector<delimiter_t> { WITH_SPACE, WITH_EQUAL, WITH_SLASH }) {
        std::string dumped;
        for (auto dump_load_or_parse : std::vector<function_t> { PARSE, DUMP, LOAD }) {
            int nargs = 0;
            nargs += make_arg<std::string>(nargs, argv, "check", delimiter_t::RAW);
            CheckParser p;
            p.init(nargs, (char **)argv, mode).add<T>("aaa", def).set_caption("bbb");
			T res;
            switch (dump_load_or_parse) {
                case PARSE: res = p.run().get<T>("aaa"); break;
                case DUMP: p.run(); res = p.dump(dumped).get<T>("aaa"); break;
                case LOAD: p.run(); p.load(dumped); res = p.run().get<T>("aaa"); break;
                default: assert(0);
            } 
            assert(res == def && p.result && !p.except);
        }
    }
}

template <typename T>
void default_vector_check(const char *def, size_t n) {
    const char *argv[1024]; 
    for (auto mode : std::vector<delimiter_t> { WITH_SPACE, WITH_EQUAL, WITH_SLASH }) {
        std::string dumped;
        for (auto dump_load_or_parse : std::vector<function_t> { PARSE, DUMP, LOAD }) {
	        int nargs = 0;
    	    nargs += make_arg<std::string>(nargs, argv, "check", delimiter_t::RAW);
        	CheckParser p;
	        p.init(nargs, (char **)argv, mode).add_vector<T>("aaa", def).set_caption("bbb");
    	    vector<T> result;
            switch (dump_load_or_parse) {
                case PARSE: p.run().get<T>("aaa", result); break;
                case DUMP: p.run(); p.dump(dumped).get<T>("aaa", result); break;
                case LOAD: p.run(); p.load(dumped); p.run().get<T>("aaa", result); break;
                default: assert(0);
            } 
	        assert(result.size() == n && p.result && !p.except);
		}
    }
}

template <typename T>
void default_vector_check_ext(const char *def, const char *sval, size_t n, T val1, T val2) {
    const char *argv[1024]; 
    for (auto mode : std::vector<delimiter_t> { WITH_SPACE, WITH_EQUAL, WITH_SLASH }) { 
		std::string dumped;
		for (auto dump_load_or_parse : std::vector<function_t> { PARSE, DUMP, LOAD }) {
			int nargs = 0;
			nargs += make_arg<std::string>(nargs, argv, "check", delimiter_t::RAW);
			nargs += make_args<std::string>(nargs, argv, "aaa", sval, mode);
			CheckParser p;
			p.init(nargs, (char **)argv, mode).add_vector<T>("aaa", def).set_caption("bbb");
			vector<T> result;
            switch (dump_load_or_parse) {
                case PARSE: p.run().get<T>("aaa", result); break;
                case DUMP: p.run(); p.dump(dumped).get<T>("aaa", result); break;
                case LOAD: p.run(); p.load(dumped).get<T>("aaa", result); break;
                default: assert(0);
            } 
			assert(result.size() == n && p.result && !p.except);
			assert(result[0] == val1 && result[1] == val2);
		}
    }
}

void basic_flag_check() {
    const char *argv[1024]; 
    for (auto mode : std::vector<delimiter_t> { WITH_SPACE, WITH_EQUAL, WITH_SLASH }) {
		std::string dumped;
		for (auto dump_load_or_parse : std::vector<function_t> { PARSE, DUMP, LOAD }) {
			int nargs = 0;
			nargs += make_arg<std::string>(nargs, argv, "check", delimiter_t::RAW);
			nargs += make_arg<std::string>(nargs, argv, "aaa", mode);
			CheckParser p;
			p.init(nargs, (char **)argv, mode).add_flag("aaa");
            p.pparser->add_flag("bbb");
			bool res_a, res_b;
            switch (dump_load_or_parse) {
                case PARSE: res_a = p.run().get<bool>("aaa"); 
							res_b = p.pparser->get<bool>("bbb");
                            break;
                case DUMP: p.run(); 
                           res_a = p.dump(dumped).get<bool>("aaa"); 
						   res_b = p.pparser->get<bool>("bbb"); 
                           break;
                case LOAD: p.load(dumped); 
                           res_a = p.run().get<bool>("aaa"); 
                           res_b = p.pparser->get<bool>("bbb");
						   break;
                default: assert(0);
            } 
			assert(res_a && !res_b && p.result && !p.except);
		}
    }
}


void basic_map_check(const char *sval, size_t n, 
                     const std::map<std::string, std::string> &expected) {
    const char *argv[1024]; 
    for (auto mode : std::vector<delimiter_t> { WITH_SPACE, WITH_EQUAL, WITH_SLASH }) {    
		std::string dumped;
		for (auto dump_load_or_parse : std::vector<function_t> { PARSE, DUMP, LOAD }) {
			int nargs = 0;
			nargs += make_arg<std::string>(nargs, argv, "check", delimiter_t::RAW);
			nargs += make_args<std::string>(nargs, argv, "aaa", sval, mode);
			CheckParser p;
			p.init(nargs, (char **)argv, mode).add_map("aaa", '!').set_caption("bbb");
			std::map<std::string, std::string> result;
            switch (dump_load_or_parse) {
                case PARSE: p.run().get("aaa", result); break;
                case DUMP: p.run(); p.dump(dumped).get("aaa", result); break;
                case LOAD: p.load(dumped); p.run().get("aaa", result); break;
                default: assert(0);
            } 
			assert(result.size() == n && p.result && !p.except);
			for (auto kv : expected) {
				auto key = kv.first;
				auto value = kv.second;
				assert(result[key] == value);
			}
		}
    }
}

void default_map_check_ext(const char *def, const char *sval, size_t n, 
                           const std::map<std::string, std::string> &expected) {
    const char *argv[1024]; 
    for (auto mode : std::vector<delimiter_t> { WITH_SPACE, WITH_EQUAL, WITH_SLASH }) {    
		std::string dumped;
		for (auto dump_load_or_parse : std::vector<function_t> { PARSE, DUMP, LOAD }) {
			int nargs = 0;
			nargs += make_arg<std::string>(nargs, argv, "check", delimiter_t::RAW);
			nargs += make_args<std::string>(nargs, argv, "aaa", sval, mode);
			CheckParser p;
			p.init(nargs, (char **)argv, mode).add_map("aaa", def).set_caption("bbb");
			std::map<std::string, std::string> result;
            switch (dump_load_or_parse) {
                case PARSE: p.run().get("aaa", result); break;
                case DUMP: p.run(); p.dump(dumped).get("aaa", result); break;
                case LOAD: p.run(); p.load(dumped); p.run().get("aaa", result); break;
                default: assert(0);
            } 
			assert(result.size() == n && p.result && !p.except);
			for (auto kv : expected) {
				auto key = kv.first;
				auto value = kv.second;
				assert(result[key] == value);
			}
		}
    }
}

void check_help() {
    const char *argv[1024];
    for (auto mode : std::vector<delimiter_t> { WITH_SPACE, WITH_EQUAL, WITH_SLASH }) {    
        {
            int nargs = 0;
            nargs += make_arg<std::string>(nargs, argv, "check", delimiter_t::RAW);
            nargs += make_arg<std::string>(nargs, argv, "help", mode);
            CheckParser p;
            p.init(nargs, (char **)argv, mode);
            p.run();
            assert(p.output_contains("check")); 
            assert(p.output_contains("option"));
            assert(p.output_contains("Usage:"));
            assert(!p.result);
        }
        {
            int nargs = 0;
            nargs += make_arg<std::string>(nargs, argv, "check", delimiter_t::RAW);
            nargs += make_args<std::string>(nargs, argv, "help", "aaa", mode);
            CheckParser p;
            p.init(nargs, (char **)argv, mode).add<int>("aaa").set_caption("bbb").set_description("ccc");
            p.run();
            assert(p.output_contains("Option:")); 
            assert(p.output_contains("bbb"));
            assert(p.output_contains("ccc"));
            assert(!p.result);
        }
        {
            int nargs = 0;
            nargs += make_arg<std::string>(nargs, argv, "check", delimiter_t::RAW);
            nargs += make_arg<std::string>(nargs, argv, "help", mode);
            CheckParser p;
            p.init(nargs, (char **)argv, mode).add<int>("aaa").set_caption("bbb").set_description("ccc");
            p.run();
            assert(p.output_contains("check")); 
            assert(p.output_contains("help")); 
            assert(p.output_contains("bbb"));
            assert(p.output_contains("aaa"));
            assert(!p.result);
        }
        {
            int nargs = 0;
            nargs += make_arg<std::string>(nargs, argv, "check", delimiter_t::RAW);
            nargs += make_arg<std::string>(nargs, argv, "help", mode);
            CheckParser p;
            p.init(nargs, (char **)argv, mode);
	        p.parser().set_current_group("EXTRA_ARGS");
    	    p.parser().add<std::string>("(first_list)");
        	p.parser().add<int>("(second_list)");
	        p.parser().set_default_current_group();
			p.parser().set_caption(0, "aaa");
			p.parser().set_caption(1, "bbb");
            p.run();
            assert(p.output_contains("check")); 
            assert(p.output_contains("help")); 
            assert(p.output_contains("bbb"));
            assert(p.output_contains("aaa"));
            assert(!p.result);
        }
    }
}

void check_extra_args() {
    const char *argv[1024];
    {
        int nargs = 0;
        nargs += make_arg<std::string>(nargs, argv, "check", delimiter_t::RAW);
        nargs += make_arg<std::string>(nargs, argv, "aaa", delimiter_t::RAW);
        nargs += make_arg<std::string>(nargs, argv, "bbb", delimiter_t::RAW);
        CheckParser p;
        p.init(nargs, (char **)argv, delimiter_t::WITH_EQUAL).set_flag(args_parser::ALLOW_UNEXPECTED_ARGS);
        p.run();
        assert(p.result && !p.except);
        std::vector<std::string> args;
        p.parser().get_unknown_args(args);
        assert(args.size() == 2 && args[0] == "aaa" && args[1] == "bbb");
    }
    {
        int nargs = 0;
        nargs += make_arg<std::string>(nargs, argv, "check", delimiter_t::RAW);
        nargs += make_arg<std::string>(nargs, argv, "aaa,bbb,ccc", delimiter_t::RAW);
        nargs += make_arg<std::string>(nargs, argv, "1:2:3", delimiter_t::RAW);
        nargs += make_arg<std::string>(nargs, argv, "zzz", delimiter_t::RAW);
        CheckParser p;
        p.init(nargs, (char **)argv, delimiter_t::WITH_EQUAL).set_flag(args_parser::ALLOW_UNEXPECTED_ARGS);
        p.parser().set_current_group("SYS");
        p.parser().add_flag("dummy");
        p.parser().set_default_current_group();
        p.parser().set_current_group("EXTRA_ARGS");
        p.parser().add_vector<std::string>("(first_list)", "");
        p.parser().add_vector<int>("(second_list)", "", ':');
        p.parser().set_default_current_group();
        p.parser().set_caption(0, "caption1");
        p.parser().set_caption(1, "caption2");
        p.run();
        assert(p.result && !p.except);
        std::vector<std::string> args;
        p.parser().get<std::string>("(first_list)", args);
        std::vector<int> ints;
        p.parser().get<int>("(second_list)", ints);
        p.parser().get_unknown_args(args);
        assert(args.size() == 4 && args[0] == "aaa" && args[1] == "bbb" && args[2] == "ccc" && args[3] == "zzz");
        assert(ints.size() == 3 && std::accumulate(ints.begin(), ints.end(), 0) == 6);
    }
}

void check_parser()
{
    basic_scalar_check<int>(5);
    basic_scalar_check<int>(-5);
    basic_scalar_check<float>(5.5);
    basic_scalar_check<float>(-5.5);
    basic_scalar_check<bool>(true);
    basic_scalar_check<bool>(true);
    basic_scalar_check<std::string>("ccc");
    
    basic_vector_check<int>(5, 5);
    basic_vector_check<int>(5, -5);
    basic_vector_check<int>(-5, 5);
    basic_vector_check<float>(5.5, 5.5);
    basic_vector_check<float>(-5.5, 5.5);
    basic_vector_check<bool>(true, false);
    basic_vector_check<bool>(true, false);
    basic_vector_check<std::string>("ccc", "ddd");

    default_scalar_check<int>(5);
    default_scalar_check<float>(5.5);
    default_scalar_check<bool>(true);
    default_scalar_check<std::string>("ccc");

    default_vector_check<int>("", 0);
    default_vector_check<int>("5", 1);
    default_vector_check<int>("5,-5", 2);
    default_vector_check<float>("", 0);
    default_vector_check<float>("5.5", 1);
    default_vector_check<float>("5.5,.3", 2);
    default_vector_check<bool>("", 0);
    default_vector_check<bool>("true", 1);
    default_vector_check<bool>("true,false", 2);
    default_vector_check<std::string>("", 0);
    default_vector_check<std::string>("ccc", 1);
    default_vector_check<std::string>("ccc,ddd", 2);

    default_vector_check_ext<int>("5,-5", "1", 2, 1, -5);
    default_vector_check_ext<int>("5,-5", "1,1", 2, 1, 1);
    default_vector_check_ext<float>("5.,-5.0e0", ".1", 2, 0.1, -5.0);
    default_vector_check_ext<float>("5.", ".1,1e-6", 2, 0.1, 1e-6);
    default_vector_check_ext<bool>("true,false", "false", 2, false, false);
    default_vector_check_ext<bool>("true", "false,true", 2, false, true);
    default_vector_check_ext<bool>("", "false,true", 2, false, true);
    default_vector_check_ext<bool>("false,false", "", 2, false, false);
    default_vector_check_ext<std::string>("aaa,bbbb", "ccc", 2, "ccc", "bbbb");
    default_vector_check_ext<std::string>("aaa", "ccc,ddd", 2, "ccc", "ddd");
    default_vector_check_ext<std::string>("", "ccc,ddd", 2, "ccc", "ddd");
    default_vector_check_ext<std::string>("aaa,bbbb", "", 2, "aaa", "bbbb");

	basic_flag_check();

    basic_map_check("xxx=aaa!yyy=bbbb", 2, { {"xxx", "aaa"}, {"yyy", "bbbb"} });
    basic_map_check("yyy=bbbb", 1, { {"yyy", "bbbb"} });
    basic_map_check("yyy=bbbb!xxx=aaa!xxx=111!xxx=222", 2, 
                                             { {"xxx", "aaa;111;222"}, {"yyy", "bbbb"} });
    basic_map_check("yyy=bbbb!xxx=aaa!xxx=111!xxx=222", 2, 
                                             { {"xxx", "aaa;111;222"}, {"yyy", "bbbb"} });
    basic_map_check("yyy=bbbb!xxx=aaa!xxx=111!xxx=222!yyy=zzzz", 2, 
                                        { {"xxx", "aaa;111;222"}, {"yyy", "bbbb;zzzz"} });

    default_map_check_ext("xxx=aaa:yyy=bbbb", "", 2, { {"xxx", "aaa"}, {"yyy", "bbbb"} });
    default_map_check_ext("xxx=aaa", "yyy=bbbb", 2, { {"xxx", "aaa"}, {"yyy", "bbbb"} });
    default_map_check_ext("", "yyy=bbbb:xxx=aaa:xxx=111:xxx=222", 2, 
                                              { {"xxx", "aaa;111;222"}, {"yyy", "bbbb"} });
    default_map_check_ext("xxx=21", "yyy=bbbb:xxx=aaa:xxx=111:xxx=222", 2, 
                                              { {"xxx", "aaa;111;222"}, {"yyy", "bbbb"} });
    default_map_check_ext("xxx=21:yyy=ccc", "yyy=bbbb:xxx=aaa:xxx=111:xxx=222:yyy=zzzz", 2, 
                                         { {"xxx", "aaa;111;222"}, {"yyy", "bbbb;zzzz"} });

    err_scalar_check<std::string>("ccc");

    check_help();

    check_extra_args();



}

