/*
 * hdastar.hpp
 *
 *  Created on: Jul 17, 2014
 *      Author: yuu
 */

#ifndef HDASTAR_HPP_
#define HDASTAR_HPP_

#include <vector>
#include <pthread.h>

#include "search.hpp"
#include "utils.hpp"
#include "hashtbl.hpp"
#include "heap.hpp"
#include "pool.hpp"
#include "buffer.hpp"

#include "zobrist.hpp"

template<class D> class HDAstar: public SearchAlg<D> {

	struct Node {
		char f, g, pop;
	        char zbr; // zobrist value. stored here for now. Also the size is char for now.
		int openind;
		Node *parent;
		typename D::PackedState packed;
		HashEntry<Node> hentry;

		bool pred(Node *o) {
			if (f == o->f)
				return g > o->g;
			return f < o->f;
		}

	        // Unused function and value.
		void setindex(int i) {
		}

		const typename D::PackedState &key() {
			return packed;
		}

		HashEntry<Node> &hashentry() {
			return hentry;
		}
	};

	buffer<Node>* income_buffer;
	std::vector<typename D::State> path;

  // Here for now. 
  Zobrist z;

	int tnum;

	typename D::State init;

public:

	// closed might be waaaay too big for my memory....
	// original 512927357
	// now      200000000
	HDAstar(D &d) :
			SearchAlg<D>(d), tnum(1) {	//:
//			SearchAlg<D>(d), closed(200000000), open(100) {
//		tnum = 1;
		income_buffer = new buffer<Node>[1];
	}

	HDAstar(D &d, int thread_number) :
			SearchAlg<D>(d),tnum(thread_number) {	//:
//			SearchAlg<D>(d), closed(200000000), open(100) {
		income_buffer = new buffer<Node>[tnum];
	}

	// expanded 32334,
	//length 46 : 14 1 9 6 4 8 12 5 7 2 3 0 10 11 13 15
	// 5,934,442 Nodes   : 13 14 6 12 4 5 1 0 9 3 10 2 15 11 8 7
	// 62,643,179 Nodes  : 5 12 10 7 15 11 14 0 8 2 1 13 3 4 9 6
	// 565,994,203 Nodes : 14 7 8 2 13 11 10 4 9 12 5 0 3 6 1 15

	void* thread_search(void * arg) {

		int thrd = 0;
//		buffer<Node> outgo_buffer[tnum];

		// TODO: Must optimize these numbers
		HashTable<typename D::PackedState, Node> closed(200000000/tnum);
		Heap<Node> open(100);
		Pool<Node> nodes(2048);
		uint expd_here = 0;
		uint gend_here = 0;
//		std::vector<typename D::State> path;

		// TODO: Must put initial state here.
		open.push(wrap(init, 0, 0, -1, nodes));
		std::vector<Node*> tmp;

		while (path.size() == 0 && !(open.isempty() && income_buffer[thrd].isempty())) {
//			printf("expd = %d\n", expd_here);
//			printf("buf size = %d\n", income_buffer.size());
			// TODO: Get nodes from income buffer and put it into open list.
			if (!income_buffer[thrd].isempty()) {
				tmp = income_buffer[thrd].pull_all();
			}
			uint size = tmp.size();
			printf("size = %d\n", size);
			for (int i = 0; i < size; ++i) {
//				printf("%d, ",i);
				open.push(tmp[i]);
			}
			tmp.clear();
//			printf("\n");

			Node *n = static_cast<Node*>(open.pop());
			if (closed.find(n->packed)) {
				nodes.destruct(n);
				continue;
			}

			typename D::State state;
			this->dom.unpack(state, n->packed);

//			printf("\n\nExpand:");
//			printf("f,g = %d, %d \n", n->f, n->g);
//			for (int i = 0; i < 16; ++i) {
//				printf("%d ", state.tiles[i]);
//			}
//			printf("\n");

			if (this->dom.isgoal(state)) {
				printf("GOAL!\n");
				for (Node *p = n; p; p = p->parent) {
					typename D::State s;
					this->dom.unpack(s, p->packed);
					path.push_back(s);
				}
				break;
			}

			closed.add(n);

			expd_here++;
			for (int i = 0; i < this->dom.nops(state); i++) {
				int op = this->dom.nthop(state, i);
				if (op == n->pop) {
//					printf("erase %d \n", i);
					continue;
				}

				gend_here++;
				Edge<D> e = this->dom.apply(state, op);


				// TODO: Push items to thread safe income buffer.
				// 		 If the buffer is locked, push the node to local outgo_buffer for now.
//				income_buffer.push(wrap(state, n, e.cost, e.pop, nodes));
				Node* next = wrap(state, n, e.cost, e.pop, nodes);
//				printf("zbr = %d\n", z.hash(state.tiles) % tnum);
				income_buffer[thrd].push(next);
//				printf("Created with %d , f = %d, g = %d\n", i, next->f, next->g);
//				for (int j = 0; j < 16; ++j) {
//					printf("%d ", state.tiles[j]);
//				}
//				printf("\n");

				this->dom.undo(state, e);
			}
		}
		this->expd += expd_here;
		this->gend += gend_here;
		printf("END\n");
		return 0;
	}

	struct targ{
		HDAstar* th;
		int thread_id;
	};

	std::vector<typename D::State> search(typename D::State &init) {
		//AD HOC
//		lds[0]->open.push(lds[0]->wrap(init, 0, 0, -1));
		pthread_t t[tnum];
		this->init = init;

//		Node* n;
//		{
//		n->g = 0;
//		n->f = this->dom.h(init);
//		n->pop = -1;
//		n->parent = 0;
//		this->dom.pack(n->packed, init);
//		}
//		income_buffer.push(n);


		for (int i = 0; i < tnum; ++i) {
			pthread_create(&t[tnum], NULL,
					(void*(*)(void*))&HDAstar::thread_helper, this);
		}
		for (int i = 0; i < tnum; ++i) {
			pthread_join(t[tnum], NULL);
		}

		return path;
	}

	static void* thread_helper(void* arg) {
		return static_cast<HDAstar*>(arg)->thread_search(arg);
	}

	// AD HOC for now (not sure how to factor it in...)

	inline Node *wrap(typename D::State &s, Node *p, int c, int pop,
			Pool<Node> &nodes) {
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

#endif /* HDASTAR_HPP_ */
