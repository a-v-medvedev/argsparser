# params unit tests

BASEPATH = ..
TARGETS = params_utest

INCLUDES += -I$(BASEPATH) -I$(BASEPATH)/param
LIBS += -L$(BASEPATH) -largsparser -Wl,-rpath=$(BASEPATH)

INCLUDES += -I$(BASEPATH)/yaml-cpp/include
LIBS += -L$(BASEPATH)/yaml-cpp/lib -lyaml-cpp -Wl,-rpath=$(BASEPATH)/yaml-cpp/lib

CXXFLAGS += -Wall -Wextra -std=c++11 $(INCLUDES)

all: $(TARGETS)

params_utest: params_utest.o
	$(CXX) params_utest.o -o params_utest $(LDFLAGS) $(LIBS)

#params_utest.o: $(BASEPATH)/param/params.h $(BASEPATH)/param/params_override.h $(BASEPATH)/param/dict.inl $(BASEPATH)/param/list.inl $(BASEPATH)/param/override.inl $(BASEPATH)/param/value.inl

params_utest.o: params_utest.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $^

clean:
	rm -f params_utest params_utest.o
