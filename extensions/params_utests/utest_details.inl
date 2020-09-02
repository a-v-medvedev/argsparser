#pragma once

#define ALLFAMILIES {}
#define NOMINMAX {}
#define ALLALLOWED {}

const params::expected_params_t utest_params_details::expected_params = {
	{ "family", { params::value::S, false,	ALLFAMILIES, 		NOMINMAX, 		ALLALLOWED } },
	{ "aaa", 	{ params::value::I, true, 	ALLFAMILIES, 		NOMINMAX, 		ALLALLOWED } },
	{ "bbb", 	{ params::value::F, true, 	{ "xxx", "zzz" }, 	NOMINMAX, 		ALLALLOWED } },
	{ "eee", 	{ params::value::S, true, 	{ "xxx", "yyy" }, 	NOMINMAX, 		{ "test1", "test2", "test3" } } },
	{ "ccc", 	{ params::value::F, false, 	{ "yyy" }, 			NOMINMAX, 		ALLALLOWED } },
	{ "ddd", 	{ params::value::I, true, 	{ "yyy", "zzz" }, 	NOMINMAX, 		ALLALLOWED } },
	{ "fff", 	{ params::value::S, false, 	{ "yyy", "xxx" }, 	NOMINMAX, 		ALLALLOWED } },
	{ "hhh", 	{ params::value::I, false, 	{ "!yyy" }, 		{ "1", "3" }, 	ALLALLOWED } },
	{ "iii", 	{ params::value::I, false, 	ALLFAMILIES, 		NOMINMAX, 		{ "1", "2", "5" } } },
};

#undef ALLFAMILIES 
#undef NOMINMAX 
#undef ALLALLOWED 

