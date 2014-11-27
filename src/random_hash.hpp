/*
 * zobrist.h
 *
 *  Created on: Jun 10, 2014
 *      Author: yuu
 */

#ifndef RANDOM_HASH_
#define RANDOM_HASH_

#include <random>


template<int size>
class RandomHash {
public:
	enum ABST {
		SINGLE = 0, PAIR = 1, LINE = 2, BLOCK = 3
	};

	RandomHash(int tnum_ = 1, ABST abst = SINGLE) :
			tnum(tnum_) {
		initHash();
	}

	unsigned char inc_hash(unsigned char previous, const int number,
			const int from, const int to, const char* const newBoard) {
		return hash() % tnum;
	}

	unsigned char hash_tnum(const char* const board) {
		return hash() % tnum;
	}
private:

	unsigned char hash() {
		unsigned char r = static_cast<unsigned char>(rand());
		unsigned char rtnum = r % tnum;
//		printf("rand mod tnum = %u \n", rtnum);
		return r;
	}

	void initHash() {
		gen = std::mt19937(rd());
		dis = std::uniform_int_distribution<>(0, tnum - 1);
	}

	std::random_device rd;
	std::mt19937 gen;
	std::uniform_int_distribution<> dis;
	int tnum;

};
#endif /* RANDOM_HASH_ */
