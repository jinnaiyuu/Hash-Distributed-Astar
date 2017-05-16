CXX=clang++-3.5
MPI_CXX=mpic++

CXXFLAGS=-g -Wall -Wno-unused-value -Wno-char-subscripts -pthread -std=c++11 -O3 -static 
MPI_CXXFLAGS=-g -Wall -Wno-unused-value -Wno-char-subscripts -std=c++11 -O3 #-static
PROFFLAGS=-pg -pthread -std=c++11 -static 
MASSFLAGS=-g -pthread -std=c++11




# -g -Wall
all: strips.out tiles tiles_mpi strips_mpi
prof: pstrips.out

tiles: main/main_tiles.cc *.cc *.hpp 
	$(CXX) $(CXXFLAGS) main/main_tiles.cc *.cc -o tiles -I${JEMALLOC_PATH}/include -L${JEMALLOC_PATH}/lib -Wl,-rpath,${JEMALLOC_PATH}/lib -ljemalloc

msa: main/main_tiles.cc *.cc *.hpp msa/*.hpp 
	$(CXX) $(CXXFLAGS) main/main_msa.cc msa/*.cc *.cc -o msa.out -I${JEMALLOC_PATH}/include -L${JEMALLOC_PATH}/lib -Wl,-rpath,${JEMALLOC_PATH}/lib -ljemalloc

tiles_mpi: main/main_comb_mpi.cc mpi/*.hpp *.cc *.hpp 
	$(MPI_CXX) $(MPI_CXXFLAGS) main/main_comb_mpi.cc *.cc -o tiles_mpi -I${JEMALLOC_PATH}/include -L${JEMALLOC_PATH}/lib -L/usr/lib -Wl,-rpath,${JEMALLOC_PATH}/lib -ljemalloc 

#prof: *.cc *.hpp msa/*.cc msa/*.hpp strips/*.cc strips/*.hpp
#	$(CXX) $(PROFFLAGS) *.cc msa/*.cc strips/*.cc -o ptiles

#mass: *.cc *.hpp msa/*.cc msa/*.hpp strips/*.cc strips/*.hpp
#	$(CXX) $(MASSFLAGS) *.cc msa/*.cc strips/*.cc -o mtiles

strips.out: main/main_strips.cc *.cc *.hpp strips/*.cc strips/*.hpp
	$(CXX) $(CXXFLAGS) main/main_strips.cc *.cc strips/*.cc -o strips.out

strips_mpi: main/main_planning_mpi.cc mpi/*.hpp *.cc *.hpp strips/*.cc strips/*.hpp
	$(MPI_CXX) $(MPI_CXXFLAGS) main/main_planning_mpi.cc  *.cc  strips/*.cc -o strips_mpi -I${JEMALLOC_PATH}/include -L${JEMALLOC_PATH}/lib -L/usr/lib -Wl,-rpath,${JEMALLOC_PATH}/lib -ljemalloc 

parser: strips/utils.hpp strips/parser/*.cc
	$(CXX) $(CXXFLAGS) strips/parser/*.cc -o parser 

pstrips.out: strips/*.cc strips/*.hpp
	$(CXX) $(PROFFLAGS) *.cc strips/*.cc -o pstrips.out

tiles_d: main/main_tiles.cc *.cc *.hpp 
	$(CXX) $(CXXFLAGS) -DDEBUG main/main_tiles.cc *.cc -o tiles_d -I${JEMALLOC_PATH}/include -L${JEMALLOC_PATH}/lib -Wl,-rpath,${JEMALLOC_PATH}/lib -ljemalloc


clean:
	rm -fr *.o tiles ptiles mtiles strips.out tiles_mpi
