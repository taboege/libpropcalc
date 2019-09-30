CXX = g++
CC = gcc
CPPFLAGS += -I.
CXXFLAGS += -std=c++11 -Wall -Werror -O0 -ggdb
CFLAGS += -Wall -Werror -O0 -ggdb

libpropcalc.so: ast.o formula.o
libpropcalc.so: propcalc.o
	$(CXX) $(CXXFLAGS) -o $@ -shared $(LDFLAGS) $^

%.o: %.cpp %.hpp propcalc.hpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ -fPIC $<

propcalc.cpp: propcalc.h

cpptest: cpptest.cpp libpropcalc.so
	$(CXX) $(CPPFLAGS) $(CFLAGS) -o $@ $< -L. -lpropcalc

ctest: ctest.c libpropcalc.so
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $< -L. -lpropcalc

test:
	@echo make cpptest and ctest, run them, check the output
	@echo check that they have no memory leaks
	@echo actual tests to come in the perl library

.PHONY: clean
clean:
	rm -f *.o libpropcalc.so cpptest ctest
