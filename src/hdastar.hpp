/*
 * hdastar.hpp
 *
 *  Created on: Jul 17, 2014
 *      Author: yuu
 */

/**
 * TODO:
 * 	1. Incremental Zobrist hashing
 * 	2. Implement try_lock
 *
 */

#ifndef HDASTAR_HPP_
#define HDASTAR_HPP_

#include <vector>
#include <pthread.h>
#include <atomic>
#include <unistd.h>

#include "search.hpp"
#include "utils.hpp"
#include "hashtbl.hpp"
#include "heap.hpp"
#include "pool.hpp"

#include "buffer.hpp"
#include "zobrist.hpp"

//#define INCOME_ANALYZE
//#define OUTGO_ANALYZE

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
	typename D::State init;
	int tnum;
	std::atomic<int> thread_id; // set thread id for zobrist hashing.
	Zobrist<16> z; // Members for Zobrist hashing.

	std::atomic<int> incumbent; // The best solution so far.

	int max_income;
	int max_outgo; // For analyzing the size of outgo_buffer;
//	int outgo_threshould = 10000;

public:

	// closed might be waaaay too big for my memory....
	// original 512927357
	// now      200000000
	HDAstar(D &d) :
			SearchAlg<D>(d), tnum(1), thread_id(0), z(1), max_income(0), max_outgo(
					0) {	//:
//			SearchAlg<D>(d), closed(200000000), open(100) {
//		tnum = 1;
		income_buffer = new buffer<Node> [1];
	}

	HDAstar(D &d, int tnum_) :
			SearchAlg<D>(d), tnum(tnum_), thread_id(0), z(tnum), max_income(0), max_outgo(
					0) {	//:
//			SearchAlg<D>(d), closed(200000000), open(100) {
		income_buffer = new buffer<Node> [tnum];
	}

	// expanded 32334,

	// length 46 : 14 1 9 6 4 8 12 5 7 2 3 0 10 11 13 15

	// 5,934,442 Nodes   : 13 14 6 12 4 5 1 0 9 3 10 2 15 11 8 7
	// 62,643,179 Nodes  : 5 12 10 7 15 11 14 0 8 2 1 13 3 4 9 6
	// 565,994,203 Nodes : 14 7 8 2 13 11 10 4 9 12 5 0 3 6 1 15

	void* thread_search(void * arg) {

		int thrd = thread_id.fetch_add(1);
		dbgprintf("thrd = %d\n", thrd);
//		buffer<Node> outgo_buffer[tnum];

		// TODO: Must optimize these numbers
		HashTable<typename D::PackedState, Node> closed(200000000 / tnum);
		Heap<Node> open(100);
		Pool<Node> nodes(2048);

		// If the buffer is locked when the thread pushes a node,
		// stores it locally and pushes it afterward.
		std::vector<Node*> outgo_buffer[tnum];

		// TODO: temporal buffer to store nodes from income buffer.
		std::vector<Node*> tmp;

		uint expd_here = 0;
		uint gend_here = 0;
		int max_outgo_buffer_size = 0;
		int max_income_buffer_size = 0;

		while (path.size() == 0) {
//			&&  !(open.isempty() && income_buffer[thrd].isempty())
//			dbgprintf("id: %d\n"
//					"expd = %d\n"
//					"bufsize = %d\n", thrd, expd_here,
//					income_buffer[thrd].size());
//			sleep(1);
//			printf("expd = %d\n", expd_here);
//			printf("buf size = %d\n", income_buffer[thrd].size());
			// TODO: Get nodes from income buffer and put it into open list.
			if (!income_buffer[thrd].isempty()) {
				tmp = income_buffer[thrd].pull_all();
			}
			uint size = tmp.size();
#ifdef INCOME_ANALYZE
			if (max_income_buffer_size < size) {
				max_income_buffer_size = size;
			}
			dbgprintf("size = %d\n", size);
#endif // INCOME_ANALYZE
			for (int i = 0; i < size; ++i) {
				dbgprintf("pushing %d, ", i);
				open.push(tmp[i]); // Not sure optimal or not.
			}
			tmp.clear();
//			printf("\n");

			// TODO: minf(int f):
			if (open.isempty()) {
				dbgprintf("open is empty.\n");
				continue; // ad hoc
			}
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

			// TODO: incumbent solution.
			if (this->dom.isgoal(state)) {
				dbgprintf("GOAL!\n");
				std::vector<typename D::State> newpath;

				for (Node *p = n; p; p = p->parent) {
					typename D::State s;
					this->dom.unpack(s, p->packed);
					newpath.push_back(s); // This triggers the main loop to terminate.
				}
				path = newpath;
//				length = newpath.size();
//
//				// TODO: need it to be atomic.
//				if (incumbent > length) {
//					incumbent = length;
//				}

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
//				int zbr = n->zbr ^ z.hashinc(state.tiles, state.blank, op);
				int zbr = z.hash_tnum(state.tiles);
				dbgprintf("zbr = %d\n", zbr);

				// TODO: trylock
				// Need to store to local buffer and go to next node.
//				income_buffer[zbr].push(next);

				//try_push return false when fail to lock.
				if (income_buffer[zbr].try_lock()) {
					// if able to acquire the lock, then push all nodes in local buffer.
					income_buffer[zbr].push_with_lock(next);
					income_buffer[zbr].push_all_with_lock(outgo_buffer[zbr]);
					outgo_buffer[zbr].clear();
					income_buffer[zbr].release_lock();
				} else {
					// Stacking over threshould would not happen so often.
					// Therefore, first try to acquire the lock & then check whether the size
					// exceeds the threshould.
					// For more bigger system, this might change.
//					if (outgo_buffer[zbr].size() > outgo_threshould) {
//						income_buffer[zbr].lock();
//						income_buffer[zbr].push_with_lock(next);
//						income_buffer[zbr].push_all_with_lock(
//								outgo_buffer[zbr]);
//						outgo_buffer[zbr].clear();
//						income_buffer[zbr].release_lock();
//					} else {
						// if the buffer is locked, store the node locally.
						outgo_buffer[zbr].push_back(next);
						// printf("%d: size = %d\n",zbr, outgo_buffer[zbr].size());
#ifdef OUTGO_ANALYZE
						int size = outgo_buffer[zbr].size();
						if (size > max_outgo_buffer_size) {
							max_outgo_buffer_size = size;
						}
#endif //OUTGO_ANALYZE
//					}
				}

//				if (!income_buffer[zbr].try_push(next)) { //try_push return false when fail to lock.
//					// failed to acquire the lock.
//					printf("Try_push failed.\n");
//					outgo_buffer[zbr].push_back(next);
////					income_buffer[zbr].push(next);
//				} else { // when succeeds to
//
//				}
//				}

//				printf("Created with %d , f = %d, g = %d\n", i, next->f, next->g);
//				for (int j = 0; j < 16; ++j) {
//					printf("%d ", state.tiles[j]);
//				}
//				printf("\n");

				this->dom.undo(state, e);
			}
		}
		dbgprintf("pathsize = %lu\n", path.size());
		this->expd += expd_here;
		this->gend += gend_here;

		this->max_outgo += max_outgo_buffer_size;
		this->max_income += max_income_buffer_size;
		printf("END\n");
		return 0;
	}

//	struct targ{
//		HDAstar* th;
//		int thread_id;
//	};

	std::vector<typename D::State> search(typename D::State &init) {
		//AD HOC
//		lds[0]->open.push(lds[0]->wrap(init, 0, 0, -1));
		pthread_t t[tnum];
		this->init = init;

		// wrap a new node.
		Node* n = new Node;
		{
			n->g = 0;
			n->f = this->dom.h(init);
			n->pop = -1;
			n->parent = 0;
			this->dom.pack(n->packed, init);
		}
		dbgprintf("zobrist of init = %d", z.hash_tnum(init.tiles));

		income_buffer[z.hash_tnum(init.tiles)].push(n);

		for (int i = 0; i < tnum; ++i) {
			pthread_create(&t[tnum], NULL,
					(void*(*)(void*))&HDAstar::thread_helper, this);
		}
		for (int i = 0; i < tnum; ++i) {
			pthread_join(t[tnum], NULL);
		}

		printf("average of max_income_buffer_size = %d\n", max_income / tnum);
		printf("average of max_outgo_buffer_size = %d\n", max_outgo / tnum);

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
