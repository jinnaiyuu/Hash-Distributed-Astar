/*
 * zobrist.h
 *
 *  Created on: Jun 10, 2014
 *      Author: yuu
 */

#ifndef STRIPS_ZOBRIST_H_
#define STRIPS_ZOBRIST_H_

#include <cmath>
#include <climits>
#include <cstdlib>
#include <math.h>

template<typename D>
class StripsZobrist {
public:
	// list abstraction strategies
	enum ABST {
	};

	// TODO: Should delete compatibility for performance.
	StripsZobrist(D &d, ABST abst = 0) :
			d(d) {
		structures = this->d.get_structures();
		xor_groups = this->d.get_xor_groups();

		if (abst == 2 && structures.size() < 8) {
			// use not structured predicates if structures are not enough.
			initZobrist(1);
		} else if (abst == 3 && structures.size() < 8) {
			initZobrist(1);
		} else {
			initZobrist(abst);
		}
	}

	/**
	 * TODO: incremental hash
	 * @param number: The number slided
	 * @param from : Where the number is in parent node
	 * @param to : Where the number is in child node
	 * @return the value to XOR to the Zobrist value of parent.
	 */
	unsigned int inc_hash(const unsigned int previous, const int number,
			const int from, const int to, const char* const newBoard,
			const typename D::State s) const {
		std::vector<unsigned int> p = s.propositions;
		unsigned int c = 0;
		for (unsigned int i = 0; i < p.size(); ++i) {
			c = c ^ map[p[i]];
		}
		return c;
	}

	unsigned int inc_hash(typename D::State s) const {
		return 0;
	}

//#define RANDOM_ZOBRIST_INITIALIZATION
private:
	void initZobrist(unsigned int abst) {
		map.resize(d.getGroundedPredicatesSize());
		std::fill(map.begin(), map.end(), 0);
		gen = std::mt19937(rd());
		dis = std::uniform_int_distribution<>(INT_MIN, INT_MAX);
#ifdef RANDOM_ZOBRIST_INITIALIZATION
		srand(time(NULL));
#endif

		// abst = 3: Abstraction. using structure. 1 key for 1 abstract state.
		// abst = 2: only using structure. TODO: what if no structure
		// abst = 1: using structure.
		// abst = 0: Zobrist Hash

		if (abst == 3) {
			abstraction();
		} else if (abst == 2) {
			strucutured_zobrist();
		} else if (abst == 1) {
			strucutured_zobrist();
			zobrist();
		} else if (abst == 0) {
			zobrist();
		}

	}

// Abstraction
// hash key for each abstract state.
	void abstraction() {
		// is there a great way to find the best abstraction here?
		unsigned int size = 1;
		for (unsigned int i = 0; i < xor_groups.size(); ++i) {
			size *= xor_groups[i].size();
			if (size > 16 * 16 * 16 * 16) { // ad hoc
				break;
			}
			for (unsigned int j = 0; j < xor_groups[i].size(); ++j) {
				map[xor_groups[i][j]] = random();
			}
		}
	}

// Structured Zobrist
// 1 key for each structure.
	void strucutured_zobrist() {
		for (unsigned int i = 0; i < structures.size(); ++i) {
			unsigned int r = random();
			for (unsigned int j = 0; j < structures[i].size(); ++j) {
				map[structures[i][j]] = r;
			}
		}

		for (unsigned int i = 0; i < xor_groups.size(); ++i) {
			for (unsigned int j = 0; j < xor_groups[i].size(); ++j) {
				unsigned int key = xor_groups[i][j];
				if (map[key] == 0) {
					map[key] = random();
				}
			}
		}


	}

	void zobrist() {
		for (unsigned int i = 0; i < map.size(); ++i) {
			if (map[i] == 0) {
				map[i] = random();
			}
		}
	}

// TODO: read automatic abstraction.
//	void abstraction(unsigned int abst) {
//		printf("abst = %u\n", abst);
//		if (abst == 0) {
//			++abst;
//		}
//		for (unsigned int i = 0; i < map.size(); ++i) {
//			if (i % abst == 0) {
//				map[i] = random();
//			} else {
//				map[i] = 0;
//			}
//		}
//	}

	unsigned int random() {
		return dis(gen) + INT_MAX;
	}

	D& d;
	std::vector<std::vector<unsigned int>> structures;
	std::vector<std::vector<unsigned int>> xor_groups;
//	unsigned int structure;

// TODO: ebable some kind of abstraction.
	std::vector<unsigned int> map;

	std::random_device rd;
	std::mt19937 gen;
	std::uniform_int_distribution<> dis;
};

#endif /* ZOBRIST_H_ */
