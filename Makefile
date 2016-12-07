#  :copyright: (c) 2016 The Regents of the University of California.
#  :license: MIT, see LICENSE.md for more details.

# COMPILER
# --------

SHELL = /bin/sh
CXX = c++
AR = ar
RANLIB = ranlib

# FLAGS
# -----

SOURCES = utf.cpp
OBJECTS = $(SOURCES:.cpp=.o)
TEST_SOURCES = test.cpp
TEST_OBJECTS = $(TEST_SOURCES:.cpp=.o)
TESTS = $(TEST_SOURCES:.cpp=)
INCLUDE = .
CXXFLAGS  = -Wall -O3 -fPIC -std=c++11 -I$(INCLUDE)

# TARGETS
# -------

STATICLIB = libutf++.a
SHAREDLIB = libutf++.so

all: static shared tests
static: $(OBJECTS)
	$(AR) qc $(STATICLIB) $(OBJECTS)
	$(RANLIB) $(STATICLIB)
shared: $(OBJECTS)
	$(CXX) -o $(SHAREDLIB) $(CXXFLAGS) $(OBJECTS) -shared
tests: static $(TEST_OBJECTS)
	$(foreach example, $(TESTS), $(CXX) $(example).o -o $(example) $(CXXFLAGS) $(STATICLIB))

clean:
	rm -f $(OBJECTS) $(STATICLIB) $(SHAREDLIB) $(TEST_OBJECTS) $(TESTS)
