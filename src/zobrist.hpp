/*
 * zobrist.h
 *
 *  Created on: Jun 10, 2014
 *      Author: yuu
 */

#ifndef ZOBRIST_H_
#define ZOBRIST_H_

#include <bitset>
#include <random>

template <int size>
class Zobrist {
public:
	typedef std::bitset<4> bits;
	// Should delete compatibility for performance.
	Zobrist(int tnum_ = 1) :
			tnum(tnum_) {
		initZobrist();
	}

	bits inc_hash(const int number, const int from, const int to) {
		return inc_zbr[number][from][to];
	}

	// The method to return zobrist value for the very first node.
	bits hash(const char* const board) {
		bits h(0);
		for (int i = 0; i < 16; ++i) {
			h = (h ^ zbr[board[i]][i]);
		}
		return h;
	}

//#define RANDOM_ZOBRIST_INITIALIZATION

private:
	void initZobrist() {
		// Not sure I should initialize it by time as it randomize the results for each run.
#ifdef RANDOM_ZOBRIST_INITIALIZATION
		srand(time(NULL));
#endif
		for (int j = 0; j < size; ++j) {
			zbr[0][j] = 0;
		}
		for (int i = 1; i < size; ++i) {
			for (int j = 0; j < size; ++j) {
				zbr[i][j] = rand();
//				printf("table[%d][%d] = %d\n", i, j, zbr[i][j]);
			}
		}

		for (int i = 1; i < size; ++i) { // num
			for (int j = 0; j < size; ++j) { // from
				for (int k = 0; k < size; ++k) { // to
					inc_zbr[i][j][k] = zbr[i][j] ^ zbr[i][k];
				}
			}
		}
	}

	bits random_bitset(double p = 0.5) {
		bits bitset;
		std::random_device rd;
		std::mt19937 gen(rd());
		std::bernoulli_distribution d(p);

		for (int n = 0; n < size; ++n) {
			bitset[n] = d(gen);
		}

		return bitset;
	}


// Currently hard coding, set to 16.
int tnum;
bits zbr[size][size];

// inc_zbr is the incremental XOR value for zobrist hash function.
// The value to XOR when the number moved from a to b is
// inc_zbr[number][a][b]  or inc_zbr[number][b][a]
bits inc_zbr[size][size][size];

// the value to XOR to the zbr value.
// Slide
//	int zbrincr[16][16];
};
#endif /* ZOBRIST_H_ */
