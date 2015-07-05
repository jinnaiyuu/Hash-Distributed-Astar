#ifndef DIJKSTRA2_HPP_
#define DIJKSTRA2_HPP_

#include <stdio.h>

#include "strips.hpp"
#include "utils.hpp"
#include "../search.hpp"
#include "../utils.hpp"
#include "../hashtbl.hpp"
#include "../heap.hpp"
#include "../pool.hpp"

// This algorithm is only for STRIPS planning.
// TODOs: FROM A* TO DIJKSTRA(or BFS)
// TODO: narrow down the action. if it does not affect pattern then discard.
// TODO: this implementation is far from optimal. it's more like a prototype.
// XXX:  seems this implementation is buggy as it does not obtain all patterns available.
template<class D> class Dijkstra2: public SearchAlg<D> {

	const unsigned int SEPARATOR = -1;
	const unsigned int TRUE_PREDICATE = 10000000;
	const unsigned int NO_HEURISTIC = -3;

	struct Node {
		unsigned int f, g;
		char pop;
		int openind;
		Node *parent;
		typename D::PackedState packed;
		HashEntry<Node> hentry;

		bool pred(Node *o) {
			if (f == o->f)
				return g > o->g;
			return f < o->f;
		}

		void setindex(int i) {
		}

		const typename D::PackedState &key() {
			return packed;
		}

		HashEntry<Node> &hashentry() {
			return hentry;
		}
	};

	HashTable<typename D::PackedState, Node> closed;
	Heap<Node> open; // TODO: TODO
	std::vector<typename D::State> path;
	Pool<Node> nodes;

	unsigned int incumbent;

	std::vector<std::vector<unsigned int>> patterns;
	std::vector<std::vector<unsigned int>> groups;
	std::vector<unsigned int> costs; // costs are set to be infinitie at first.
	unsigned int costs_it;
	unsigned int updated = 0;

public:

	Dijkstra2(D &d, std::vector<std::vector<unsigned int>> patterns,
			std::vector<std::vector<unsigned int>> groups) :
				SearchAlg<D>(d), closed(5129273), open(120), incumbent(1000000), patterns(
					patterns), groups(groups), costs(patterns.size(),
					NO_HEURISTIC), costs_it(0), updated(0), expd(0), gend(0) {
	}

	// TODO: ungroupeds are true for every day.
	// TODO: possibly need to define new domain for pdb.(PDBD?)
	std::vector<typename D::State> search(typename D::State &init) {
		open.push(wrap(init, 0, 0, -1));

		// t is to detect stuck of the search.
		// if update did not change for 2 minutes then the search is stuck.
		double time = 30;
		double t = walltime();

//		printf("start dijkstra\n");
		while (!open.isempty() && !terminate()) {
//			printf("l\n");

//			printf("while loop\n");
			if (walltime() - t > time) {
				std::cout << walltime() - t << " sec " << std::endl;
				break;
			}

			Node *n = static_cast<Node*>(open.pop());
			if (closed.find(n->packed)) {
				nodes.destruct(n);
//				printf("searched\n");
				continue;
			}
			typename D::State state;
			this->dom.unpack(state, n->packed);

//			printf("expd = %lu\n", this->expd);
//			Grid::State s = static_cast<Grid::State>(state);
//			printf("x = %u\n", state.blank);
//			printf("f,g = %d, %d\n", n->f, n->g);

//			if (this->dom.isgoal(state)) {
////				printf("goal!\n");
////				printf("f = %u\n", n->f);
//				for (Node *p = n; p; p = p->parent) {
//					typename D::State s;
//					this->dom.unpack(s, p->packed);
//					path.push_back(s);
//				}
//				break;
//			}

			closed.add(n);

			// TODO: find the pattern of current node.
			unsigned int p = match_pattern(state);
			if (costs[p] > n->g) {
				costs[p] = n->g;
				++updated;
				if (updated % (costs.size() / 1000 + 200) == 0) {
					std::cout << "updated " << updated << std::endl;
				}
//				std::cout << "cost for " << p << " = " << n->g << std::endl;
				t = walltime(); // update t as it found new update.
			}

			this->expd++;
			std::vector<unsigned int> ops = this->dom.nops(state);
//			printf("%d ops\n", ops.size());

			for (int i = 0; i < ops.size(); i++) {
				int op = ops[i];
//				printf("op = %u\n", op);
				if (op == n->pop)
					continue;
				Edge<D> e = this->dom.apply(state, op);

				Node* next = wrap(state, n, e.cost, e.pop);
				if (next->f > incumbent) {
//					delete next;
					printf("f > incumbent\n");
					this->dom.undo(state, e);
					continue;
				}
				this->gend++;
				open.push(next);
				this->dom.undo(state, e);
			}
//			printf("expd done\n\n");
//			printf("\n");
		}

//		// if the search has ended due to
		if (walltime() - t > time || !terminate()) {
			std::cout << "pdb not complete." << std::endl;
		}

//		printf("return astar\n");
		this->wtime = walltime();
		this->ctime = cputime();

		return path;
	}

	Node *wrap(typename D::State &s, Node *p, int c, int pop) {
		Node *n = nodes.construct();
		n->g = c;
		if (p)
			n->g += p->g;
		n->f = n->g; // f is set to zero.
//		unsigned int nw = n->g + this->dom.h(s);
//		printf("h, wh = %u, %u\n", this->dom.h(s), static_cast<unsigned int>(this->dom.h(s) * w));
//		printf("h = %d\n", this->dom.weight_h(s));
		n->pop = pop;
		n->parent = p;
		this->dom.pack(n->packed, s);
		return n;
	}

	// use costs_it to remember how far it went before.
	bool terminate() {
		for (int p = costs_it; p < costs.size(); ++p) {
			if (costs[p] == NO_HEURISTIC) {
				return false;
			} else {
				costs_it = p;
			}
		}
		return true;
	}

	// return the number of pattern that matches the node.
	unsigned int match_pattern(typename D::State &s) {
		// TODO: build BDD (or perfect hash) for the patterns.
		unsigned int arg_pat = 0;
//		std::cout << "g: ";
		for (int gs = groups.size() - 1; gs >= 0; --gs) {
			for (int ps = 0; ps < groups[gs].size(); ++ps) {
				if (isContainedSortedVectors(groups[gs][ps], patterns[s.id])) {
//					std::cout << ps << " ";
					arg_pat += ps;
					if (gs != 0) {
						arg_pat *= groups[gs].size();
					}
					break;
				}
				if (groups[gs][ps] == TRUE_PREDICATE) {
//					std::cout << gs << "," << ps << ": -2" << std::endl;
					arg_pat += ps;
					if (gs != 0) {
						arg_pat *= groups[gs].size();
					}
					break;
				}
			}
		}
		return arg_pat;
	}

	std::vector<unsigned int> get_costs() {
		return costs;
	}

	unsigned long expd, gend;
	double wtime, ctime;

};

#endif
