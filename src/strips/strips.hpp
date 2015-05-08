#ifndef STRIPS_HPP_
#define STRIPS_HPP_

// Copyright 2012 Ethan Burns. All rights reserved.
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file.
#include "action_table.hpp"
#include "trie.hpp"

#include "../search.hpp"
#include "../fatal.hpp"
#include "../hashtbl.hpp"

#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <vector>
#include <iostream>
#include <stdint.h>

// Burns' implementation: block-14 5 sec in A*.
struct Strips {

	// TODO: would 16 bit enough
	struct State {
		std::vector<unsigned int> propositions; // need to be sorted
		int h;
	};

	struct PackedState {
		unsigned long word;
		std::vector<unsigned int> propositions;

		unsigned long hash() const {
			return word;
		}

		// however long you take for the hash, it does not cover all the sequences.
		bool eq(const PackedState &h) const {
//			return word == h.word;
			for (unsigned int i = 0; i < h.propositions.size(); ++i) {
				if (propositions[i] != h.propositions[i]) {
					return false;
				}
			}
			return true;
		}
	};

	// Tiles constructs a new instance by reading
	// the initial state from the given file which is
	// expected to be in Wheeler's tiles instance
	// format.
	Strips(std::istream &domain, std::istream &instance);
	~Strips() {}


	State initial() const {
		// TODO: put an initial state.
		State s;
		s.propositions = init_state;
		s.h = heuristic(s);
		return s;
	}

	int h(const State &s) const {
		return s.h;
	}

	bool isgoal(const State &s) const {
	// TODO: should be optimized. 2 sequences are sorted.
		std::vector<unsigned int> p = s.propositions;
		for (int i = 0; i < goal_condition.size(); ++i) {
			if (std::find(p.begin(), p.end(), goal_condition[i]) != p.end()) {
				continue;
			} else {
				return false;
			}
		}
		return true;
	}

	int nops(const State &s) const {
		// TODO: accesss prefixtree
		return 0;
	}

	// TODO: nops&nthop is duplication of work for STRIPS planning. should be optimized.
	int nthop(const State &s, int n) const {
		// TODO: access prefixtree
		return 0;
	}

	struct Undo {
		int action;
		int h;
	};

	Edge<Strips> apply(State &s, int action) const {
		int cost = 1;
		Edge<Strips> e(cost, action, -100);
		e.undo.action = action;
		e.undo.h = s.h;

		// TODO: apply action here

		s.h = heuristic(s);

		return e;
	}

	// invert the add&delete effect of the action
	void undo(State &s, const Edge<Strips> &e) const {
		s.h = e.undo.h;

		// TODO: undo action here
	}

	// pack: packes state s into the packed state dst.
	void pack(PackedState &dst, State &s) const {
		dst.word = 0; // to make g++ shut up about uninitialized usage.
		for (int i = 0; i < s.propositions.size(); i++) {
			dst.word = (dst.word << 1) | s.propositions[i];
		}
		dst.propositions = s.propositions; // copy
	}

	// unpack: unpacks the packed state s into the state dst.
	void unpack(State &dst, PackedState s) const {
		dst.propositions = s.propositions;
		dst.h = heuristic(dst);
	}


	void set_heuristic(int h) {
		which_heuristic = h;
	}

private:
	// proposition library (prefixtree)
	// action library (table of number to action)

	int which_heuristic; // 0:blind, 1:goal_count, 2:hmax
	Trie actionTrie; // dictionary which returns feasible actions for given state.
	ActionTable actionTable; // table of action_key to action description.
	std::vector<unsigned int> init_state;
	std::vector<unsigned int> goal_condition;

	void apply_action(State& s, int action_key) {

	}

	void undo_action(State& s, int action_key) {

	}

	/////////////////////////////
	// heuristics
	/////////////////////////////
	int heuristic(const State& s) const {
		switch (which_heuristic) {
		case 0: // blind
			return blind(s);
			break;
		case 1: // goal_count
			return goal_count(s);
			break;
		case 2: // hmax
			return hmax(s);
			break;
		default:
			assert(false);
			break;
		}
		return 0;
	}

	int blind(const State& s) const {
		return 0;
	}

	int goal_count(const State& s) const {
		// TODO: should be optimized. 2 arrays are sorted.
		int goals = 0;
		std::vector<unsigned int> p = s.propositions;
		for (int i = 0; i < goal_condition.size(); ++i) {
			if (std::find(p.begin(), p.end(), goal_condition[i]) != p.end()) {
				++goals;
			}
		}
		return goals;
	}

	int hmax(const State& s) const {
		return 0;
	}


	/////////////////////////////
	// parser
	/////////////////////////////

	// Predicates will be grounded after this initialization.
public:
	struct Predicate {
		unsigned int key;
		std::string symbol;
		unsigned int number_of_arguments;
		Predicate(){};
		Predicate(unsigned int key, std::string symbol, unsigned int number_of_arguments)
		: key(key), symbol(symbol), number_of_arguments(number_of_arguments){}

	};

	struct Object {
		unsigned int key;
		std::string symbol;
	};

	struct GroundedPredicate {
		unsigned int key;
		std::string symbol;
	};

private:
	void readPredicates(std::istream &domain, std::vector<Predicate>& predicates);
	void readObjects(std::istream &instance, std::vector<Object>& objects);
	void readInit(std::istream &instance);
	void readGoal(std::istream &instance);
	void readAction(std::istream &domain);

};
#endif
