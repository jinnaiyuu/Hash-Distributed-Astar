CXX=clang++
CXXFLAGS=-g -Wall -Wno-unused-value -Wno-char-subscripts -static -pthread -std=c++11 -O3
# -g -Wall
all: tiles

tiles: *.cc *.hpp
	$(CXX) $(CXXFLAGS) *.cc -o tiles

clean:
	rm -fr *.o tiles tiles_solver
