// Copyright 2012 Ethan Burns. All rights reserved.
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file.
#ifndef TSP_
#define TSP_


#include "search.hpp"
#include "fatal.hpp"
#include "hashtbl.hpp"
#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <vector>
#include <stdint.h>
#include <math.h>
#include <iostream>

struct Tsp {
	enum {
		Width = 4,
		Height = 4,
		Ntiles = Width*Height,
	};
	struct State {
		char tiles[15];
		char blank;
		std::vector<bool> visited; // include current city
		unsigned int current; // current city
		unsigned int h;
	};


	// TODO: is this supposed to be perfect hashing?
	struct PackedState {
		std::vector<bool> visited; // include current city
		unsigned int current; // current city
		uint64_t word;

		unsigned long hash() const {
			return word;
		}

		// TODO: is this for perfectly equal? or just to check if the hash value is equal?
		bool eq(const PackedState &h) const {
//			printf("PacekdState::eq\n");
			return word == h.word;
		}
	};

	// Tiles constructs a new instance by reading
	// the initial state from the given file which is
	// expected to be in Wheeler's tiles instance
	// format.
	Tsp(std::istream &in);

	State initial() const {
		State s;
		s.visited.resize(number_of_cities);
		for (unsigned int i = 0; i < number_of_cities; ++i) {
			s.visited[i] = false;
		}
		s.current = 0;
		s.h = calc_heuristic(s); // TODO: heuristic();

		return s;
	}

	unsigned int h(const State &s) const {
		return s.h; // TODO: h should be double, but no need to be that precise.
	}

	bool isgoal(const State &s) const {
		for (unsigned int i = 0; i < number_of_cities; ++i) {
			if (!s.visited[i]) {
				return false;
			}
		}
		return true;
	}

	// TODO: for all unvisited cities
	// the number of false in s.visited
	int nops(const State &s) const {
		unsigned int n_not_visited = 0;
		for (unsigned int i = 1; i < number_of_cities; ++i) {
			if (!s.visited[i]) {
				++n_not_visited;
			}
		}

		if (n_not_visited) {
			// if there are cities not visited other than the home town.
			return n_not_visited;
		} else {
//			printf("to goal nops\n");
			// if all cities other than the home town visited
			return 1;
		}
	}

	// TODO: for all unvisited cities
	// the nth number of false in s.visited
	// n start from 0.
	int nthop(const State &s, int n) const {
		unsigned int n_not_visited = 0;

		bool to_goal = true;
		for (unsigned int i = 1; i < number_of_cities; ++i) {
			if (!s.visited[i]) {
				to_goal = false;
				break;
			}
		}
		if (to_goal) {
//			printf("to goal\n");
			if (n != 0) {
				printf("ERROR nthop\n");
			}
			return 0;
		}

		for (unsigned int i = 1; i < number_of_cities; ++i) {
			if (!s.visited[i]) {
				if (n_not_visited == n) {
					return i;
				}
				++n_not_visited;
			}
		}
		printf("ERROR_nthop\n");
		return 0;
	}


	struct Undo { unsigned int h, whereitwas; };

	// TODO: flip visited & current location. also apply heuristic.
	Edge<Tsp> apply(State &s, int newb) const {

		// newb is the number of new city
		int cost = distances[s.current * number_of_cities + newb];

		// arguments are: cost, operation, operation to back
		// Operation to back is not used for tsp.
		Edge<Tsp> e(cost, newb, -1);
		e.undo.h = s.h;
		e.undo.whereitwas = s.current;

		s.visited[newb] = true;
		s.current = newb;
		s.h = calc_heuristic(s); //heuristic();

		if (newb == 0) {
//			printf("TO GOAL !\n");
		}

//		printf("%u :", s.current);
//		for (unsigned int i = 0; i < number_of_cities; ++i) {
//			if (s.visited[i]) {
//				printf("O");
//			} else {
//				printf("X");
//			}
//		}
//		printf("\n");

		return e;
	}

	// TODO: just flip visited & back to whereitwas. also back off h.
	void undo(State &s, const Edge<Tsp> &e) const {
		s.visited[s.current] = false;
		s.current = e.undo.whereitwas;
		s.h = e.undo.h;
	}

	// pack packes state s into the packed state dst.
	// TODO: so, packed state need to be perfect hash somehow.
	void pack(PackedState &dst, State &s) const {
		dst.visited = s.visited;
		dst.current = s.current;
		dst.word = 0;	// to make g++ shut up about uninitialized usage.
		for (unsigned int i = 0; i < number_of_cities; ++i) {
			if (s.visited[i]) {
				dst.word += (1 << i);
			}
		}
	}

	// unpack unpacks the packed state s into the state dst.
	// TODO: so, packed state need to be perfect hash somehow.
	void unpack(State &dst, PackedState s) const {
//		printf("unpack\n");
		dst.visited = s.visited;
		dst.current = s.current;
		dst.h = calc_heuristic(dst);

	}

