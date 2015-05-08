// Copyright 2012 Ethan Burns. All rights reserved.
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file.
#include "search.hpp"
#include "fatal.hpp"
#include "hashtbl.hpp"

#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <stdint.h>




struct Tiles {
	enum {
		Width = 4,
		Height = 4,
		Ntiles = Width*Height,
	};

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
		s.h = mdist(s.blank, s.tiles);
		return s;
	}

	int h(const State &s) const {
		return s.h;
	}

	int weight_h(const State &s) const {
		double h = static_cast<double>(s.h);
		double wh = h * weight;
		return static_cast<int>(wh);
	}

	bool isgoal(const State &s) const {
		return s.h == 0;
	}

	int nops(const State &s) const {
		return optab[(int) s.blank].n;
	}

	int nthop(const State &s, int n) const {
		return optab[(int) s.blank].ops[n];
	}

	struct Undo { int h, blank; };

	Edge<Tiles> apply(State &s, int newb) const {
		Edge<Tiles> e(1, newb, s.blank);
		e.undo.h = s.h;
		e.undo.blank = s.blank;

		int tile = s.tiles[newb];
		s.tiles[(int) s.blank] = tile;
		s.h += mdincr[tile][newb][(int) s.blank];
		s.blank = newb;

		return e;
	}

	void undo(State &s, const Edge<Tiles> &e) const {
		s.h = e.undo.h;
		s.tiles[(int) s.blank] = s.tiles[(int) e.undo.blank];
		s.blank = e.undo.blank;
	}

	// pack packes state s into the packed state dst.
	void pack(PackedState &dst, State &s) const {
		dst.word = 0;	// to make g++ shut up about uninitialized usage.
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
		assert (dst.blank >= 0);
	}

	void set_weight(double weight_) {
		this->weight = weight_;
	}

	// TODO: garbage method for debugging 24 tiles.
	unsigned int print_h(char tiles[]) const {}

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

	double weight;

	// optab is indexed by the blank position.  Each
	// entry is a description of the possible next
	// blank positions.
	struct { int n, ops[4]; } optab[Ntiles];
};
