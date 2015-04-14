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
#include <iostream>

struct Grid {
	enum {
		Width = 4,
		Height = 4,
		Ntiles = Width*Height,
	};

	struct State {
		char tiles[4]; // not optimal
		unsigned int blank;
		unsigned int x;
		unsigned int y;
		unsigned int h;
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
	Grid(std::istream &in, unsigned int scale);
//	Grid(FILE*, int);

	State initial() const {
		State s;
		s.x = initx;
		s.y = inity;
		s.h = mdist(s, goal);
		s.blank = s.x * 100000 + s.y;


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

	// TODO
	int nops(const State &s) const {
//		unsigned int x = s.x / scale;
//		unsigned int y = s.y / scale;

		unsigned int ops = 0;
		if ((s.x < total_width - 1) && !obstacles[(s.y/scale) * width + (s.x + 1)/scale]) {
			++ops;
		}
		if ((s.y < total_height - 1) && !obstacles[(s.y + 1) / scale * width + s.x / scale]) {
			++ops;
		}
		if ((s.x > 0) && !obstacles[s.y / scale * width + (s.x - 1) / scale]) {
			++ops;
		}
		if ((s.y > 0) && !obstacles[(s.y - 1)/scale * width + s.x / scale]) {
			++ops;
		}
		return ops;
	}

	// This function returns the possible action for the state.
	int nthop(const State &s, int n) const {
//		unsigned int x = s.x / scale;
//		unsigned int y = s.y / scale;

		unsigned int ops = -1;

		if ((s.x < total_width - 1) && !obstacles[(s.y/scale) * width + (s.x + 1)/scale]) {
			++ops;
		}
		if (n == ops) {
			return 0;
		}
		if ((s.y < total_height - 1) && !obstacles[(s.y + 1) / scale * width + s.x / scale]) {
			++ops;
		}
		if (n == ops) {
			return 1;
		}
		if ((s.x > 0) && !obstacles[s.y / scale * width + (s.x - 1) / scale]) {
			++ops;
		}
		if (n == ops) {
			return 2;
		}
		if ((s.y > 0) && !obstacles[(s.y - 1)/scale * width + s.x / scale]) {
			++ops;
		}
		if (n == ops) {
			return 3;
		}
		printf("ERROR");
		return NULL;

	}

	// Operation and the value of h.
	struct Undo {
		int h, op;
	};

	// TODO
	Edge<Grid> apply(State &s, int newb) const {
		Edge<Grid> e(1, newb, (newb + 2) % 4);
		e.undo.h = s.h;
		e.undo.op = (newb + 2) % 4;

		if (newb == 0) {
			++s.x;
		} else if (newb == 1) {
			++s.y;
		} else if (newb == 2) {
			--s.x;
		} else {
			--s.y;
		}
		s.h = mdist(s, goal);
		s.blank = s.x * 100000 + s.y;


		return e;
	}

	// TODO
	void undo(State &s, const Edge<Grid> &e) const {
		s.h = e.undo.h;

		int undo = e.undo.op;
		if (undo == 0) {
			++s.x;
		} else if (undo == 1) {
			++s.y;
		} else if (undo == 2) {
			--s.x;
		} else {
			--s.y;
		}
		s.blank = s.x * 100000 + s.y;


	}

	// TODO
	void pack(PackedState &dst, State &s) const {
		dst.word = s.x * total_width + s.y;
	}

	// TODO
	// unpack unpacks the packed state s into the state dst.
	void unpack(State &dst, PackedState s) const {
		dst.x = s.word / total_width;
		dst.y = s.word % total_width;
		dst.h = mdist(dst, goal);
		dst.blank = dst.x * 100000 + dst.y;

	}

	void set_weight(double weight_) {
		this->weight = weight_;
	}

	unsigned int get_width() {
		return width;
	}

	unsigned int get_height() {
		return height;
	}

private:

	// mdist returns the Manhattan distance of the given tile array.
	int mdist(const State from, const State to) const {
		return abs(from.x - to.x) + abs(from.y - to.y);
	}

	unsigned int initx;
	unsigned int inity;

	unsigned int goalx;
	unsigned int goaly;

	State goal;

	unsigned int width;
	unsigned int height;

	unsigned int scale;

	unsigned int total_width;
	unsigned int total_height;


	std::vector<bool> obstacles; // = new bool[width * height]

	double weight;

	// optab is indexed by the blank position.  Each
	// entry is a description of the possible next
	// blank positions.

	// Operations: Move
	// left  = 0
	// up    = 1
	// right = 2
	// down  = 3

	//	struct { int n, ops[4]; } optab[Ntiles];
};
