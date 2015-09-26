CXX=clang++-3.5
MPI_CXX=mpic++

CXXFLAGS=-g -Wall -Wno-unused-value -Wno-char-subscripts -pthread -std=c++11 -O3 -static 
MPI_CXXFLAGS=-g -Wall -Wno-unused-value -Wno-char-subscripts -std=c++11 -O3 #-static
PROFFLAGS=-pg -pthread -std=c++11 -static 
MASSFLAGS=-g -pthread -std=c++11




# -g -Wall
all: strips.out tiles
prof: pstrips.out

tiles: main_comb/main_comb.cc *.cc *.hpp msa/*.cc msa/*.hpp
	$(CXX) $(CXXFLAGS) main_comb/*.cc *.cc msa/*.cc -o tiles -I${JEMALLOC_PATH}/include -L${JEMALLOC_PATH}/lib -Wl,-rpath,${JEMALLOC_PATH}/lib -ljemalloc

tiles_mpi: mpi/*.cc mpi/*.hpp *.cc *.hpp 
	$(MPI_CXX) $(MPI_CXXFLAGS) mpi/mpi_main_comb.cc *.cc -o tiles_mpi -I${JEMALLOC_PATH}/include -L${JEMALLOC_PATH}/lib -L/usr/lib -Wl,-rpath,${JEMALLOC_PATH}/lib -ljemalloc 

#prof: *.cc *.hpp msa/*.cc msa/*.hpp strips/*.cc strips/*.hpp
#	$(CXX) $(PROFFLAGS) *.cc msa/*.cc strips/*.cc -o ptiles

#mass: *.cc *.hpp msa/*.cc msa/*.hpp strips/*.cc strips/*.hpp
#	$(CXX) $(MASSFLAGS) *.cc msa/*.cc strips/*.cc -o mtiles

strips.out: *.cc *.hpp strips/*.cc strips/*.hpp
	$(CXX) $(CXXFLAGS) *.cc strips/*.cc -o strips.out


parser: strips/utils.hpp strips/parser/*.cc
	$(CXX) $(CXXFLAGS) strips/parser/*.cc -o parser 

pstrips.out: strips/*.cc strips/*.hpp
	$(CXX) $(PROFFLAGS) *.cc strips/*.cc -o pstrips.out


clean:
	rm -fr *.o tiles ptiles mtiles strips.out tiles_mpi
