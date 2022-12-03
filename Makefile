# have to use C++ compiler due to SimConnect header using C++ features
CXX=x86_64-w64-mingw32-g++

.PHONY: all format

all: bin/flightmon.exe

bin/flightmon.exe: flightmon.c
	$(CXX) -o bin/flightmon flightmon.c -I include -L lib -l:SimConnect.dll

format:
	astyle --style=linux flightmon.c
