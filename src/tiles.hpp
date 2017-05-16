// Copyright 2012 Ethan Burns. All rights reserved.
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file.
#include "search.hpp"
#include "fatal.hpp"
#include "dist_hash.hpp"
#include "zobrist.hpp"

#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <stdint.h>

struct Tiles {
	enum {
		Width = 4, Height = 4, Ntiles = Width * Height,
	};

	// tiles represent the id of the tile positions
	// 0: blank tile
	struct State {
		char tiles[Ntiles];
		char blank;
		char h;
	};

	struct PackedState {
		uint64_t word;

		unsigned long hash() const {
			return word;
		}

		bool eq(const PackedState &h) const {
			return word == h.word;
		}

		unsigned int byteSize() const {
			return 8;
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
				word += (uint64_t) d[i] << (8 * (n - i - 1));
			}
		}
	};

	// Tiles constructs a new instance by reading
	// the initial state from the given file which is
	// expected to be in Wheeler's tiles instance
	// format.
	Tiles(FILE*);
	Tiles(FILE*, int);

	State initial() const {
		State s;
		s.blank = -1;
		for (int i = 0; i < Ntiles; i++) {
			if (init[i] == 0)
				s.blank = i;
			s.tiles[i] = init[i];
		}
		if (s.blank < 0)
			throw Fatal("No blank tile");

		switch(which_heuristic) {
		case 0:
			s.h = mdist(s.blank, s.tiles);
			break;
		case 1:
			s.h = displacement(s, -1, 0);
			break;
		case 2:
			s.h = 0;
			break;
		}

		return s;
	}

	State goal() const {
		State s;
		s.blank = 0;
		for (int i = 0; i < Ntiles; i++) {
			s.tiles[i] = i;
		}
		if (s.blank < 0)
			throw Fatal("No blank tile");


		switch(which_heuristic) {
		case 0:
			s.h = mdistback(s.blank, s.tiles);
			break;
		case 1:
			s.h = displacement(s, -1, 1);
			break;
		case 2:
			s.h = 0;
			break;
		}
//		printf("goal h val = %d\n", s.h);
		return s;
	}

	int h(const State &s) const {
//		return 0;
		return s.h;
	}

	int weight_h(const State &s) const {
		double h = static_cast<double>(s.h);
		double wh = h * weight;
		return static_cast<int>(wh);
	}

	bool isgoal(const State &s) const {
		switch (which_heuristic) {
		case 0:
			return s.h == 0;
			break;
		case 1:
			return s.h == 0;
			break;
		case 2:
			return displacement(s, -1, 0) == 0;
		}
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

	Edge<Tiles> apply(State &s, int newb) const {
		Edge<Tiles> e(1, newb, s.blank);
		e.undo.h = s.h;
		e.undo.blank = s.blank;

		int tile = s.tiles[newb];
		s.tiles[(int) s.blank] = tile;
		s.h = heuristic(s, newb, 0);
//		s.h += mdincr[tile][newb][(int) s.blank];
		s.blank = newb;

		return e;
	}

	Edge<Tiles> applybidr(State &s, int newb, bool isbackward) const {
		if (isbackward) {
//			printf("applybidr back\n");
			Edge<Tiles> e(1, newb, s.blank);

			e.undo.h = s.h;
			e.undo.blank = s.blank;
			int tile = s.tiles[newb];
			s.tiles[(int) s.blank] = tile;

//			printf("oldh = %d, action = %d, newh = %d\n", s.h,
//					bmdincr[tile][newb][(int) s.blank],
//					s.h + bmdincr[tile][newb][(int) s.blank]);
//			s.h += bmdincr[tile][newb][(int) s.blank];
			s.h = heuristic(s, newb, isbackward);

			s.blank = newb;
			return e;
		} else {
//			printf("applybidr forward\n");
			return apply(s, newb);
		}
	}

	int heuristic(const State &s, int newb, bool isbackward) const {
		switch (which_heuristic) {
		{
			case 0:
			// Manhattan distance
			int tile = s.tiles[newb];
			if (isbackward) {
				return s.h + bmdincr[tile][newb][(int) s.blank];
			} else {
				return s.h + mdincr[tile][newb][(int) s.blank];
			}
			break;
		}
		{
			case 1:
			// Displacement
			return displacement(s, newb, isbackward);
			break;
		}
		{
			case 2:
			// Blind
			return 0;
			break;
		}
		}
	}

	int displacement(const State &s, int newb, bool isbackward) const {
		if (isbackward) {
			int h = 0;
			for (int i = 0; i < Ntiles; ++i) {
				if (s.tiles[i] != 0 && s.tiles[i] != init[i]) {
					++h;
				}
			}
			return h;

		} else {
			int h = 0;
			for (int i = 1; i < Ntiles; ++i) {
				if (s.tiles[i] != i) {
					++h;
				}
			}
			return h;
		}
	}

	void undo(State &s, const Edge<Tiles> &e) const {
		s.h = e.undo.h;
		s.tiles[(int) s.blank] = s.tiles[(int) e.undo.blank];
		s.blank = e.undo.blank;
	}

// pack packes state s into the packed state dst.
	void pack(PackedState &dst, State &s) const {
		dst.word = 0; // to make g++ shut up about uninitialized usage.
		s.tiles[(int) s.blank] = 0;
		for (int i = 0; i < Ntiles; i++)
			dst.word = (dst.word << 4) | s.tiles[i]; // TODO: 15 puzzle specific
	}

// unpack unpacks the packed state s into the state dst.
	void unpack(State &dst, PackedState s) const {
		dst.h = 0;
		dst.blank = -1;
		for (int i = Ntiles - 1; i >= 0; i--) {
			int t = s.word & 0xF; // 15 Puzzle specific
			s.word >>= 4;
			dst.tiles[i] = t;
			if (t == 0)
				dst.blank = i;
			else
				dst.h += md[t][i];
		}
		assert(dst.blank >= 0);
	}

// unpack unpacks the packed state s into the state dst.
	void unpackbidr(State &dst, PackedState s, bool isbackward) const {
		if (isbackward) {
			dst.h = 0;
			dst.blank = -1;
			for (int i = Ntiles - 1; i >= 0; i--) {
				int t = s.word & 0xF; // 15 Puzzle specific
				s.word >>= 4;
				dst.tiles[i] = t;
				if (t == 0)
					dst.blank = i;
				else
					dst.h += bmd[t][i];
			}
			assert(dst.blank >= 0);
		} else {
			unpack(dst, s);
		}
	}

	void set_heuristic(int heursitic) {
		which_heuristic = heursitic;
	}

	void set_weight(double weight_) {
		this->weight = weight_;
	}

// TODO: garbage method for debugging 24 tiles.
	unsigned int print_h(char tiles[]) const {
	}

	unsigned int print_state(const State &s) const {
		printf("h=%d: ", s.h);
		for (int i = 0; i < Ntiles; ++i) {
			printf("%d ", s.tiles[i]);
		}
		printf("\n");
	}

	void set_dist_hash(int h, int rand_seed = 0) {
		which_dist_hash = h;
		dist_h = new Zobrist<Tiles, 16>(which_dist_hash, rand_seed);
	}

	unsigned int dist_hash(const State &s) const {
		return dist_h->dist_h(s);
	}

private:

// mdist returns the Manhattan distance of the given tile array.
	int mdist(int blank, char tiles[]) const {
		int sum = 0;
		for (int i = 0; i < Ntiles; i++) {
			if (i == blank)
				continue;
			int row = i / Width, col = i % Width;
			int grow = tiles[i] / Width, gcol = tiles[i] % Width;
			sum += abs(gcol - col) + abs(grow - row);
		}
		return sum;
	}

// Given a state tiles[], return a mdist to the init state.
	int mdistback(int blank, char tiles[]) const {
		int inv[Ntiles];
		for (int i = 0; i < Ntiles; i++) {
			inv[init[i]] = i;
		}

		int sum = 0;
		for (int i = 1; i < Ntiles; i++) {
			if (i == blank)
				continue;
			int row = tiles[i] / Width, col = tiles[i] % Width;
			int grow = inv[i] / Width, gcol = inv[i] % Width;
			sum += abs(gcol - col) + abs(grow - row);
		}
		return sum;
	}

	int which_heuristic;

// initmd initializes the md and mdincr tables.
	void initmd();

// initoptob initializes the operator table, optab.
	void initoptab();

// init is the initial tile positions.
	int init[Ntiles];

// md is indexed by tile and location.  Each entry
	int md[Ntiles][Ntiles];

// mdincr is indexed by tile, source location, and
// destination location.  Each entry is the change
// in Manhattan distance for that particular move.
	int mdincr[Ntiles][Ntiles][Ntiles];

// MD for backward search.
	int bmd[Ntiles][Ntiles];
	int bmdincr[Ntiles][Ntiles][Ntiles];

	double weight;

	int which_dist_hash;
	DistributionHash<Tiles>* dist_h;

// optab is indexed by the blank position.  Each
// entry is a description of the possible next
// blank positions.
	struct {
		int n, ops[4];
	} optab[Ntiles];

};
