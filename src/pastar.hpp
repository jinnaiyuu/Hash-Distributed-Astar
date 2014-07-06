// Copyright 2012 Ethan Burns. All rights reserved.
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file.
#include "search.hpp"
#include "utils.hpp"
#include "hashtbl.hpp"
#include "pheap.hpp"
#include "pool.hpp"
#include <thread>

#include <pthread.h>

template<class D> class Pastar: public SearchAlg<D> {

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
	PHeap<Node> open;
	std::vector<typename D::State> path;
	Pool<Node> nodes;

	bool searching;
	pthread_mutex_t m;

public:

	// closed might be waaaay too big for my memory....
	// original 512,927,357
	// now      200,000,000 200MB
	Pastar(D &d) :
			SearchAlg<D>(d), closed(20000000), open(100), searching(true) {
		pthread_mutex_init(&m, NULL);
	}

	// 546,344 Nodes     : 14 1 9 6 4 8 12 5 7 2 3 0 10 11 13 15
	// 5,934,442 Nodes   : 13 14 6 12 4 5 1 0 9 3 10 2 15 11 8 7
	// 62,643,179 Nodes  : 5 12 10 7 15 11 14 0 8 2 1 13 3 4 9 6
	// 565,994,203 Nodes : 14 7 8 2 13 11 10 4 9 12 5 0 3 6 1 15

	std::vector<typename D::State> search(typename D::State &init) {
		printf("Search\n");
		open.push(wrap(init, 0, 0, -1));
		printf("pushed\n");
		std::thread t1(&Pastar::astar, this);
//		std::thread t2(&Pastar::astar, this);
//		std::thread t3(&Pastar::astar, this);
//		astar();
		t1.join();
//		t2.join();
//		t3.join();
//		pthread_t t1;
//		pthread_t t2;
//		pthread_t t3;
//		pthread_create(&t1, NULL, &astar, this);
//		pthread_create(&t2, NULL, (void*(*)(void*))Pastar::astar, NULL);
//		pthread_create(&t3, NULL, (void*(*)(void*))this->astar, NULL);
//		pthread_join(t1, NULL);
//		pthread_join(t2, NULL);
//		pthread_join(t3, NULL);
		return path;
	}

	void astar() {
//		printf("astar\n");
//		printf("T %ld\n", std::this_thread::get_id());
		while (!open.isempty() && path.size() == 0 && searching) {

			// TODO: open.pop() should be thread safe and n should not duplicate.
//			printf("pastar%d:\n", __LINE__);
			Node *n = static_cast<Node*>(open.pop());

			// TODO: make sure this is thread safe (which is likely to be true)
			if (closed.find(n->packed)) {
				pthread_mutex_lock(&m);
				nodes.destruct(n);
				pthread_mutex_unlock(&m);
				continue;
			}

			typename D::State state;
			this->dom.unpack(state, n->packed);

			// TODO: should wait for other threads if better solution might be possible.
			// Also, we dont need two goal states.
			if (this->dom.isgoal(state)) {
				searching = false;
//				printf("GOOOOOOOOOOOOOOOOOOOOAL\n");
				open.clear();
				for (Node *p = n; p; p = p->parent) {
					typename D::State s;
					this->dom.unpack(s, p->packed);
					path.push_back(s);
				}
				break;
			}

			// TODO: adding a new pair to hash table should avoid collision.
			// Need to check
			closed.add(n);

			this->expd++;

			for (int i = 0; i < this->dom.nops(state); i++) {
//				printf("pastar%d:\n", __LINE__);
				int op = this->dom.nthop(state, i);
				if (op == n->pop)
					continue;
				this->gend++;
				Edge<D> e = this->dom.apply(state, op);

				// TODO: make it thread safe.
				pthread_mutex_lock(&m);
				open.push(wrap(state, n, e.cost, e.pop));
				pthread_mutex_unlock(&m);
				this->dom.undo(state, e);
			}
		}

	}

	Node *wrap(typename D::State &s, Node *p, int c, int pop) {
		Node *n = nodes.construct();
		n->g = c;
		if (p)
			n->g += p->g;
		n->f = n->g + this->dom.h(s);
		n->pop = pop;
		n->parent = p;
		this->dom.pack(n->packed, s);
		return n;
	}
};
