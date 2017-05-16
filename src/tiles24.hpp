// Copyright 2012 Ethan Burns. All rights reserved.
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file.
#include "search.hpp"
#include "fatal.hpp"
#include "hashtbl.hpp"
#include "zobrist.hpp"
#include "dist_hash.hpp"
#include <cstdio>
#include <cstdlib>
#include <cassert>

// Exact Table size for 24 puzzle's pattern databases.
#define TABLESIZE 244140625

// Enable 128 bit integer. It is not the optimal way to implement.
#include <stdint.h>
typedef unsigned int uint128_t __attribute__((mode(TI)));

struct Tiles24 {
	enum {
		Width = 5, Height = 5, Ntiles = Width * Height,
	};

	struct State {
		char tiles[Ntiles]; // tiles[a] = b means tile b is at position a.
		char blank;
		char h;
	};

	struct PackedState {
		uint128_t word;
		uint64_t hash_key;
		char h;

		unsigned long hash() const {
			return hash_key;
		}

		bool eq(const PackedState &h) const {
			return word == h.word;
		}

		unsigned int byteSize() const {
			return 16;
		}

		void stateToChars(unsigned char* d) const {
			int n = sizeof word;
			for (int y = 0; n-- > 0; y++)
				d[y] = (word >> (n * 8)) & 0xff;
		}

		void charsToState(unsigned char* d) {
			int n = sizeof word;
			word = 0;
			for (int i = 0; i < n; ++i) {
				word += (uint128_t) d[i] << (8 * (n - i - 1));
			}
			hash_key = (uint64_t) word;
		}
	};

	// Tiles constructs a new instance by reading
	// the initial state from the given file which is
	// expected to be in Wheeler's tiles instance
	// format.
	Tiles24(FILE*);
	Tiles24(FILE*, int);

	State initial() {
		State s;
		s.blank = -1;
		for (int i = 0; i < Ntiles; i++) {
			if (init[i] == 0)
				s.blank = i;
			s.tiles[i] = init[i];
		}
		if (s.blank < 0)
			throw Fatal("No blank tile");
		s.h = heuristic(s.tiles);
		return s;
	}

	int h(const State &s) const {
		return s.h;
	}

	int weight_h(const State &s) const {
		assert(false);
		return s.h;
	}

	bool isgoal(const State &s) const {
		if (s.h != 0) {
			return false;
		}
		for (int i = 0; i < Ntiles; ++i) {
			if (s.tiles[i] != i) {
				return false;
			}
		}
		return true;
	}

	int nops(const State &s) const {
		return optab[(int) s.blank].n;
	}

	int nthop(const State &s, int n) const {
		return optab[(int) s.blank].ops[n];
	}

	struct Undo {
		int h, blank;
	};

	Edge<Tiles24> apply(State &s, int newb) const {
		Edge<Tiles24> e(1, newb, s.blank);
		e.undo.h = s.h;
		e.undo.blank = s.blank;

//		printf("before apply: ");
//		for (int i = 0; i < Ntiles; ++i) {
//			printf("%.2d ", s.tiles[i]);
//		}
//		printf("\n");

		int tile = s.tiles[newb];
		s.tiles[(int) s.blank] = tile;

		// TODO: Here we need to implement Pattern Databases Heuristic.
//		s.h += mdincr[tile][newb][(int) s.blank];

		s.tiles[newb] = 0;

		s.blank = newb;

//		printf("after  apply: ");
//		for (int i = 0; i < Ntiles; ++i) {
//			printf("%.2d ", s.tiles[i]);
//		}
//		printf("\n");

//		s.h = mdist(static_cast<int>(s.blank), s.tiles);
		s.h = heuristic(s.tiles);
		return e;
	}

	void undo(State &s, const Edge<Tiles24> &e) const {
//		printf("before undo : ");
//		for (int i = 0; i < Ntiles; ++i) {
//			printf("%.2d ", s.tiles[i]);
//		}
//		printf("\n");

		s.h = e.undo.h;
		s.tiles[(int) s.blank] = s.tiles[(int) e.undo.blank];
		s.tiles[e.undo.blank] = 0;
		s.blank = e.undo.blank;

//		printf("after  undo : ");
//		for (int i = 0; i < Ntiles; ++i) {
//			printf("%.2d ", s.tiles[i]);
//		}
//		printf("\n");

	}

