/*
 * zobrist.h
 *
 *  Created on: Jun 10, 2014
 *      Author: yuu
 */

#ifndef ZOBRIST_H_
#define ZOBRIST_H_

template <int size>
class Zobrist {
public:
	// Should delete compatibility for performance.
	Zobrist(int tnum_ = 1) :
			tnum(tnum_) {
		initZobrist();
	}

	/**
	 * TODO: Incremental hash value
	 *
	 * @param number: The number slided
	 * @param from  : Where the number is in parent node
	 * @param to    : Where the number is in child node
	 * @return the value to XOR to the Zobrist value of parent.
	 */
	unsigned char inc_hash_tnum(const int number, const int from, const int to) {
		return inc_zbr[number][from][to] % tnum;
	}

	unsigned char inc_hash(const int number, const int from, const int to) {
		return inc_zbr[number][from][to];
	}

	char hash_tnum(const char* const board) {
		char h = hash(board) % tnum;
		if (h < 0) {
			return tnum + h;
		}
		return h;
	}

	// The method to return zobrist value for the very first node.
	char hash(const char* const board) {
		unsigned char h = '\0';
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

	// Currently hard coding, set to 16.
	int tnum;
	unsigned char zbr[size][size];

	// inc_zbr is the incremental XOR value for zobrist hash function.
	// The value to XOR when the number moved from a to b is
	// inc_zbr[number][a][b]  or inc_zbr[number][b][a]
	unsigned char inc_zbr[size][size][size];

	// the value to XOR to the zbr value.
	// Slide
	//	int zbrincr[16][16];
};
#endif /* ZOBRIST_H_ */
