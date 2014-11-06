/*
 * zobrist.h
 *
 *  Created on: Jun 10, 2014
 *      Author: yuu
 */

#ifndef TRIVIAL_HASH_
#define TRIVIAL_HASH_

#include <bitset>
#include <random>

template <int size>
class TrivialHash {
public:
	enum ABST {SINGLE = 0, PAIR = 1, LINE = 2, BLOCK = 3};
//	typedef std::bitset<4> bits;
	// Should delete compatibility for performance.
	TrivialHash(int tnum_ = 1, ABST abst = SINGLE) :
			tnum(tnum_) {
		initHash();
	}
//	unsigned char inc_hash_tnum(const int number, const int from,
//			const int to, const char* const newBoard) {
//		return hash_tnum(newBoard);
//	}
	unsigned char inc_hash(unsigned char previous, const int number, const int from, const int to, const char* const newBoard) {
		return hash(newBoard) % tnum;
	}

	unsigned char hash_tnum(const char* const board) {
		return hash(board) % tnum;
	}
private:

	unsigned char hash(const char* const board) {
		unsigned char h = 0;
		for (int i = 0; i < size; ++i) {
			h = (h * 33) ^ board[i]; // Optimized to shift and add
		}
//		printf("row hash = %d\n", h);
		return h;
	}

	void initHash() {
	}

	int tnum;

};
#endif /* TRIVIAL_HASH_ */
