# Author: David Holmqvist <daae19@student.bth.se>

CXX=g++-13
# [I] Enables AVX optimizations
# [O3] Optimize for speed
CXXFLAGS=-std=c++17 -g -O3 -mavx -Wunused -Wall -Wunused -pthread

all: pearson verify pearson_par

pearson_par: vector dataset analysis_par pearson_par.cpp 
	$(CXX) $(CXXFLAGS) pearson_par.cpp vector.o dataset.o analysis_par.o -o pearson_par

pearson: vector dataset analysis pearson.cpp 
	$(CXX) $(CXXFLAGS) pearson.cpp vector.o dataset.o analysis.o -o pearson

analysis: vector analysis.hpp analysis.cpp
	$(CXX) $(CXXFLAGS) -c analysis.cpp -o analysis.o

analysis_par: vector analysis_par.hpp analysis_par.cpp
	$(CXX) $(CXXFLAGS) -c analysis_par.cpp -o analysis_par.o

dataset: vector dataset.hpp dataset.cpp
	$(CXX) $(CXXFLAGS) -c dataset.cpp -o dataset.o

vector: vector.hpp vector.cpp
	$(CXX) $(CXXFLAGS) -c vector.cpp -o vector.o

verify: verify.c
	$(CC) verify.c -o verify

clean:
	rm -rf verify pearson_par pearson *.o *.dSYM 2> /dev/null
