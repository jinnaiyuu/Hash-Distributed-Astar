/*
 * zobrist.h
 *
 *  Created on: Jun 10, 2014
 *      Author: yuu
 */

#ifndef TSP_HASH_
#define TSP_HASH_

#include "tsp.hpp"
#include <bitset>
#include <random>
#include <vector>

template<typename D>
class TspHash {
public:
	enum ABST {
		SINGLE = 0, PAIR = 1, LINE = 2, BLOCK = 3
	};

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

	TspHash(Tsp &d, ABST structure = 1) :
			structure(structure) {
		if (this->structure == 0) {
			printf("structure = 1\n");
			this->structure = 1;
		}

		// TODO: for now signal which abstraction to use by >1000.
		if (this->structure > 1000) {
			this->structure -= 1000;
			init_zbr();
		} else {
			init_structure_zbr(d);
		}
	}

	~TspHash() {
		delete zbr;
		delete structure_map;
	}

	unsigned char inc_hash(const unsigned char previous, const int number,
			const int from, const int to, const char* const newBoard,
			const typename D::State s) const {
		if (is_structure) {
			bool visited_blocks[number_of_blocks]; // not sure how this works
			for (unsigned int i = 0; i < number_of_blocks; ++i) {
				visited_blocks[i] = false;
			}
			for (unsigned int i = 0; i < s.visited.size(); ++i) {
				if (s.visited[i]) {
					visited_blocks[structure_map[i]] = true;
				}
			}
			unsigned int z = 0;
			for (unsigned int i = 0; i < number_of_blocks; ++i) {
				if (visited_blocks[i]) {
//					printf("visited %u\n", i);
					z = z ^ zbr[i];
				}
			}
//			printf("z = %u\n", z);
//			delete visited_blocks;
			return z;
		} else {
			unsigned int z = 0;
			for (unsigned int i = 0; i < s.visited.size(); ++i) {
				if (s.visited[i]) {
					z = z ^ zbr[i];
				}
			}
			return z;
		}
	}

	unsigned char inc_hash(typename D::State s) {
		return 0;
	}

	unsigned char hash_tnum(const char* const board) {
		return 0;
	}
private:

	void init_zbr() {
		is_structure = false;
		printf("tsp_hash init_zbr cities = %u\n", number_of_cities);
		printf("structure = %u\n", structure);

		zbr = new unsigned int[number_of_cities];
		// a bit not optimized but for more clarity.
		for (unsigned int i = 0; i < number_of_cities; ++i) {
			zbr[i] = 0;
		}

		for (unsigned int i = 0; i < number_of_cities; i += structure) {
			zbr[i] = rand();
		}
		printf("zbrend\n");
	}

	void init_structure_zbr(D &d) {
		number_of_cities = d.get_number_of_cities();
		const std::vector<unsigned int> distances = d.get_distances();

		is_structure = true;
		printf("tsp_hash init_zbr cities = %u\n", number_of_cities);
		printf("nearest neighbor abstraction\n");
		printf("structure = %u\n", structure);

		zbr = new unsigned int[number_of_cities];
		number_of_blocks = number_of_cities / structure;
		structure_map = new unsigned int[number_of_cities];




		for (int t = 0; t < number_of_cities; t++) {
			for (int f = 0; f < number_of_cities; f++) {
				printf("%u ", distances[f * number_of_cities + t]);
			}
			printf("\n");
		}

		bool is_mapped[number_of_cities];
		for (unsigned int i = 0; i < number_of_cities; ++i) {
			is_mapped[i] = false;
		}


		unsigned int st = 0;
		for (unsigned int i = 0; i < number_of_cities; ++i) {
			if (!is_mapped[i]) {
				is_mapped[i] = true;
				unsigned int min_length = 1000000;
				unsigned int neighbor1, neighbor2;
				for (unsigned int j = 0; j < number_of_cities; ++j) {
					if (!is_mapped[j] && distances[i * number_of_cities + j] < min_length) {
						min_length = distances[i * number_of_cities + j];
						neighbor1 = j;
					}
				}
				is_mapped[neighbor1] = true;

				min_length = 1000000;
				for (unsigned int j = 0; j < number_of_cities; ++j) {
					if (!is_mapped[j] && distances[i * number_of_cities + j] < min_length) {
						min_length = distances[i * number_of_cities + j];
						neighbor2 = j;
					}
				}
				is_mapped[neighbor2] = true;

				structure_map[i] = st;
				structure_map[neighbor1] = st;
				structure_map[neighbor2] = st;
				++st;
			}
		}

		printf("map: ");
		for (unsigned int i = 0; i < number_of_cities; ++i) {
			printf("%u ", structure_map[i]);
		}
		printf("\n");

		// TODO: here we need to implement nearest neighbors.
//		for (unsigned int i = 0; i < number_of_blocks; ++i) {
//			for (unsigned int j = 0; j < structure; ++j) {
//				structure_map[i * structure + j] = i;
//			}
//		}

		for (unsigned int i = 0; i < number_of_blocks; ++i) {
			zbr[i] = rand();
		}
		printf("zbrend\n");
	}

	unsigned int number_of_cities = 1000;
	unsigned int structure;

	bool is_structure;
	unsigned int number_of_blocks;
	unsigned int* structure_map;

	unsigned int* zbr;

};
#endif /* TRIVIAL_HASH_ */
