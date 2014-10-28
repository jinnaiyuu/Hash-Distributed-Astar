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
//	typedef std::bitset<4> bits;
	// Should delete compatibility for performance.
	TrivialHash(int tnum_ = 1) :
			tnum(tnum_) {
		initHash();
	}

	int hash(const char* const board) {
		int h = 0;
		for (int i = 0; i < size; ++i) {
			h = h * 17 + board[i];
		}
		return h;
	}

private:
	void initHash() {
	}

	int tnum;

};
#endif /* TRIVIAL_HASH_ */
