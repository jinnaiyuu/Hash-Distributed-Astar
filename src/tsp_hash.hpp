/*
 * zobrist.h
 *
 *  Created on: Jun 10, 2014
 *      Author: yuu
 */

#ifndef TSP_HASH_
#define TSP_HASH_

#include <bitset>
#include <random>

template <typename D>
class TspHash {
public:
	enum ABST {SINGLE = 0, PAIR = 1, LINE = 2, BLOCK = 3};

	// TODO: Note here is a hack.
	// I'm using ABST just as a unsigned integer, not enum.
	// This should be refactored afterall.
	TspHash(unsigned int number_of_cities = 1000, ABST structure = 1) :
			structure(structure) {
		if (this->structure == 0) {
			printf("structure = 1\n");
			this->structure = 1;
		}
		init_zbr();

	}

	~TspHash(){
		delete zbr;
	}

	unsigned char inc_hash(const unsigned char previous, const int number,
			const int from, const int to, const char* const newBoard, const typename D::State s) const {
		// from = x * 100000 + y
		unsigned int z = 0;
		for (unsigned int i = 0; i < s.visited.size(); ++i) {
			if (s.visited[i]) {
				z = z ^ zbr[i];
			}
		}
		return z;
	}

	unsigned char inc_hash(typename D::State s) {
		return 0;
	}

	unsigned char hash_tnum(const char* const board) {
		return 0;
	}
private:

	void init_zbr() {
		printf("tsp_hash init_zbr cities = %u\n", number_of_cities);
		printf("structure = %u\n", structure);

		zbr = new unsigned int[number_of_cities];
		// a bit not optimized but for more clarity.
		for (unsigned int i = 0; i < number_of_cities; ++i) {
			zbr[i] = 0;
		}

		for (unsigned int i = 0; i < number_of_cities; i+= structure) {
			zbr[i] = rand();
		}
		printf("zbrend\n");
	}

	unsigned int number_of_cities = 1000;
	unsigned int structure;

	unsigned int* zbr;

};
#endif /* TRIVIAL_HASH_ */
