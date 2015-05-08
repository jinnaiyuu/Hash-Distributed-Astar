CXX=clang++-3.5
CXXFLAGS=-g -Wall -Wno-unused-value -Wno-char-subscripts -static -pthread -std=c++11 -O3
# -g -Wall
all: tiles

tiles: *.cc *.hpp msa/*.cc msa/*.hpp strips/*.cc strips/*.hpp
	$(CXX) $(CXXFLAGS) *.cc msa/*.cc strips/*.cc -o tiles

clean:
	rm -fr *.o tiles tiles_solver