	void set_heuristic(unsigned int h) {
		switch (h) {
		case 0:
			printf("blind\n");
			break;
		case 1:
			printf("roundtrip\n");
			break;
		case 2:
			printf("mst\n");
			break;
		case 3:
			printf("onetree\n");
			break;
		default:
			printf("set_heuristic ERROR\n");
			break;
		}
		this->heuristic = h;
	}

	unsigned int get_number_of_cities(void) const {
		return number_of_cities;
	}

	const std::vector<unsigned int> get_distances(void) const {
		return distances;
	}

private:

	unsigned int calc_heuristic(const State &s) const {
		switch (this->heuristic) {
		case 0:
			return blind_h(s);
		case 1:
			return roundtrip_h(s);
		case 2:
			return mst_h(s);
		case 3:
			return onetree_h(s);
		default:
			return blind_h(s);
		}
	}
	unsigned int blind_h(const State &s) const {
		return 1000000;
	}

	unsigned int roundtrip_h(const State &s) const {
		unsigned int max_dist = 0;
		for (unsigned int i = 0; i < number_of_cities; ++i) {
			if (!s.visited[i] && distances[s.current * number_of_cities + i] > max_dist) {
				max_dist = distances[s.current * number_of_cities + i];
			}
		}
		return max_dist;
	}

	unsigned int mst_h(const State &s) const {
		std::vector<bool> visited(s.visited);
		visited[s.current] = false;
		return mst(visited);
	}

	unsigned int onetree_h(const State &s) const {
		std::vector<bool> visited(s.visited);
		visited[0] = true;

		// from current to not visited
		unsigned int min_from = 1000000;
		for (unsigned int i = 1; i < number_of_cities; ++i) {
			if (!visited[i] && distances[s.current * number_of_cities + i] < min_from) {
				min_from = distances[s.current * number_of_cities + i];
			}
		}
		if (min_from == 1000000) {
			min_from = 0;
		}
		// from not visited to goal
		unsigned int min_to = 1000000;
		for (unsigned int i = 1; i < number_of_cities; ++i) {
			if (!visited[i] && distances[s.current * number_of_cities + i] < min_to) {
				min_to = distances[i * number_of_cities + 0];
			}
		}
		if (min_to == 1000000) {
			min_to = 0;
		}

		unsigned int smst = mst(visited);
//		visited[0] = false;
//		visited[s.current] = false;
//		unsigned int fmst = mst(visited);
		if (smst == 0) {
//			printf("from, to, mst = %u, %u, %u\n", min_from, min_to, smst);
		}

		return min_from + min_to + smst;
	}

	unsigned int mst(const std::vector<bool> visited) const {


		std::vector<unsigned int> vertices; // vertices are the nodes for the MSP.
		std::vector<unsigned int> t; // building
		std::vector<unsigned int> g_minus_t; // The rest of the graph

	//	printf("not_visited = ");
		unsigned int cost = 0;

		for (unsigned int i = 0; i < visited.size(); ++i) {
			if (visited[i] == false) {
	//			printf("O");
				vertices.push_back(i);
				g_minus_t.push_back(i);
			} else {
	//			printf("X");
			}
		}
	//	printf("\n");

		if (vertices.size() <= 1) {
			if (vertices.empty()) {
//				printf("empty mst\n");
			} else {
	//			printf("single vertex\n");
			}
			return 0;
		}
		t.push_back(vertices.back()); // initial node
		g_minus_t.pop_back(); // delete from g_minus_t.
	//	printf("t.size = %lu\n", t.size());
	//	printf("vertices.size = %lu\n", vertices.size());
		while (t.size() < vertices.size()) {
			double min_edge = 100000.0; // max edge of this domain is 1.412 (or sqrt(2))
			unsigned int new_vertex = -1;

			// Get the minimum edge to a new vertex.
			for (unsigned int i = 0; i < t.size(); ++i) {
				for (unsigned int j = 0; j < g_minus_t.size(); ++j) {
					if (distances[t[i] * number_of_cities + g_minus_t[j]] < min_edge) {
						min_edge = distances[t[i] * number_of_cities + g_minus_t[j]];
	//					new_vertex = g_minus_t[j];
						new_vertex = j;
					}
				}
			}
	//		printf("new vertex = %u\n", g_minus_t[new_vertex]);
			t.push_back(g_minus_t[new_vertex]);
			g_minus_t.erase(g_minus_t.begin() + new_vertex); // This won't be a issue as the size of vector is <100.
			cost += min_edge;
		}
//		printf("cost = %u\n", cost);
		return cost;
	}

	double md(double fromx, double fromy, double tox, double toy) {
		return fabs(fromx - tox) + fabs(fromy - toy);
	}


	std::vector<unsigned int> distances;
	unsigned int number_of_cities;

	unsigned int heuristic; // heuristics

	// optab is indexed by the blank position.  Each
	// entry is a description of the possible next
	// blank positions.
};

#endif /* TSP_ */