	// pack packes state s into the packed state dst.
	// TODO: Not memory efficient but time efficient. May need to reconsider as its the memory that matters.
	void pack(PackedState &dst, State &s) const {
		dst.word = 0; // to make g++ shut up about uninitialized usage.
		s.tiles[(int) s.blank] = 0;
		for (int i = 0; i < Ntiles; i++)
			dst.word = (dst.word << 5) | s.tiles[i];

		dst.hash_key = 0;
		for (int i = 0; i < Ntiles; i++) {
			dst.hash_key = dst.hash_key ^ closed_hash_tbl[i][s.tiles[i]];
		}

		dst.h = s.h;
	}

	// unpack unpacks the packed state s into the state dst.
	void unpack(State &dst, PackedState s) const {
		dst.blank = -1;
		for (int i = Ntiles - 1; i >= 0; i--) {
			int t = s.word & 0x1F;
			s.word >>= 5;
			dst.tiles[i] = t;
			if (t == 0)
				dst.blank = i;
		}
		dst.h = heuristic(dst.tiles);
		assert(dst.blank >= 0);
	}

	unsigned int print_h(char tiles[]) const {
		// tiles[a] = b means tile b is at position a.
		// inv[a] = b means tile a is at position b.
		char inv[Ntiles];
		for (int i = 0; i < Ntiles; ++i) {
			inv[tiles[i]] = i;
//			inv[i] = tiles[i];
		}

		unsigned int origin = hash0(inv) + hash1(inv) + hash2(inv) + hash3(inv);
		unsigned int reflection = hashref0(inv) + hashref1(inv) + hashref2(inv)
				+ hashref3(inv);

//		printf("origin = %u + %u + %u + %u = %u\n", hash0(inv), hash1(inv), hash2(inv), hash3(inv), origin);
//		printf("reflection = %u + %u + %u + %u = %u\n", hashref0(inv), hashref1(inv), hashref2(inv), hashref3(inv), reflection);
//		printf("\n");
//		printf("origin, reflection = %u, %u\n", origin, reflection);
		return max(origin, reflection);
//		return origin;
	}

	void setHeuristic(unsigned int hfun) {
		hfunction = hfun;
	}

	void set_dist_hash(int h, int rand_seed = 0) {
		which_dist_hash = h;
		dist_h = new Zobrist<Tiles24, 25>(which_dist_hash, rand_seed);
	}

	unsigned int dist_hash(const State &s) const {
		return dist_h->dist_h(s);
	}


private:

	std::vector<std::vector<unsigned int> > closed_hash_tbl;

	unsigned int hfunction = 0;

	int which_dist_hash;
	DistributionHash<Tiles24>* dist_h;


//	// mdist returns the Manhattan distance of the given tile array.
//	int mdist(int blank, char tiles[]) const{
////		int sum = 0;
////		for (int i = 0; i < Ntiles; i++) {
////			if (i == blank)
////				continue;
////			int row = i / Width, col = i % Width;
////			int grow = tiles[i] / Width, gcol = tiles[i] % Width;
////			sum += abs(gcol - col) + abs(grow - row);
////		}
////		return sum;
//		return heuristic(tiles);
//	}

	// initmd initializes the md and mdincr tables.
//	void initmd();

	// initoptob initializes the operator table, optab.
	void initoptab();

	void readfile(unsigned char table[TABLESIZE], FILE* infile);

	// init is the initial tile positions.
	int init[Ntiles];



	unsigned int heuristic(char tiles[]) const {
		if (hfunction == 0) {
			return pdb(tiles);
		} else if (hfunction == 1) {
			return manhattan(tiles);
		} else {
			return 0;
		}
	}

