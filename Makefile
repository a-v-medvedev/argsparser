#
# Copyright (c) 2018-2019 Alexey V. Medvedev
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

all: $(STATIC_LIB) $(SHARED_LIB) install

override CXXFLAGS += -fPIC $(CFLAGS_OPT) -I. -I$(YAML_DIR)/include  -Wall -Wextra -pedantic -std=c++11 
#override LDFLAGS += -lgcov

LIBOBJS = argsparser.o argsparser_iface.o

$(STATIC_LIB): $(LIBOBJS)
	ar rcs $@ $^

$(SHARED_LIB): $(LIBOBJS)
	$(CXX) -shared -Wl,-soname,$@ -o $@ $^ $(LDFLAGS)

install: $(STATIC_LIB) $(SHARED_LIB)
	@mkdir -p argsparser
	@mkdir -p argsparser/include
	@mkdir -p argsparser/extensions
	@cp $(SHARED_LIB) argsparser
	@cp argsparser_iface.h argsparser/include
	@cp argsparser.h argsparser/include
	@cp -r extensions/params argsparser/exteansions

argsparser_utests: argsparser_utests.o
	$(CXX) $(CXXFLAGS) -o $@ $^ -L. -L$(YAML_DIR)/lib -largsparser -lyaml-cpp

.cpp.o:
	$(CXX) $(CXXFLAGS) -c -o $@ $^

clean:
	rm -f $(OBJS) $(LIBOBJS) $(STATIC_LIB) $(SHARED_LIB) argsparser_utests.o

