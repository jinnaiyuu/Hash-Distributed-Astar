/*
 * hdastar.hpp
 *
 *  Created on: Jul 17, 2014
 *      Author: yuu
 */

#ifndef HDASTAR_SHARED_CLOSED_HPP_
#define HDASTAR_SHARED_CLOSED_HPP_

#include <vector>
#include <pthread.h>
#include <atomic>
#include <unistd.h>
#include <math.h>
#include <algorithm>
#include <array>

#include "search.hpp"
#include "utils.hpp"
#include "concurrent_hashtbl.hpp"
#include "heap.hpp"
#include "pool.hpp"
#include "buffer.hpp"

#include "naive_heap.hpp"

#include "zobrist.hpp"
#include "trivial_hash.hpp"
#include "random_hash.hpp"

// DELAY 10,000,000 -> 3000 nodes per second
#define DELAY 0

template<class D, class hash> class HDAstarSharedClosed: public SearchAlg<D> {

	struct Node {
		char f, g, pop;
		unsigned int zbr; // zobrist value. stored here for now. Also the size is char for now.
		int openind;
//		char thrown; // How many times this node has been outsourced.
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

	std::atomic<int> globalOrder;

	struct Logfvalue {
		double walltime;
		int fvalue;
		int hvalue;
		Logfvalue(double walltime_, int fvalue_, int hvalue_) :
				walltime(walltime_), fvalue(fvalue_), hvalue(hvalue_) {
		}
		;
	};
	std::vector<Logfvalue>* logfvalue;

	struct LogIncumbent {
		double walltime;
		int incumbent;
		LogIncumbent(double walltime_, int incumbent_) :
				walltime(walltime_), incumbent(incumbent_) {
		}
		;
	};
	std::vector<LogIncumbent>* logincumbent;

	struct LogNodeOrder {
		int globalOrder;
		uint64_t packedState;
		int fvalue;
		int openlistSize;
		LogNodeOrder(int globalOrder_, char* tiles, int fvalue_ = -1,
				int openlistSize_ = -1) :
				globalOrder(globalOrder_), fvalue(fvalue_), openlistSize(
						openlistSize_) {
			packedState = pack(tiles);
		}
		uint64_t pack(char* tiles) {
			uint64_t word = 0; // to make g++ shut up about uninitialized usage.
			for (int i = 0; i < D::Ntiles; i++) {
				word = (word << 4) | tiles[i];
			}
			return word;
		}
		char* unpack() {
			uint64_t word = packedState;
			char* tiles = new char[D::Ntiles];
			for (int i = D::Ntiles - 1; i >= 0; i--) {
				int t = word & 0xF; // 15 Puzzle specific
				word >>= 4;
				tiles[i] = t;
			}
			return tiles;
		}
	};
	std::vector<LogNodeOrder>* lognodeorder;

	double wall0 = 0; // ANALYZE_FTRACE

	int* open_sizes;

#ifdef ANALYZE_INCOME
	int max_income = 0;
#endif
#ifdef ANALYZE_OUTGO
	int max_outgo = 0;
#endif
#ifdef ANALYZE_DUPLICATE
	int* duplicates = 0;
#endif
#ifdef ANALYZE_GLOBALF
	int globalf = 10000000; // Ad hoc number.
#endif

	int* fvalues; // OUTSOURCING
	int* opensizes; // OUTSOURCING
#ifdef OUTSOURCING
#ifdef ANALYZE_OUTSOURCING
	int outsource_pushed = 0; // ANALYZE_OUTSOURCING
#endif
#endif

	unsigned int self_pushes = 0;

	int overrun;
//	unsigned int closedlistsize;
	ConcurrentHashTable<typename D::PackedState, Node> closed; // shared closed list

public:

	HDAstarSharedClosed(D &d, int tnum_, int income_threshold_ = 1000000,
			int outgo_threshold_ = 10000000, int abst_ = 0, int overrun_ = 0,
			unsigned int closedlistsize = 110503, unsigned int division = 1023) :
			SearchAlg<D>(d), tnum(tnum_), thread_id(0), z(tnum,
					static_cast<typename hash::ABST>(abst_)), incumbent(100000), income_threshold(
					income_threshold_), outgo_threshold(outgo_threshold_), globalOrder(
					0), overrun(overrun_), closed(closedlistsize, division) {
		income_buffer = new buffer<Node> [tnum];
		terminate = new bool[tnum];
		for (int i = 0; i < tnum; ++i) {
			terminate[i] = 0;
		}
		expd_distribution = new int[tnum];
		gend_distribution = new int[tnum];

		duplicates = new int[tnum];

		// Fields for Out sourcing
		fvalues = new int[tnum];

		logfvalue = new std::vector<Logfvalue>[tnum];
		logincumbent = new std::vector<LogIncumbent>[tnum];
		lognodeorder = new std::vector<LogNodeOrder>[tnum];

		open_sizes = new int[tnum];
	}

	void set_closedlistsize(unsigned int closedlistsize) {
//		this->closedlistsize = closedlistsize;
	}

//      32,334 length 46 : 14 1 9 6 4 8 12 5 7 2 3 0 10 11 13 15
//     909,442 length 53 : 13 14 6 12 4 5 1 0 9 3 10 2 15 11 8 7
//   5,253,685 length 57 : 5 12 10 7 15 11 14 0 8 2 1 13 3 4 9 6
// 565,994,203 length ?? : 14 7 8 2 13 11 10 4 9 12 5 0 3 6 1 15
	template<class heap>
	void* thread_search(void * arg) {

		int id = thread_id.fetch_add(1);

		// closed list is waaaay too big for my computer.
		// original 512927357
		// TODO: Must optimize these numbers
		//  9999943
		// 14414443
		//129402307
//		HashTable<typename D::PackedState, Node> closed(512927357 / tnum);
//		HashTable<typename D::PackedState, Node> closed(closedlistsize);

//	printf("closedlistsize = %u\n", closedlistsize);

//		Heap<Node> open(100, overrun);
		heap open(150, overrun);
		Pool<Node> nodes(2048);

		// If the buffer is locked when the thread pushes a node,
		// stores it locally and pushes it afterward.
		// TODO: Array of dynamic sized objects.
		// This array would be allocated in heap rather than stack.
		// Therefore, not the best optimized way to do.
		// Also we need to fix it to compile in clang++.
		std::vector<std::vector<Node*>> outgo_buffer;
		outgo_buffer.reserve(tnum);

		std::vector<Node*> tmp;
		tmp.reserve(10); // TODO: ad hoc random number

		uint expd_here = 0;
		uint gend_here = 0;
		int max_outgo_buffer_size = 0;
		int max_income_buffer_size = 0;
		int duplicate_here = 0;

		int current_f = 0;

		double lapse;

		int useless = 0;

		int fval = -1;

		// How many of the nodes sent to itself.
		// If this high, then lower communication overhead.
		unsigned int self_push = 0;

		//		while (path.size() == 0) {

//	printf("id = %d\n", id);

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
#ifdef OUTSOURCING
			open_sizes[id] = open.getsize();
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
			Logfvalue* lg = new Logfvalue(walltime() - wall0, n->f, n->f - n->g);
			logfvalue[id].push_back(*lg);
			if (fvalues[id] != newf) {
//				printf("ftrace %d %d %f\n", id, fvalues[id],
//						walltime() - wall0);
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
//		if (n->thrown == 0) {
			Node *duplicate = this->closed.find(n->packed);
			if (duplicate) {
				if (duplicate->f <= n->f) {
					dbgprintf("Discarded\n");
					nodes.destruct(n);
					continue;
#ifdef ANALYZE_DUPLICATE
				} else {
					duplicate_here++;
#endif // ANALYZE_DUPLICATE
				}
				// Node access here is unnecessary duplicates.
//					printf("Duplicated\n");
			}
			//	}
#ifdef ANALYZE_LAPSE
			endlapse(lapse, "closedlist");
#endif

#ifdef OUTSOURCING
			if ((n->thrown < 5) && (expd_here > 600) && outsourcing(n, id)) {
				if (n->thrown == 0) {
					closed.add(n);
				}
#ifdef ANALYZE_OUTSOURCING
				outsource_pushed++;
#endif
				dbgprintf("Out sourced a node\n");
				continue;
			}
#endif // OUTSOURCING
			typename D::State state;
			this->dom.unpack(state, n->packed);

#ifdef ANALYZE_ORDER
			if (fval != n->f) {
				fval = n->f;
				LogNodeOrder* ln = new LogNodeOrder(globalOrder.fetch_add(1),
						state.tiles, fval, open.getsize());
				lognodeorder[id].push_back(*ln);
			} else {
				LogNodeOrder* ln = new LogNodeOrder(globalOrder.fetch_add(1),
						state.tiles, -1, open.getsize());
				lognodeorder[id].push_back(*ln);
			}
#endif // ANALYZE_ORDER
			if (this->dom.isgoal(state)) {
				// TODO: For some reason, sometimes pops broken node.
				if (state.tiles[1] == 0) {
					printf("isgoal ERROR\n");
					continue;
				}
//				print_state(state);

				std::vector<typename D::State> newpath;

				for (Node *p = n; p; p = p->parent) {
					typename D::State s;
					this->dom.unpack(s, p->packed);
					newpath.push_back(s); // This triggers the main loop to terminate.
				}
				int length = newpath.size();
				printf("Goal! length = %d\n", length);
				// TODO: need to be atomic. Really?
				if (incumbent > length) {
					incumbent = length;
					LogIncumbent* li = new LogIncumbent(walltime() - wall0,
							incumbent);
					logincumbent[id].push_back(*li);
					path = newpath;
				}

				continue;
			}
#ifdef OUTSOURCING
			if (n->thrown == 0) {
				closed.add(n);
			}
#else
			this->closed.add(n);
#endif
			expd_here++;
			//		printf("expd: %d\n", id);

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

//				printf("gend: %d\n", id);
				gend_here++;

				useless += uselessCalc(useless);

				int moving_tile = state.tiles[op];
				int blank = state.blank;

				Edge<D> e = this->dom.apply(state, op);

				Node* next = wrap(state, n, e.cost, e.pop, nodes);
				//printf("mv blank op = %d %d %d \n", moving_tile, blank, op);
//				print_state(state);

				// TODO: Make Zobrist hash appropriate for 24 threads.
				next->zbr = z.inc_hash(n->zbr, moving_tile, blank, op,
						state.tiles, state);
				unsigned int zbr = next->zbr % tnum;
//				printf("zbr, zbr_tnum = (%u, %u)\n", next->zbr, zbr);

				// If the node belongs to itself, just push to this open list.
				if (zbr == id) {
					++self_push;
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

		// Solved (maybe)

		this->wtime = walltime();
		this->ctime = cputime();

		printf("useless = %d\n", useless);

		this->open_sizes[id] = open.getsize();

		// From here, you can dump every comments as it would not be included in walltime & cputime.

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
		this->duplicates[id] = duplicate_here;
#endif

		self_pushes += self_push;

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
#ifdef OUTSOURCING
		for (int i = 0; i < tnum; ++i) {
			fvalues[i] = n->f;
		}
#endif

		for (int i = 0; i < tnum; ++i) {
			pthread_create(&t[tnum], NULL,
					(void*(*)(void*))&HDAstarSharedClosed::thread_helper, this);
				}
		for (int i = 0; i < tnum; ++i) {
			pthread_join(t[tnum], NULL);
		}

		for (int i = 0; i < tnum; ++i) {
			this->expd += expd_distribution[i];
			this->gend += gend_distribution[i];
		}

#ifdef ANALYZE_INCOME
		printf("average of max_income_buffer_size = %d\n", max_income / tnum);
#endif
#ifdef ANALYZE_OUTGO
		printf("average of max_outgo_buffer_size = %d\n", max_outgo / tnum);
#endif
#ifdef ANALYZE_DUPLICATE
		printf("duplicate_distribution =");
		for(int i=0; i<tnum;++i) {
			printf(" %d", duplicates[i]);
		}
		printf("\n");
#endif
#ifdef ANALYZE_DISTRIBUTION
		printf("expansion balance = %f\n", load_balance(expd_distribution));
		printf("expansion stddev = %f\n",
				analyze_distribution(expd_distribution));
		printf("expansion_distribution =");
		for(int i=0; i<tnum;++i) {
			printf(" %d", expd_distribution[i]);
		}
		printf("\n");

		printf("generation balance = %f\n", load_balance(gend_distribution));
		printf("generation stddev = %f\n",
				analyze_distribution(gend_distribution));
		printf("generation =");
		for(int i=0; i<tnum;++i) {
			printf(" %d", gend_distribution[i]);
		}
		printf("\n");
#endif
		printf("forcepush incomebuffer = %d\n", force_income);
		printf("forcepush outgobuffer = %d\n", force_outgo);

#ifdef ANALYZE_FTRACE
		for (int i = 0; i < tnum; ++i) {
			for (int j = 0; j < this->logfvalue[i].size(); ++j) {
				printf("ftrace %f %d %d %d\n", logfvalue[i][j].walltime,
						i, logfvalue[i][j].fvalue, logfvalue[i][j].hvalue);
			}
		}

#endif
#ifdef ANALYZE_INCUMBENT_TRACE
		for (int i = 0; i < tnum; ++i) {
			for (int j = 0; j < this->logincumbent[i].size(); ++j) {
				printf("incumbent %f %d %d\n", logincumbent[i][j].walltime,
						i, logincumbent[i][j].incumbent);
			}
		}

#endif

#ifdef ANALYZE_ORDER
		for (int id = 0; id < tnum; ++id) {
			for (int i = 0; i < lognodeorder[id].size(); ++i) {
				printf("%d %d %016lx %d %d\n", id, lognodeorder[id][i].globalOrder, lognodeorder[id][i].packedState,
						lognodeorder[id][i].fvalue, lognodeorder[id][i].openlistSize);
			}
		}
#endif // ANALYZE_ORDER
#ifdef ANALYZE_OUTSOURCING
		printf("outsource node pushed = %d\n", outsource_pushed);
#endif

		printf("openlist size =");
		for (int id = 0; id < tnum; ++id) {
			printf(" %d", this->open_sizes[id]);
		}
		printf("\n");

		printf("expd + openlist size =");
		for (int id = 0; id < tnum; ++id) {
			int sum = expd_distribution[id] + this->open_sizes[id];
			printf(" %d", sum);
		}
		printf("\n");

		printf("self_pushes: %u\n", self_pushes);

		return path;
	}

	static void* thread_helper(void* arg) {
		return static_cast<HDAstarSharedClosed*>(arg)->thread_search<Heap<Node> >(arg);
//		return static_cast<HDAstar*>(arg)->thread_search<NaiveHeap<Node> >(arg);
	}

	inline Node *wrap(typename D::State &s, Node *p, int c, int pop,
			Pool<Node> &nodes) {
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

	inline bool hasterminated() {
		for (int i = 0; i < tnum; ++i) {
			if (terminate[i] == false) {
				return false;
			}
		}
		return true;
	}

	void print_state(typename D::State state) {
		for (int i = 0; i < D::Ntiles; ++i) {
			printf("%d ", state.tiles[i]);
		}
		printf("\n");
	}

// MAX / AVERAGE
	double load_balance(int* distribution) {
		double avrg = 0, max = 0;
		for (int i = 0; i < tnum; ++i) {
			avrg += distribution[i];
			if (max < distribution[i]) {
				max = distribution[i];
			}
		}
		avrg /= tnum;
		return max / avrg;
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
//		for (int i = 0; i < tnum; ++i) {
//			printf("%d ", distribution[i]);
//		}
		return stddev;
	}

// CAUTIONS!
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

	int uselessCalc(int useless) {
//		static const int zero = 0;
//		int test = 0;
		int uselessLocal = 0;
		for (int i = 0; i < DELAY; ++i) {
//			++test;
			int l = 0;
			l += useless;
			l = 2 * l - uselessLocal;
			if (l != 0) {
				uselessLocal = l % useless;
			} else {
				uselessLocal = useless * 2;
			}
		}
//		printf("DELAY = %d\n", test);
		return uselessLocal;
	}

// TODO: Parameter would differ for every problem and every environment.
	bool outsourcing(Node *p, int id) {
		static const double threshold = 1.03;
		static const int uneven = 2;

		if (tnum == 1) {
			return false; // Single thread.
		}

		int minsize = 1410065408; // max integer
		int minid = -1;

		for (int i = 0; i < tnum; ++i) {
			if (i != id) {
				if (open_sizes[i] < minsize
						&& open_sizes[id] > open_sizes[i] * threshold) {
					minid = i;
					minsize = open_sizes[i];
				}
			}
		}
		if (minid == -1) {
			dbgprintf("failed %d\n", id);
			return false;
		}
//		p->thrown += 1; // it indicates that the node has outsourced.

		// Should this be try_push?
		income_buffer[minid].push(p);
		dbgprintf("send %d to %d\n", id, minid);
		return true;
	}
};

#endif /* HDASTAR_HPP_ */
