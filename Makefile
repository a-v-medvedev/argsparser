#
# Copyright (c) 2018 Alexey V. Medvedev
# This code is an extension of the parts of Intel MPI benchmarks project.
# It keeps the same Community Source License (CPL) license.
#

SHORT_NAME = argsparser
STATIC_LIB = lib$(SHORT_NAME).a
SHARED_LIB = lib$(SHORT_NAME).so

YAML_DIR ?= yaml-cpp

CFLAGS_OPT = -O2

all: $(STATIC_LIB) $(SHARED_LIB) install

override CXXFLAGS += -fPIC $(CFLAGS_OPT) -I. -I$(YAML_DIR)/include  -Wall -Wextra -pedantic -std=c++11 
override LDFLAGS += $(CFLAGS_OPT)

LIBOBJS = argsparser.o argsparser_utests.o argsparser_iface.o

$(STATIC_LIB): $(LIBOBJS)
	ar rcs $@ $^

$(SHARED_LIB): $(LIBOBJS)
	$(CXX) -shared -Wl,-soname,$@ -o $@ $^

install: $(STATIC_LIB) $(SHARED_LIB)
	@mkdir -p argsparser
	@mkdir -p argsparser/include
	@cp $(SHARED_LIB) argsparser
	@cp argsparser_iface.h argsparser/include
	@cp argsparser.h argsparser/include


.cpp.o:
	$(CXX) -I. $(CXXFLAGS) -c -o $@ $^

clean:
	rm -f $(OBJS) $(LIBOBJS) $(STATIC_LIB) $(SHARED_LIB)

