#
# Copyright (c) 2018-2020 Alexey V. Medvedev
# This code is an extension of the parts of Intel(R) MPI Benchmarks project.
# It keeps the same Community Source License (CPL) license.
#

SHORT_NAME = argsparser
STATIC_LIB = lib$(SHORT_NAME).a
SHARED_LIB = lib$(SHORT_NAME).so

YAML_DIR ?= yaml-cpp

CFLAGS_OPT ?= -O2
#CFLAGS_OPT = -O0 -g
#CFLAGS_OPT = -O0 -g --coverage

all: libs install

libs: $(STATIC_LIB) $(SHARED_LIB)

override CXXFLAGS += -fPIC $(CFLAGS_OPT) -I. -I$(YAML_DIR)/include  -Wall -Wextra -pedantic -std=c++11 -D_GNU_SOURCE
override LDFLAGS = -L$(YAML_DIR)/lib -lyaml-cpp
#override LDFLAGS += -lgcov

LIBOBJS = argsparser.o argsparser_iface.o

$(STATIC_LIB): $(LIBOBJS)
	ar rcs $@ $^

$(SHARED_LIB): $(LIBOBJS)
	$(CXX) -shared -Wl,-soname,$@ -o $@ $^ $(LDFLAGS)

install: libs
	@echo Doing install
	@mkdir -p argsparser
	@mkdir -p argsparser/include
	@mkdir -p argsparser/extensions
	@cp $(SHARED_LIB) $(STATIC_LIB) argsparser
	@cp argsparser_iface.h argsparser/include
	@cp argsparser.h argsparser/include
	@cp -r extensions/params argsparser/extensions

argsparser_utests: argsparser_utests.o
	$(CXX) $(CXXFLAGS) -o $@ $^ -L. -L$(YAML_DIR)/lib -largsparser -lyaml-cpp

.cpp.o:
	$(CXX) $(CXXFLAGS) -c -o $@ $^

clean:
	rm -f $(LIBOBJS) $(STATIC_LIB) $(SHARED_LIB) argsparser_utests.o

