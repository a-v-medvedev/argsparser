# 
#  Copyright (c) 2020 Alexey V. Medvedev
#  This code is licensed under Community Source License v.1 (CPL v.1) license.
#  See license.txt file for details.
# 

# Params extension unit tests

BASEPATH = ../..
PARAMS_DIR = $(BASEPATH)/extensions/params
TARGETS = params_utest

INCLUDES += -I$(BASEPATH) -I$(PARAMS_DIR)
LIBS += -L$(BASEPATH) -largsparser -Wl,-rpath=$(BASEPATH)

INCLUDES += -I$(BASEPATH)/yaml-cpp/include
LIBS += -L$(BASEPATH)/yaml-cpp/lib -lyaml-cpp -Wl,-rpath=$(BASEPATH)/yaml-cpp/lib

CXXFLAGS += -Wall -Wextra -std=c++11 $(INCLUDES)

all: $(TARGETS)

params_utest: params_utest.o
	$(CXX) params_utest.o -o params_utest $(LDFLAGS) $(LIBS)

params_utest.o: $(PARAMS_DIR)/params.h $(PARAMS_DIR)/params_override.h $(PARAMS_DIR)/dict.inl $(PARAMS_DIR)/list.inl $(PARAMS_DIR)/override.inl $(PARAMS_DIR)/value.inl $(PARAMS_DIR)/params.inl

.cpp.o:
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f params_utest params_utest.o
