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
#include <atomic>
#include <unistd.h>
#include <math.h>

#include "search.hpp"
#include "utils.hpp"
#include "hashtbl.hpp"
#include "heap.hpp"
#include "pool.hpp"

#include "buffer.hpp"
#include "zobrist.hpp"

//#define ANALYZE_INCOME
//#define ANALYZE_OUTGO
//#define ANALYZE_DUPLICATE
//#define ANALYZE_DISTRIBUTION
#define ANALYZE_FTRACE

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

	bool* terminate;

	int max_income;
	int max_outgo; // For analyzing the size of outgo_buffer;
	int duplicate;

	int* expd_distribution;

	double wall0; // wall time to trace the move of f value

public:

	// closed might be waaaay too big for my memory....
	// original 512927357
	// now      200000000

	HDAstar(D &d, int tnum_ = 0) :
			SearchAlg<D>(d), tnum(tnum_), thread_id(0), z(tnum), incumbent(100), max_income(
					0), max_outgo(0), duplicate(0) {	//:
		//			SearchAlg<D>(d), closed(200000000), open(100) {
		income_buffer = new buffer<Node> [tnum];
		terminate = new bool[tnum];
		expd_distribution = new int[tnum];
		//		terminate_code = (2 << (tnum+1)) - 1;
		//		printf("terminate code = %d", terminate_code);
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
		// TODO: Array of dynamic sized objects.
		// This array would be allocated in heap rather than stack.
		// Therefore, not the best optimized way to do.
		// Also we need to fix it to compile in Clang++.
		std::vector<Node*> outgo_buffer[tnum];

		// TODO: temporal buffer to store nodes from income buffer.
		// Might need to optimize with reserve().
		std::vector<Node*> tmp;
		tmp.reserve(10);

		uint expd_here = 0;
		uint gend_here = 0;
		int max_outgo_buffer_size = 0;
		int max_income_buffer_size = 0;
		int duplicate_here = 0;

		int current_f = 0;

		//		while (path.size() == 0) {
		while (true) {
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
				terminate[thrd] = false;
				if (income_buffer[thrd].try_lock()) {
					tmp = income_buffer[thrd].pull_all_with_lock();
					income_buffer[thrd].release_lock();

					uint size = tmp.size();
#ifdef ANALYZE_INCOME
					if (max_income_buffer_size < size) {
						max_income_buffer_size = size;
					}
					dbgprintf("size = %d\n", size);
#endif // ANALYZE_INCOME
					for (int i = 0; i < size; ++i) {
						dbgprintf("pushing %d, ", i);
						open.push(tmp[i]); // Not sure optimal or not.
					}
					tmp.clear();
				}
			}
			//			printf("\n");

			// TODO: minf(int f):
			//			printf("incumbent = %d \n", incumbent.load());
			//			bool em = open.isemptyunder(incumbent.load());
			//			printf("inc = %d, isemptyunber = %d\n", incumbent.load(), em);
			if (open.isemptyunder(incumbent.load())) {
				dbgprintf("open is empty.\n");
				terminate[thrd] = true;
				if (hasterminated()) {
					break;
				}
				continue; // ad hoc
			}
			dbgprintf("incumbent = %d, open.min = %d\n", incumbent.load(),
					open.minf());

			Node *n = static_cast<Node*>(open.pop());

#ifdef ANALYZE_FTRACE
			if (n->f != current_f) {
				current_f = n->f;
				printf("ftrace %d %d %f\n", thrd, current_f, walltime() - wall0);
			}
#endif //ANALYZE_FTRACE

			// If the new node n is duplicated and
			// the f value is higher than or equal to the duplicate, discard it.
			Node *duplicate = closed.find(n->packed);
			if (duplicate) {
				if (duplicate->f <= n->f) {
					dbgprintf("Discarded\n");
					nodes.destruct(n);
					continue;
				}
				// Node access here is unnecessary duplicates.
				dbgprintf("Duplicated\n");
#ifdef ANALYZE_DUPLICATE
				duplicate_here++;
#endif // ANALYZE_DUPLICATE
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
				// TODO: For some reason, sometimes pops broken node.
				if (state.tiles[1] == 0) {
					printf("ERROR\n");
					return 0;
				}
				dbgprintf("GOAL!\n");
				std::vector<typename D::State> newpath;

				for (Node *p = n; p; p = p->parent) {
					typename D::State s;
					this->dom.unpack(s, p->packed);
					newpath.push_back(s); // This triggers the main loop to terminate.
				}
//				for (auto iter = newpath.end() - 1; iter != newpath.begin() - 1; --iter) {
//					for (int i = 0; i < 16; ++i) {
//						printf("%2d ", iter->tiles[i]);
//					}
//					printf("\n");
//				}
				//				path = newpath;
				int length = newpath.size();
				dbgprintf("length = %d\n", length);
				//				TODO: need it to be atomic.
				if (incumbent > length) {
					incumbent = length;
					path = newpath;
				}

				continue;
			}

			closed.add(n);

			expd_here++;
			for (int i = 0; i < this->dom.nops(state); i++) {
				// op is the next blank position.
				int op = this->dom.nthop(state, i);
				if (op == n->pop) {
					//					printf("erase %d \n", i);
					continue;
				}

				gend_here++;
				int moving_tile = state.tiles[op];
				int blank = state.blank;
//				printf("\n\nTile to move = %d, ", moving_tile);
//				printf("op = %d\n", op);
//				printf("Before:\n");
//				print_state(state);

				Edge<D> e = this->dom.apply(state, op);
//				printf("After:\n");
//				print_state(state);

				// TODO: Push items to thread safe income buffer.
				// 		 If the buffer is locked, push the node to local outgo_buffer for now.
				//				income_buffer.push(wrap(state, n, e.cost, e.pop, nodes));
				Node* next = wrap(state, n, e.cost, e.pop, nodes);


				dbgprintf("mv blank op = %d %d %d \n", moving_tile, blank, op);
				int zbr = (n->zbr ^ z.inc_hash_tnum(moving_tile, blank, op));
				dbgprintf("inc_zbr_tnum = %d, ", z.inc_hash_tnum(moving_tile, blank, op));
//				int zbr = z.hash_tnum(state.tiles);
				dbgprintf("zbr = %d\n", zbr);

				// TODO: trylock
				// Need to store to local buffer and go to next node.
				//				income_buffer[zbr].push(next);

				// If the node belongs to itself, just push to this open list.
				if (zbr == thrd) {
					open.push(next);
				} else if (income_buffer[zbr].try_lock()) {
					// if able to acquire the lock, then push all nodes in local buffer.
					income_buffer[zbr].push_with_lock(next);
					income_buffer[zbr].push_all_with_lock(outgo_buffer[zbr]);
					income_buffer[zbr].release_lock();
					outgo_buffer[zbr].clear();
				} else {
					// Stacking over threshold would not happen so often.
					// Therefore, first try to acquire the lock & then check whether the size
					// exceeds the threshold.
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
#ifdef ANALYZE_OUTGO
					int size = outgo_buffer[zbr].size();
					if (size > max_outgo_buffer_size) {
						max_outgo_buffer_size = size;
					}
#endif // ANALYZE_OUTGO
					//					}
				}

				this->dom.undo(state, e);
			}
		}
		//		path =
		dbgprintf("pathsize = %lu\n", path.size());
		this->expd += expd_here;
		this->gend += gend_here;

		this->max_outgo += max_outgo_buffer_size;
		this->max_income += max_income_buffer_size;
		this->duplicate += duplicate_here;
		this->expd_distribution[thrd] = expd_here;

		dbgprintf("END\n");
		return 0;
	}

	std::vector<typename D::State> search(typename D::State &init) {

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

		wall0 = walltime();

		for (int i = 0; i < tnum; ++i) {
			pthread_create(&t[tnum], NULL,
					(void*(*)(void*))&HDAstar::thread_helper, this);
		}
		for (int i = 0; i < tnum; ++i) {
			pthread_join(t[tnum], NULL);
		}

		// TODO: Time to printing these texts are included in the walltime.
		printf("average of max_income_buffer_size = %d\n", max_income / tnum);
		printf("average of max_outgo_buffer_size = %d\n", max_outgo / tnum);
		printf("duplicated nodes = %d\n", duplicate);

		// TODO: pack to a method
#ifdef ANALYZE_DISTRIBUTION
		analyze_distribution();
#endif // ANALYZE_DISTRIBUTION
		return path;
	}

	static void* thread_helper(void* arg) {
		return static_cast<HDAstar*>(arg)->thread_search(arg);
	}

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

	inline bool hasterminated() {
		for (int i = 0; i < tnum; ++i) {
			if (terminate[i] == false) {
				return false;
			}
		}
		return true;
	}

	void print_state(typename D::State state) {
		for (int i = 0; i < 16; ++i) {
			printf("%d ", state.tiles[i]);
		}
		printf("\n");
	}

	void analyze_distribution() {
		int avrg = 0;
		for (int i = 0; i < tnum; ++i) {
			avrg += expd_distribution[i];
		}
		avrg /= tnum;

		int var = 0;
		for (int i = 0; i < tnum; ++i) {
			var +=  (avrg - expd_distribution[i]) * (avrg - expd_distribution[i]);
		}
		var /= tnum;
		double stddev = sqrt(var);
		printf("Expansion of each thread = ");
		for (int i = 0; i < tnum; ++i) {
			printf("%d ", expd_distribution[i]);
		}
		printf("\n");
		printf("distribution: stddev = %f\n", stddev);
	}

};

#endif /* HDASTAR_HPP_ */
