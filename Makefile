#
# Copyright (c) 2018-2024 Alexey V. Medvedev
# This code is an extension of the parts of Intel(R) MPI Benchmarks project.
# It keeps the same 3-Clause BSD License.
#

SHELL := /bin/bash
UNAME=$(word 1,$(subst -, ,$(shell uname -s)))

SHORT_NAME = argsparser
STATIC_LIB = lib$(SHORT_NAME).a
ifeq ($(UNAME),CYGWIN_NT)
SHARED_LIB = cyg$(SHORT_NAME).dll
SHARED_LIB_A = lib$(SHORT_NAME).dll.a
else
SHARED_LIB = lib$(SHORT_NAME).so
endif

YAML_DIR ?= yaml-cpp
WITH_STATIC_LIB ?= TRUE
WITH_SHARED_LIB ?= TRUE

CFLAGS_OPT ?= -O2
#CFLAGS_OPT = -O0 -g
#CFLAGS_OPT = -O0 -g --coverage

all: libs install

ifeq ($(WITH_SHARED_LIB),TRUE)
libs: $(SHARED_LIB)
endif
ifeq ($(WITH_STATIC_LIB),TRUE)
libs: $(STATIC_LIB)
endif

override CXXFLAGS += -fPIC $(CFLAGS_OPT) -I. -I$(YAML_DIR)/include  -Wall -Wextra -pedantic -std=c++11 -D_GNU_SOURCE
override LDFLAGS = -L$(YAML_DIR)/lib -lyaml-cpp
#override LDFLAGS += -lgcov

LIBOBJS = argsparser.o argsparser_iface.o

$(STATIC_LIB): $(LIBOBJS)
	ar rcs $@ $^


ifeq ($(UNAME),CYGWIN_NT)
$(SHARED_LIB): $(LIBOBJS)
	$(CXX) -shared -o $@ -Wl,--out-implib -Wl,$(SHARED_LIB_A) -Wl,--export-all-symbols -Wl,--enable-auto-image-base  $^ $(LDFLAGS)
else
$(SHARED_LIB): $(LIBOBJS)
	$(CXX) -shared -Wl,-soname,$@ -o $@ $^ $(LDFLAGS)
endif


install: libs
	@echo Completing $(SHORT_NAME) install:
	@mkdir -p argsparser
	@mkdir -p argsparser/include
	@mkdir -p argsparser/extensions
	@[ "$(WITH_STATIC_LIB)" == "TRUE" ] && cp -v $(STATIC_LIB) argsparser || true
	@[ "$(WITH_SHARED_LIB)" == "TRUE" ] && cp -v $(SHARED_LIB) argsparser || true
	@[ -z "$(SHARED_LIB_A)" ] || cp -v $(SHARED_LIB_A) argsparser
	@cp -v argsparser_iface.h argsparser/include
	@cp -v argsparser.h argsparser/include
	@cp -rv extensions/params argsparser/extensions

argsparser_utests: argsparser_utests.o
	$(CXX) $(CXXFLAGS) -o $@ $^ -L. -L$(YAML_DIR)/lib -largsparser -lyaml-cpp

.cpp.o:
	$(CXX) $(CXXFLAGS) -c -o $@ $^

clean:
	rm -f $(LIBOBJS) $(STATIC_LIB) $(SHARED_LIB) argsparser_utests.o

