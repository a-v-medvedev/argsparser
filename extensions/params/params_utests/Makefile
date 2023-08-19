# 
#  Copyright (c) 2020-2021 Alexey V. Medvedev
#  This code is licensed under 3-Clause BSD License.
#  See license.txt file for details.
# 

# Params extension unit tests

BASEPATH = ../../..
PARAMS_DIR = $(BASEPATH)/extensions/params
TARGETS = params_utest

override INCLUDES += -I$(BASEPATH) -I$(PARAMS_DIR)
override LIBS += -L$(BASEPATH) -largsparser -Wl,-rpath=$(BASEPATH)

override INCLUDES += -I$(BASEPATH)/yaml-cpp/include
override LIBS += -L$(BASEPATH)/yaml-cpp/lib -lyaml-cpp -Wl,-rpath=$(BASEPATH)/yaml-cpp/lib

override CXXFLAGS += -Wall -Wextra -std=c++11 $(INCLUDES)

all: $(TARGETS)

params_utest: params_utest.o
	$(CXX) params_utest.o -o params_utest $(LDFLAGS) $(LIBS)

params_utest.o: $(PARAMS_DIR)/params.h $(PARAMS_DIR)/params_override.h $(PARAMS_DIR)/dict.inl $(PARAMS_DIR)/list.inl $(PARAMS_DIR)/override.inl $(PARAMS_DIR)/value.inl $(PARAMS_DIR)/params.inl

.cpp.o:
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f params_utest params_utest.o
