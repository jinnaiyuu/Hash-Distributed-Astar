CXX=g++
CXXFLAGS=-Wall -static -lboost_system -pthread -std=c++11 -O3

all: tiles

tiles: *.cc *.hpp
	$(CXX) $(CXXFLAGS) *.cc -o tiles

clean:
	rm -fr *.o tiles tiles_solver
