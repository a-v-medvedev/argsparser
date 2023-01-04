# 
#  Copyright (c) 2020-2021 Alexey V. Medvedev
#  This code is licensed under 3-Clause BSD License.
#  See license.txt file for details.
# 

# Params extension unit tests

BASEPATH = thirdparty
PARAMS_DIR = $(BASEPATH)/argsparser.bin/extensions/params
TARGETS = example

override INCLUDES += -I$(PARAMS_DIR)
override LIBS += -L$(BASEPATH)/argsparser.bin -largsparser -Wl,-rpath=$(BASEPATH)/argsparser.bin

override INCLUDES += -I$(BASEPATH)/yaml-cpp.bin/include
override LIBS += -L$(BASEPATH)/yaml-cpp.bin/lib -lyaml-cpp -Wl,-rpath=$(BASEPATH)/yaml-cpp.bin/lib

override CXXFLAGS += -Wall -Wextra -std=c++11 -O0 -g $(INCLUDES)

all: $(TARGETS)

example: example.o
	$(CXX) example.o -o example $(LDFLAGS) $(LIBS)

example.o: $(PARAMS_DIR)/params.h $(PARAMS_DIR)/params_override.h $(PARAMS_DIR)/dict.inl $(PARAMS_DIR)/list.inl $(PARAMS_DIR)/override.inl $(PARAMS_DIR)/value.inl $(PARAMS_DIR)/params.inl example_details.h Makefile

.cpp.o:
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f example example.o
