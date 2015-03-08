/*
 * zobrist.h
 *
 *  Created on: Jun 10, 2014
 *      Author: yuu
 */

#ifndef RANDOM_HASH_
#define RANDOM_HASH_

#include <random>


template<typename D>
class RandomHash {
public:
	enum ABST {
		SINGLE = 0, PAIR = 1, LINE = 2, BLOCK = 3
	};

	RandomHash(int tnum_ = 1, ABST abst = SINGLE) :
			tnum(tnum_), round(0) {
		initHash();
	}

	RandomHash(D tnum_, ABST abst = SINGLE) :
			round(0) {
		initHash();
	}
	unsigned char inc_hash(const unsigned char previous, const int number,
			const int from, const int to, const char* const newBoard, const typename D::State s) {
		int id = round.fetch_add(1);
//		printf("id = %d\n", id % tnum);
		return id % tnum;
	}

	unsigned char inc_hash(typename D::State previous) {
		return 0;
	}

	unsigned char hash_tnum(const char* const board) {
		int id = round.fetch_add(1);
		return id % tnum;
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

	std::atomic<int> round;
};
#endif /* RANDOM_HASH_ */