	unsigned int pdb(char tiles[]) const {
		//		return 0;
		// tiles[a] = b means tile b is at position a.
		// inv[a] = b means tile a is at position b.
		char inv[Ntiles];

		// TODO: is this interpretation right?
		for (int i = 0; i < Ntiles; ++i) {
			inv[tiles[i]] = i;
			//			inv[i] = tiles[i];
		}

		unsigned int origin = hash0(inv) + hash1(inv) + hash2(inv) + hash3(inv);
		unsigned int reflection = hashref0(inv) + hashref1(inv) + hashref2(inv)
				+ hashref3(inv);
		//		unsigned int origin = hash0(inv); // + hash1(inv) + hash2(inv) + hash3(inv);
		//		unsigned int reflection = hashref0(inv);// + hashref1(inv) + hashref2(inv)
		//+ hashref3(inv);

		//		printf("tile: ");
		//		for (int i = 0; i < Ntiles; ++i) {
		//			printf("%.2d ", tiles[i]);
		//		}
		//		printf("\n");
		//		printf("inv : ");
		//		for (int i = 0; i < Ntiles; ++i) {
		//			printf("%d ", inv[i]);
		//		}
		//		printf("\n");
		//		printf("origin = %u + %u + %u + %u = %u\n", hash0(inv), hash1(inv), hash2(inv), hash3(inv), origin);
		//		printf("reflection = %u + %u + %u + %u = %u\n", hashref0(inv), hashref1(inv), hashref2(inv), hashref3(inv), reflection);
		//		printf("\n");
		//		printf("origin, reflection = %u, %u\n", origin, reflection);
		//		printf("hash0 = %u\n", origin);
		return max(origin, reflection);
		//		return origin;
	}

	unsigned int manhattan(char tiles[]) const {
		int sum = 0;
		for (int i = 0; i < Ntiles; i++) {
			if (tiles[i] == 0) {
				continue;
			}
			int row = i / Width, col = i % Width;
			int grow = tiles[i] / Width, gcol = tiles[i] % Width;
			sum += abs(gcol - col) + abs(grow - row);
		}
		return sum;
	}

	/* HASH0 takes an inverse state, and maps the tiles in the 0 pattern to an
	 integer that represents those tile positions uniquely.  It then returns the
	 actual heuristic value from the pattern database. */
	unsigned int hash0(char inv[]) const {
		int hashval = ((((inv[1] * Ntiles + inv[2]) * Ntiles + inv[5]) * Ntiles
				+ inv[6]) * Ntiles + inv[7]) * Ntiles + inv[12];
		return (h0[hashval]);
	} /* total moves for this pattern */
	unsigned int hashref0(char inv[]) const {
		/* index into heuristic table */
		int hashval = (((((ref[inv[5]] * Ntiles + ref[inv[10]]) * Ntiles
				+ ref[inv[1]]) * Ntiles + ref[inv[6]]) * Ntiles + ref[inv[11]])
				* Ntiles + ref[inv[12]]);
		return (h0[hashval]);
	} /* total moves for this pattern */

