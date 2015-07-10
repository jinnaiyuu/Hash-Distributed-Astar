#ifndef GRAPH_HPP_
#define GRAPH_HPP_
// Copyright 2012 Ethan Burns. All rights reserved.
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file.
#include "../search.hpp"
#include "../fatal.hpp"
#include "../hashtbl.hpp"

#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <stdint.h>
#include <vector>
#include <utility>

struct Graph {

	struct State {
		int id;
		char h; // do we need this?
	};

	struct PackedState {
		int word; // = state.id

		unsigned long hash() const {
			return word;
		}

		bool eq(const PackedState &h) const {
			return word == h.word;
		}
	};

	Graph(const std::vector<std::vector<unsigned int>>& dic) :
			action_cost(false) {
		edge_dictionary = dic;
	}

//	Graph(const std::vector<std::vector<unsigned int>>& dic,
//			const std::vector<std::vector<unsigned int>>& cost) :
//			action_cost(true) {
//		edge_dictionary = dic;
//		edgecost_dictionary = cost;
//	}

	State initial() const {
		State s;
		s.id = init_id;
		s.h = 0;
		return s;
	}

	int h(const State &s) const {
		return s.h;
	}

	// TODO: this is useless for dijkstra regression search.
	bool isgoal(const State &s) const {
		return false;
	}

//	int nops(const State &s) const {
//		return edge_dictionary[s.id].size();
//	}
//
//	int nthop(const State &s, int n) const {
//		return edge_dictionary[s.id][n];
//	}

	std::vector<unsigned int> nops(const State &s) const {
		return edge_dictionary[s.id];
	}

	struct Undo {
		int id;
	};

	Edge<Graph> apply(State &s, int newb) const {
		int cost = 1;

		// TODO: this is not so simple as newb does not  contain information of the edge.
		if (action_cost) {
//			cost = edgecost_dictionary[];
		}
		Edge<Graph> e(cost, newb, s.id);
		e.undo.id = s.id;
		s.id = newb;
		return e;
	}

	void undo(State &s, const Edge<Graph> &e) const {
		s.id = e.undo.id;
	}

	// pack packes state s into the packed state dst.
	void pack(PackedState &dst, State &s) const {
		dst.word = s.id;
	}

	// unpack unpacks the packed state s into the state dst.
	void unpack(State &dst, PackedState s) const {
		dst.id = s.word;
	}

	void set_init(int id) {
		init_id = id;
	}

	// TODO: garbage method for debugging 24 tiles.
	unsigned int print_h(char tiles[]) const {
		return 0;
	}

private:

	std::vector<std::vector<unsigned int>> edge_dictionary;
	std::vector<std::vector<unsigned int>> edgecost_dictionary;

	int init_id;
	bool action_cost;

};

#endif
