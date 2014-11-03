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
#include <algorithm>
#include <array>

#include "search.hpp"
#include "utils.hpp"
#include "hashtbl.hpp"
#include "heap.hpp"
#include "pool.hpp"
#include "buffer.hpp"

#include "zobrist.hpp"
#include "trivial_hash.hpp"


template<class D, class hash> class HDAstar: public SearchAlg<D> {

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
	hash z; // Members for Zobrist hashing.

	std::atomic<int> incumbent; // The best solution so far.
	bool* terminate;

	int income_threshold;
	int outgo_threshold;

	int force_income = 0;
	int force_outgo = 0;

	int* expd_distribution;
	int* gend_distribution;

#ifdef ANALYZE_INCOME
	int max_income = 0;
#endif
#ifdef ANALYZE_OUTGO
	int max_outgo = 0;
#endif
#ifdef ANALYZE_DUPLICATE
	int duplicate = 0;
#endif
#ifdef ANALYZE_FTRACE
	double wall0 = 0; // ANALYZE_FTRACE
#endif
#ifdef ANALYZE_GLOBALF
	int globalf = 10000000; // Ad hoc number.
#ifndef ANALYZE_FTRACE
	double wall0 = 0;
#endif
#endif
	int* fvalues; // OUTSOURCING

public:

	HDAstar(D &d, int tnum_, int income_threshold_ = 1000000, int outgo_threshold_ = 10000000
			, int abst = 0) :
			SearchAlg<D>(d), tnum(tnum_), thread_id(0), z(tnum, static_cast<typename hash::ABST>(abst)), incumbent(
					100000), income_threshold(income_threshold_), outgo_threshold(outgo_threshold_) {
		income_buffer = new buffer<Node> [tnum];
		terminate = new bool[tnum];
		for (int i = 0; i < tnum; ++i) {
			terminate[i] = 0;
		}
		expd_distribution = new int[tnum];
		gend_distribution = new int[tnum];

		// Fields for Out sourcing
		fvalues = new int[tnum];

	}

	//      32,334 length 46 : 14 1 9 6 4 8 12 5 7 2 3 0 10 11 13 15
	//     909,442 length 53 : 13 14 6 12 4 5 1 0 9 3 10 2 15 11 8 7
	//   5,253,685 length 57 : 5 12 10 7 15 11 14 0 8 2 1 13 3 4 9 6
	// 565,994,203 length ?? : 14 7 8 2 13 11 10 4 9 12 5 0 3 6 1 15

	void* thread_search(void * arg) {

		int id = thread_id.fetch_add(1);
		printf("id = %d\n", id);

		// closed might be waaaay too big for my memory....
		// original 512927357
		// now      200000000
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
		std::vector<std::vector<Node*>> outgo_buffer;
		outgo_buffer.reserve(8);

		std::vector<Node*> tmp;
		tmp.reserve(10); // TODO: ad hoc random number

		uint expd_here = 0;
		uint gend_here = 0;
		int max_outgo_buffer_size = 0;
		int max_income_buffer_size = 0;
		int duplicate_here = 0;

		int current_f = 0;

		double lapse;
		TrivialHash<16> th(tnum);

		//		while (path.size() == 0) {
		while (true) {
			Node *n;

#ifdef ANALYZE_LAP
			startlapse(lapse); // income buffer
#endif
			if (!income_buffer[id].isempty()) {
				terminate[id] = false;
				if (income_buffer[id].size() >= income_threshold) {
					++force_income;
					income_buffer[id].lock();
					tmp = income_buffer[id].pull_all_with_lock();
					income_buffer[id].release_lock();
					uint size = tmp.size();
#ifdef ANALYZE_INCOME
					if (max_income_buffer_size < size) {
						max_income_buffer_size = size;
					}
					dbgprintf("size = %d\n", size);
#endif // ANALYZE_INCOME
					for (int i = 0; i < size; ++i) {
						dbgprintf("pushing %d, ", i);
						open.push(tmp[i]); // Not sure optimal or not. Vector to Heap.
					}
					tmp.clear();
				} else if (income_buffer[id].try_lock()) {
					tmp = income_buffer[id].pull_all_with_lock();
//					printf("%d", __LINE__);
					income_buffer[id].release_lock();

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
#ifdef ANALYZE_LAPSE
			endlapse(lapse, "incomebuffer");
			startlapse(&lapse); // open list
#endif
			if (open.isemptyunder(incumbent.load())) {
				dbgprintf("open is empty.\n");
				terminate[id] = true;
				if (hasterminated() && incumbent != 100000) {
					printf("terminated\n");
					break;
				}
				continue; // ad hoc
			}
			n = static_cast<Node*>(open.pop());
#ifdef ANALYZE_LAPSE
			endlapse(lapse, "openlist");
#endif

#ifdef ANALYZE_FTRACE
			int newf = open.minf();
			if (fvalues[id] != newf) {
				printf("ftrace %d %d %f\n", id, fvalues[id],
						walltime() - wall0);
				fvalues[id] = newf;
			}
#endif // ANALYZE_FTRACE

			// TODO: Might not be the best way.
			// Would there be more novel way?

#ifdef ANALYZE_GLOBALF
			if (n->f != fvalues[id]) {
				fvalues[id] = n->f;
				int min = *std::min_element(fvalues, fvalues+tnum);

				if (min != globalf) {
					globalf = min;
					printf("globalf %d %d %f\n", id, min, walltime() - wall0);
				}
			}
#endif

			// If the new node n is duplicated and
			// the f value is higher than or equal to the duplicate, discard it.
#ifdef ANALYZE_LAPSE
			startlapse(&lapse); // closed list
#endif
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
#ifdef ANALYZE_LAPSE
			endlapse(lapse, "closedlist");
#endif

			typename D::State state;
			this->dom.unpack(state, n->packed);

			if (this->dom.isgoal(state)) {
				// TODO: For some reason, sometimes pops broken node.
				if (state.tiles[1] == 0) {
					printf("isgoal ERROR\n");
					continue;
				}
				printf("GOAL!\n");
				print_state(state);
				std::vector<typename D::State> newpath;

				for (Node *p = n; p; p = p->parent) {
					typename D::State s;
					this->dom.unpack(s, p->packed);
					newpath.push_back(s); // This triggers the main loop to terminate.
				}
				int length = newpath.size();
				dbgprintf("length = %d\n", length);
				// TODO: need to be atomic. Really?
				if (incumbent > length) {
					incumbent = length;
					path = newpath;
				}

				continue;
			}

			closed.add(n);
			expd_here++;
#ifdef ANALYZE_LAPSE
			startlapse(&lapse);
#endif
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

				Edge<D> e = this->dom.apply(state, op);
//				printf("%d %d\n", id, n->zbr);

				Node* next = wrap(state, n, e.cost, e.pop, nodes);
				dbgprintf("mv blank op = %d %d %d \n", moving_tile, blank, op);
				next->zbr = z.inc_hash(n->zbr, moving_tile, blank, op, state.tiles);
//				next->zbr = (n->zbr ^ z.inc_hash(moving_tile, blank, op, state.tiles)) % tnum;
				unsigned long zbr = next->zbr; // % tnum;
				dbgprintf("zbr = %d\n", zbr);

				// If the node belongs to itself, just push to this open list.
				if (zbr == id) {
					open.push(next);
				}
#ifdef SEMISYNC
				// Synchronous communication to avoid search overhead
				else if (outgo_buffer[zbr].size() > outgo_threshold) {
					income_buffer[zbr].lock();
					income_buffer[zbr].push_with_lock(next);
					income_buffer[zbr].push_all_with_lock(outgo_buffer[zbr]);
//					printf("%d", __LINE__);
					income_buffer[zbr].release_lock();
					outgo_buffer[zbr].clear();
#ifdef ANALYZE_SEMISYNC
					++force_outgo;
//					printf("semisync = %d to %d\n", id, zbr);
#endif // ANALYZE_SEMISYNC
				}
#endif // SEMISYNC
				else if (income_buffer[zbr].try_lock()) {
					// if able to acquire the lock, then push all nodes in local buffer.
					income_buffer[zbr].push_with_lock(next);
					if (outgo_buffer[zbr].size() != 0) {
						income_buffer[zbr].push_all_with_lock(
								outgo_buffer[zbr]);
					}
//					printf("%d", __LINE__);
					income_buffer[zbr].release_lock();
					outgo_buffer[zbr].clear();
				} else {
					// Stacking over threshold would not happen so often.
					// Therefore, first try to acquire the lock & then check whether the size
					// exceeds the threshold.
					// For more bigger system, this might change.

					// if the buffer is locked, store the node locally.
					outgo_buffer[zbr].push_back(next);
					// printf("%d: size = %d\n",zbr, outgo_buffer[zbr].size());
#ifdef ANALYZE_OUTGO
					int size = outgo_buffer[zbr].size();
					if (size > max_outgo_buffer_size) {
						max_outgo_buffer_size = size;
					}
#endif // ANALYZE_OUTGO
				}

				this->dom.undo(state, e);
			}
#ifdef ANALYZE_LAPSE
			endlapse(lapse, "expand");
#endif
		}
		//		path =
		dbgprintf("pathsize = %lu\n", path.size());

		this->expd_distribution[id] = expd_here;
		this->gend_distribution[id] = gend_here;

#ifdef ANALYZE_OUTGO
		this->max_outgo += max_outgo_buffer_size;
#endif
#ifdef ANALYZE_INCOME
		this->max_income += max_income_buffer_size;
#endif
#ifdef ANALYZE_DUPLICATE
		this->duplicate += duplicate_here;
#endif

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

#ifdef ANALYZE_FTRACE
		wall0 = walltime();
#else
#ifdef ANALYZE_GLOBALF
		wall0 = walltime();
#endif
#endif
#ifdef OUTSOURCING
		for (int i = 0; i < tnum; ++i) {
			fvalues[i] = n->f;
		}
#endif

		for (int i = 0; i < tnum; ++i) {
			pthread_create(&t[tnum], NULL,
					(void*(*)(void*))&HDAstar::thread_helper, this);
		}
		for (int i = 0; i < tnum; ++i) {
			pthread_join(t[tnum], NULL);
		}

		for (int i = 0; i < tnum; ++i) {
			this->expd += expd_distribution[i];
			this->gend += gend_distribution[i];
		}

		this->wtime = walltime();
		this->ctime = cputime();

#ifdef ANALYZE_INCOME
		printf("average of max_income_buffer_size = %d\n", max_income / tnum);
#endif
#ifdef ANALYZE_OUTGO
		printf("average of max_outgo_buffer_size = %d\n", max_outgo / tnum);
#endif
#ifdef ANALYZE_DUPLICATE
		printf("duplicated nodes = %d\n", duplicate);
#endif
#ifdef ANALYZE_DISTRIBUTION
		printf("expansion distribution = ");
		printf("\nexpansion stddev = %f\n",
				analyze_distribution(expd_distribution));
		printf("generation distribution = ");
		printf("\ngeneration stddev = %f\n",
				analyze_distribution(gend_distribution));
#endif
//#ifdef ANALYZE_OUTSOURCING
//		printf("outsource node pushed = %d\n", outsource_pushed);
//#endif
		printf("forcepush incomebuffer = %d\n", force_income);
		printf("forcepush outgobuffer = %d\n", force_outgo);

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

	double analyze_distribution(int* distribution) {
		double avrg = 0;
		for (int i = 0; i < tnum; ++i) {
			avrg += distribution[i];
		}
		avrg /= tnum;

		double var = 0;
		for (int i = 0; i < tnum; ++i) {
			var += (avrg - distribution[i]) * (avrg - distribution[i]);
		}
		var /= tnum;
		double stddev = sqrt(var);
		for (int i = 0; i < tnum; ++i) {
			printf("%d ", distribution[i]);
		}
		return stddev;
	}


	// CAUTIONS!!!!:
	// These lapsing methods uses cputime methods.
	// It won't take into account for locks or any other synchronous methods.
	void startlapse(double* laptime) {
		*laptime = clock();
//		printf("st %f\n", laptime);
	}

	void endlapse(double previous, const char* comment) {
		double wt = clock() - previous;
//		printf("end %f\n",wt);
		printf("lapse %s %.0f\n", comment, wt);
	}


	// TODO: Parameter would differ for every problem and every environment.

	/*
	 bool outsourcing(Node *p, int id) {
	 if (tnum == 1) {
	 return false;
	 }
	 static const int uneven = 2;
	 int myValue = fvalues[id];

	 int bestf = 100;
	 int bestThread = 100;

	 for (int i = 0; i < tnum; ++i) {
	 if (i != id) {
	 int fi = fvalues[i];
	 // If its not the most busiest thread by far
	 if (myValue < fi - uneven) {
	 if (fi < bestf) {
	 bestf = fi;
	 bestThread = i;
	 }
	 }
	 }
	 }
	 if (bestf == 100) {
	 return false;
	 }
	 dbgprintf("send %d to %d\n", id, bestThread);
	 p->thrown += 1;
	 income_buffer[bestThread].push(p);
	 return true;
	 }
	 */
};

#endif /* HDASTAR_HPP_ */