	unsigned int hash1(char inv[]) const {
		/* index into heuristic table */
		int hashval = ((((inv[3] * Ntiles + inv[4]) * Ntiles + inv[8]) * Ntiles
				+ inv[9]) * Ntiles + inv[13]) * Ntiles + inv[14];
		return (h1[hashval]);
	} /* total moves for this pattern */
	unsigned int hashref1(char inv[]) const {
		/* index into heuristic table */
		int hashval =
				(((((ref[inv[15]] * Ntiles + ref[inv[20]]) * Ntiles
						+ ref[inv[16]]) * Ntiles + ref[inv[21]]) * Ntiles
						+ ref[inv[17]]) * Ntiles + ref[inv[22]]);
		return (h1[hashval]);
	} /* total moves for this pattern */
	unsigned int hash2(char inv[]) const {
		/* index into heuristic table */
		int hashval = ((((rot180[inv[21]] * Ntiles + rot180[inv[20]]) * Ntiles
				+ rot180[inv[16]]) * Ntiles + rot180[inv[15]]) * Ntiles
				+ rot180[inv[11]]) * Ntiles + rot180[inv[10]];
		return (h1[hashval]);
	} /* total moves for this pattern */
	unsigned int hashref2(char inv[]) const {
		/* index into heuristic table */
		int hashval = (((((rot180ref[inv[9]] * Ntiles + rot180ref[inv[4]])
				* Ntiles + rot180ref[inv[8]]) * Ntiles + rot180ref[inv[3]])
				* Ntiles + rot180ref[inv[7]]) * Ntiles + rot180ref[inv[2]]);
		return (h1[hashval]);
	} /* total moves for this pattern */
	unsigned int hash3(char inv[]) const {
		/* index into heuristic table */
		int hashval = ((((rot90[inv[19]] * Ntiles + rot90[inv[24]]) * Ntiles
				+ rot90[inv[18]]) * Ntiles + rot90[inv[23]]) * Ntiles
				+ rot90[inv[17]]) * Ntiles + rot90[inv[22]];
		return (h1[hashval]);
	} /* total moves for this pattern */
	unsigned int hashref3(char inv[]) const {
		/* index into heuristic table */
		int hashval = (((((rot90ref[inv[23]] * Ntiles + rot90ref[inv[24]])
				* Ntiles + rot90ref[inv[18]]) * Ntiles + rot90ref[inv[19]])
				* Ntiles + rot90ref[inv[13]]) * Ntiles + rot90ref[inv[14]]);
		return (h1[hashval]);
	} /* total moves for this pattern */
	// Pattern Databases Heuristics
	static unsigned char h0[TABLESIZE]; /* heuristic tables for pattern databases */
	static unsigned char h1[TABLESIZE];
	/* the pattern that each tile is in */
	int whichpat[Ntiles] = { 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 2, 2, 0, 1, 1, 2, 2,
			3, 3, 3, 2, 2, 3, 3, 3 };

	/* the reflected pattern that each tile is in */
	int whichrefpat[Ntiles] = { 0, 0, 2, 2, 2, 0, 0, 2, 2, 2, 0, 0, 0, 3, 3, 1,
			1, 1, 3, 3, 1, 1, 1, 3, 3 };

	/* the position of each tile in order, reflected about the main diagonal */
	int ref[Ntiles] = { 0, 5, 10, 15, 20, 1, 6, 11, 16, 21, 2, 7, 12, 17, 22, 3,
			8, 13, 18, 23, 4, 9, 14, 19, 24 };

	/* rotates the puzzle 90 degrees */
	int rot90[Ntiles] = { 20, 15, 10, 5, 0, 21, 16, 11, 6, 1, 22, 17, 12, 7, 2,
			23, 18, 13, 8, 3, 24, 19, 14, 9, 4 };

	/* composes the reflection and 90 degree rotation into a single array */
	int rot90ref[Ntiles] = { 20, 21, 22, 23, 24, 15, 16, 17, 18, 19, 10, 11, 12,
			13, 14, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4 };

	/* rotates the puzzle 180 degrees */
	int rot180[Ntiles] = { 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12,
			11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0 };

	/* composes the reflection and 180 degree rotation into a single array */
	int rot180ref[Ntiles] = { 24, 19, 14, 9, 4, 23, 18, 13, 8, 3, 22, 17, 12, 7,
			2, 21, 16, 11, 6, 1, 20, 15, 10, 5, 0 };
	/* tiles in each regular pattern */
	/* {1,2,5,6,7,12} {3,4,8,9,13,14} {10,11,15,16,20,21} {17,18,19,22,23,24} */

	/* tiles in each reflected pattern, in the same order as above*/
	/* {5,10,1,6,11,12} {15,20,16,21,17,22} {2,7,3,8,4,9} {13,18,23,14,19,24} */

	// optab is indexed by the blank position.  Each
	// entry is a description of the possible next
	// blank positions.
	struct {
		int n, ops[4];
	} optab[Ntiles];

	unsigned int max(unsigned int a, unsigned int b) const {
		return a > b ? a : b;
	}
	unsigned int min(unsigned int a, unsigned int b) const {
		return a < b ? a : b;
	}
};

