/*
 * zobrist.h
 *
 *  Created on: Jun 10, 2014
 *      Author: yuu
 */

#ifndef GRID_HASH_
#define GRID_HASH_

#include <bitset>
#include <random>

template <typename D>
class GridHash {
public:
	enum ABST {SINGLE = 0, PAIR = 1, LINE = 2, BLOCK = 3};

	// TODO: Note here is a hack.
	// I'm using ABST just as a unsigned integer, not enum.
	// This should be refactored afterall.
	GridHash(int tnum_ = 1, ABST structure = 1) :
			structure(structure) {}

	GridHash(D tnum_, ABST structure = 1) :
			structure(structure) {}

	unsigned char inc_hash(const unsigned char previous, const int number,
			const int from, const int to, const char* const newBoard, const typename D::State s) {
		// from = x * 100000 + y
		unsigned int x = from / 100000;
		unsigned int y = from % 100000;
		return ((x / structure) * 23 + (y / structure));
	}

	unsigned char inc_hash(typename D::State s) {
		return s.x * 19 + s.y;
	}

	unsigned char hash_tnum(const char* const board) {
		return 0;
	}
private:

	int tnum;
	unsigned int structure;

};
#endif /* TRIVIAL_HASH_ */
