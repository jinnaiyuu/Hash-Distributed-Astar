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
		SINGLE = 1, FIVE = 5
	};

	// TODO: Should delete compatibility for performance.
	StripsZobrist(D &d, ABST abst = SINGLE) :
			d(d) {
		initZobrist(abst);
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
		map.resize(d.getActionSize());
		std::fill(map.begin(), map.end(), 0);
		gen = std::mt19937(rd());
		dis = std::uniform_int_distribution<>(INT_MIN, INT_MAX);
#ifdef RANDOM_ZOBRIST_INITIALIZATION
		srand(time(NULL));
#endif

		if (abst == 2) {
			strucutured_zobrist(abst);
		} else if (abst == 1){
			strucutured_zobrist(abst);
			zobrist();
		} else {
			zobrist();
		}

	}

	// Structured Zobrist
	void strucutured_zobrist(unsigned int abst) {
		std::vector<std::vector<unsigned int>> structures = d.get_structures();
		for (unsigned int i = 0; i < structures.size(); ++i) {
			int r = random();
			for (unsigned int j = 0; j < structures[i].size(); ++j) {
				map[structures[i][j]] = r;
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
//	unsigned int structure;

	// TODO: ebable some kind of abstraction.
	std::vector<unsigned int> map;

	std::random_device rd;
	std::mt19937 gen;
	std::uniform_int_distribution<> dis;
};

#endif /* ZOBRIST_H_ */
