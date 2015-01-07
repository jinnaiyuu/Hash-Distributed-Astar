// Copyright 2012 Ethan Burns. All rights reserved.
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file.
#include "search.hpp"
#include "utils.hpp"
#include "hashtbl.hpp"
#include "heap.hpp"
#include "pool.hpp"

template<class D> class Astar : public SearchAlg<D> {

	struct Node {
		char f, g, pop;
		int openind;
		Node *parent;
		typename D::PackedState packed;
		HashEntry<Node> hentry;

		bool pred(Node *o) {
			if (f == o->f)
				return g > o->g;
			return f < o->f;
		}

 		void setindex(int i) { }

		const typename D::PackedState &key() { return packed; }

		HashEntry<Node> &hashentry() { return hentry; }
	};

	HashTable<typename D::PackedState, Node> closed;
	Heap<Node> open;
	std::vector<typename D::State> path;
	Pool<Node> nodes;

public:

	// closed might be waaaay too big for my memory....
	// original 512927357
	// now      200　000　000
	Astar(D &d) : SearchAlg<D>(d), closed(200000000), open(120) { }


	std::vector<typename D::State> search(typename D::State &init) {
		open.push(wrap(init, 0, 0, -1));

		while (!open.isempty() && path.size() == 0) {
			Node *n = static_cast<Node*>(open.pop());
			if (closed.find(n->packed)) {
				nodes.destruct(n);
				continue;
			}

			typename D::State state;
			this->dom.unpack(state, n->packed);

//			printf("expd = %lu\n", this->expd);
//			printf("f,g = %d, %d, ", n->f, n->g);
//			for (int i = 0; i < 16; ++i) {
//				printf("%d ", state.tiles[i]);
//			}
//			printf("\n");

			if (this->dom.isgoal(state)) {
				for (Node *p = n; p; p = p->parent) {
					typename D::State s;
					this->dom.unpack(s, p->packed);
					path.push_back(s);
				}
				break;
			}

			closed.add(n);

			this->expd++;
			for (int i = 0; i < this->dom.nops(state); i++) {
				int op = this->dom.nthop(state, i);
				if (op == n->pop)
					continue;
				this->gend++;
				Edge<D> e = this->dom.apply(state, op);
				open.push(wrap(state, n, e.cost, e.pop));
				this->dom.undo(state, e);
			}
		}
		return path;
	}

	Node *wrap(typename D::State &s, Node *p, int c, int pop) {
		Node *n = nodes.construct();
		n->g = c;
		if (p)
			n->g += p->g;
		n->f = n->g + this->dom.h(s);
//		printf("h = %d\n", this->dom.weight_h(s));
		n->pop = pop;
		n->parent = p;
		this->dom.pack(n->packed, s);
		return n;
	}
};
