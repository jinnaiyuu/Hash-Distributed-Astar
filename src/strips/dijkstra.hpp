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
class Dijkstra {

	const unsigned int SEPARATOR = -1;
	const unsigned int TRUE_PREDICATE = 10000000;
	const unsigned int NO_HEURISTIC = -3;


	struct Node {
		unsigned int f, g;
		char pop;
		int openind;
		Node *parent;
		typename Strips::PackedState packed;
		HashEntry<Node> hentry;

		bool pred(Node *o) {
			if (f == o->f)
				return g > o->g;
			return f < o->f;
		}

		void setindex(int i) {
		}

		const typename Strips::PackedState &key() {
			return packed;
		}

		HashEntry<Node> &hashentry() {
			return hentry;
		}
	};
	Strips &dom;
	HashTable<typename Strips::PackedState, Node> closed;
	Heap<Node> open; // TODO: TODO
	std::vector<typename Strips::State> path;
	Pool<Node> nodes;

	double w;
	unsigned int incumbent;

	std::vector<unsigned int> plan;

	std::vector<std::vector<unsigned int>> patterns;
	std::vector<std::vector<unsigned int>> groups;
	std::vector<unsigned int> ungroupeds;
	std::vector<unsigned int> costs; // costs are set to be infinitie at first.
	unsigned int costs_it;
	unsigned int updated = 0;

public:

	Dijkstra(Strips &d, std::vector<std::vector<unsigned int> > patterns,
			std::vector<std::vector<unsigned int> > groups,
			std::vector<unsigned int> ungroupeds) :
			dom(d), closed(5129273), open(120), w(1), incumbent(1000000), patterns(
					patterns), groups(groups), ungroupeds(ungroupeds), costs(
					patterns.size(), NO_HEURISTIC), costs_it(0), expd(0), gend(0) {
	}

	// TODO: ungroupeds are true for every day.
	// TODO: possibly need to define new domain for pdb.(PDBStrips?)
	void search(typename Strips::State &init, double time) {
		open.push(wrap(init, 0, 0, -1));

		// t is to detect stuck of the search.
		// if update did not change for 2 minutes then the search is stuck.
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
			typename Strips::State state;
			this->dom.unpack(state, n->packed);

//			printf("expd = %lu\n", this->expd);
//			Grid::State s = static_cast<Grid::State>(state);
//			printf("x = %u\n", state.blank);
//			printf("f,g = %d, %d\n", n->f, n->g);

//			if (this->dom.isgoal(state)) {
////				printf("goal!\n");
////				printf("f = %u\n", n->f);
//				for (Node *p = n; p; p = p->parent) {
//					typename Strips::State s;
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
			std::vector<unsigned int> ops = this->dom.r_nops(state);
//			printf("%d ops\n", ops.size());

			for (int i = 0; i < ops.size(); i++) {
				int op = ops[i];
//				printf("op = %u\n", op);
				if (op == n->pop)
					continue;
				Edge<Strips> e = this->dom.r_apply(state, op);

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

		return;
	}

	Node *wrap(typename Strips::State &s, Node *p, int c, int pop) {
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

	// return the number of pattern that matches the node.
	unsigned int match_pattern(Strips::State &s) {
		// TODO: build BDD (or perfect hash) for the patterns.
		unsigned int arg_pat = 0;
//		std::cout << "g: ";
		for (int gs = groups.size() - 1; gs >= 0; --gs) {
			for (int ps = 0; ps < groups[gs].size(); ++ps) {
				if (isContainedSortedVectors(groups[gs][ps], s.propositions)) {
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
//		std::cout << ": " << arg_pat << std::endl;
//		return arg_pat;

//		std::cout << "p: ";
//		unsigned int pt;
//		for (int p = 0; p < patterns.size(); ++p) {
//			if (isContainedSortedVectors(patterns[p], s.propositions)) {
//				pt = p;
//				break;
//			}
//		}
//
//		for (int p = 0; p < patterns[pt].size(); ++p) {
//			std::cout << patterns[pt][p] << " ";
//		}
//		std::cout << ": " << pt << std::endl;
		return arg_pat;

		std::cout << "no pattern to be matched" << std::endl;
		assert(false);
		return -1;
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

	std::vector<unsigned int> get_costs() {
		return costs;
	}

	unsigned long expd, gend;
	double wtime, ctime;

};
