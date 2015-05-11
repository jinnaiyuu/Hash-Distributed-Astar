// Copyright 2012 Ethan Burns. All rights reserved.
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file.
#include <stdio.h>

#include "search.hpp"
#include "utils.hpp"
#include "hashtbl.hpp"
#include "heap.hpp"
#include "naive_heap.hpp"
#include "pool.hpp"

template<class D> class Astar: public SearchAlg<D> {

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

	double w;
	unsigned int incumbent;

	std::vector<unsigned int> plan;

public:

	// closed might be waaaay too big for my memory....
	// original 512927357
	// now      200　000　000
	Astar(D &d) :
			SearchAlg<D>(d), closed(512927357), open(120), w(1), incumbent(
					1000000) {
	}

	Astar(D &d, unsigned int opensize) :
			SearchAlg<D>(d), closed(512927357), open(opensize), w(1), incumbent(
					1000000) {
	}

	Astar(D &d, unsigned int opensize, double weight) :
			SearchAlg<D>(d), closed(512927357), open(opensize), w(weight), incumbent(
					1000000) {
	}

	Astar(D &d, unsigned int opensize, double weight, unsigned int incumbent) :
			SearchAlg<D>(d), closed(512927357), open(opensize), w(weight), incumbent(
					incumbent) {
	}

	Astar(D &d, unsigned int opensize, double weight, unsigned int incumbent, unsigned int closed) :
			SearchAlg<D>(d), closed(closed), open(opensize), w(weight), incumbent(
					incumbent) {
	}

	std::vector<typename D::State> search(typename D::State &init) {
		open.push(wrap(init, 0, 0, -1));

		while (!open.isempty() && path.size() == 0) {
//			printf("while loop\n");
			Node *n = static_cast<Node*>(open.pop());
			if (closed.find(n->packed)) {
				nodes.destruct(n);
				continue;
			}
			typename D::State state;
			this->dom.unpack(state, n->packed);

//			printf("expd = %lu\n", this->expd);
//			Grid::State s = static_cast<Grid::State>(state);
//			printf("x = %u\n", state.blank);
//			printf("f,g = %d, %d\n", n->f, n->g);

			if (this->dom.isgoal(state)) {
				printf("goal!\n");
				printf("f = %u\n", n->f);
				for (Node *p = n; p; p = p->parent) {
					typename D::State s;
					this->dom.unpack(s, p->packed);
					path.push_back(s);
				}
				break;
			}

			closed.add(n);

			this->expd++;
//			printf("expd: \n");
			int nops = this->dom.nops(state);

			for (int i = 0; i < nops; i++) {
				int op = this->dom.nthop(state, i);
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
		n->f = n->g + this->dom.h(s) * w;
		unsigned int nw = n->g + this->dom.h(s);
//		printf("h, wh = %u, %u\n", this->dom.h(s), static_cast<unsigned int>(this->dom.h(s) * w));
//		printf("h = %d\n", this->dom.weight_h(s));
		n->pop = pop;
		n->parent = p;
		this->dom.pack(n->packed, s);
		return n;
	}

};
