CXX=clang++-3.5
CXXFLAGS=-g -Wall -Wno-unused-value -Wno-char-subscripts -static -pthread -std=c++11 -O3
PROFFLAGS=-pg -static -pthread -std=c++11
MASSFLAGS=-g -pthread -std=c++11

# -g -Wall
all: strips.out
prof: pstrips.out

tiles: *.cc *.hpp msa/*.cc msa/*.hpp
	$(CXX) $(CXXFLAGS) *.cc msa/*.cc -o tiles 

#prof: *.cc *.hpp msa/*.cc msa/*.hpp strips/*.cc strips/*.hpp
#	$(CXX) $(PROFFLAGS) *.cc msa/*.cc strips/*.cc -o ptiles

#mass: *.cc *.hpp msa/*.cc msa/*.hpp strips/*.cc strips/*.hpp
#	$(CXX) $(MASSFLAGS) *.cc msa/*.cc strips/*.cc -o mtiles

strips.out: *.cc *.hpp strips/*.cc strips/*.hpp
	$(CXX) $(CXXFLAGS) *.cc strips/*.cc -o strips.out

pstrips.out: *.cc *.hpp strips/*.cc strips/*.hpp
	$(CXX) $(PROFFLAGS) *.cc strips/*.cc -o pstrips.out


clean:
	rm -fr *.o tiles ptiles mtiles strips.out
