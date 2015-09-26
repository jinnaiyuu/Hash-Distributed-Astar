/*
 * hdastar.hpp
 *
 *  Created on: Jul 17, 2014
 *      Author: yuu
 */

#ifndef HDASTAR_COMB_MPI_HPP_
#define HDASTAR_COMB_MPI_HPP_

#include <vector>
//#include <pthread.h>
//#include <atomic>
#include <unistd.h>
#include <math.h>
#include <algorithm>
#include <array>

#include "../search.hpp"
#include "../utils.hpp"
#include "hashtbl_mpi.hpp"
//#include "hashtblbig.hpp"
#include "../heap.hpp"
//#include "pool.hpp"
//#include "buffer.hpp"

//#include "naive_heap.hpp"

#include "../zobrist.hpp"
//#include "trivial_hash.hpp"
//#include "random_hash.hpp"

#include <mpi/mpi.h>
// DELAY 10,000,000 -> 3000 nodes per second
//#define DELAY 0

#define MPI_MSG_NODE 0 // node
#define MPI_MSG_INCM 1 // for updating incumbent. message is unsigned int.
#define MPI_MSG_TERM 2 // for termination.message is empty.
template<class D, class hash> class HDAstarCombMPI: public SearchAlg<D> {

	struct Node {
		unsigned int f, g;
		unsigned int zbr; // zobrist value. stored here for now. Also the size is char for now.
		int openind;
//		Node *parent; // TODO: somehow need to hold this info
		HashEntry<Node> hentry;
		char pop;
		typename D::PackedState packed;

		const bool pred(Node *o) {
			if (f == o->f)
				return g > o->g;
			return f < o->f;
		}
		const unsigned int size() {
			return 25 + packed.byteSize(); // TODO: packed states should have size function
		}
		HashEntry<Node> &hashentry() {
			return hentry;
		}
		typename D::PackedState &key() {
			return packed;
		}
	};

	/**
	 * Char representation of the Node:
	 *
	 * 0-3:   f
	 * 4-7:   g
	 * 8-11:  zbr
	 * 12-15: openind
	 * 16-19: hash (hentry.hash)
	 * 20:    pop
	 * 21-24: packed size
	 * 25-:   packed state representation
	 *
	 */

	void uintToChars(unsigned int p, unsigned char* d) {
		int n = sizeof p;
		for (int y = 0; n-- > 0; y++)
			d[y] = (p >> (n * 8)) & 0xff;
	}
	template<typename T>
	void charsToData(T& p, unsigned char* d) {
		int n = sizeof p;
		p = 0;
		for (int i = 0; i < n; ++i) {
			p += (T) d[i] << (8 * (n - i - 1));
		}
	}

	void nodeToChars(Node* n, unsigned char* d) {
		uintToChars(n->f, &(d[0]));
		uintToChars(n->g, &(d[4]));
		uintToChars(n->zbr, &(d[8]));
		uintToChars(n->openind, &(d[12]));
		uintToChars(n->hentry.hash, &(d[16]));
		d[20] = n->pop;
		unsigned int state_size = n->packed.byteSize();
		uintToChars(state_size, &(d[21]));

		n->packed.stateToChars(&(d[25])); // TODO: check if its correct
	}

	void charsToNode(unsigned char* d, Node& n) {
		charsToData(n.f, &(d[0]));
		charsToData(n.g, &(d[4]));
		charsToData(n.zbr, &(d[8]));
		charsToData(n.openind, &(d[12]));
		charsToData(n.hentry.hash, &(d[16]));
		n.pop = d[20];
		n.packed.charsToState(&(d[25]));
	}

	void print(Node* n) {
		printf("state = %ud\n", n->packed.word);
		printf("f,g = %ud, %ud\n", n->f, n->g);
		printf("hash = %ud\n", n->hentry.hash);
		printf("openind = %ud\n", n->openind);
		printf("zbr = %ud\n", n->zbr);

	}

//	std::vector<buffer<Node>> income_buffer;
	std::vector<typename D::State> path;
	typename D::State init;
	int tnum;
//	std::atomic<int> thread_id; // set thread id for zobrist hashing.
	hash z; // Members for Zobrist hashing.

//	std::atomic<int> incumbent; // The best solution so far.
	int incumbent;
	std::vector<bool> terminate;
	bool tot_terminate;

	int id; // MPI_RANK

	int income_threshold;
	int outgo_threshold;

	int force_income = 0;
	int force_outgo = 0;

	std::vector<unsigned int> expd_distribution;
	std::vector<unsigned int> gend_distribution;
	std::vector<unsigned int> duplicates;

	std::vector<unsigned int> self_pushes;

	double wall0 = 0; // ANALYZE_FTRACE

	std::vector<int> fvalues; // OUTSOURCING
	std::vector<int> open_sizes;

#ifdef ANALYZE_INCOME
	int max_income = 0;
#endif
#ifdef ANALYZE_OUTGO
	int max_outgo = 0;
#endif
#ifdef ANALYZE_DUPLICATE
//	int* duplicates = 0;
#endif
#ifdef ANALYZE_GLOBALF
	int globalf = 10000000; // Ad hoc number.
#endif

//	int* opensizes; // OUTSOURCING
#ifdef OUTSOURCING
#ifdef ANALYZE_OUTSOURCING
	int outsource_pushed = 0; // ANALYZE_OUTSOURCING
#endif
#endif

	int overrun;
	unsigned int closedlistsize;
	unsigned int openlistsize;
	unsigned int initmaxcost;

//	bool isTimed = false;
//	double timer = 0.0;

public:

	HDAstarCombMPI(D &d, int tnum_, int income_threshold_ = 1000000,
			int outgo_threshold_ = 10000000, int abst_ = 0, int overrun_ = 0,
			unsigned int closedlistsize = 1105036, unsigned int openlistsize =
					100, unsigned int maxcost = 1000000) :
			SearchAlg<D>(d), tnum(tnum_), z(d,
					static_cast<typename hash::ABST>(abst_)), incumbent(
					maxcost), income_threshold(income_threshold_), outgo_threshold(
					outgo_threshold_), overrun(overrun_), closedlistsize(
					closedlistsize), openlistsize(openlistsize), initmaxcost(
					maxcost) {
//		income_buffer.resize(tnum);
		terminate.resize(tnum);
//		tot_terminate.resize(tnum);
		for (int i = 0; i < tnum; ++i) {
			terminate[i] = 0;
		}
		tot_terminate = false;

		expd_distribution.resize(tnum);
		gend_distribution.resize(tnum);
		duplicates.resize(tnum);
		self_pushes.resize(tnum);

//		duplicates = new int[tnum];

		// Fields for Out sourcing
		fvalues.resize(tnum);

//		logfvalue.resize(tnum);
//		logincumbent.resize(tnum);
//		lognodeorder.resize(tnum);
		open_sizes.resize(tnum);
	}

	void set_closedlistsize(unsigned int closedlistsize) {
		this->closedlistsize = closedlistsize;
	}

//      32,334 length 46 : 14 1 9 6 4 8 12 5 7 2 3 0 10 11 13 15
//     909,442 length 53 : 13 14 6 12 4 5 1 0 9 3 10 2 15 11 8 7
//   5,253,685 length 57 : 5 12 10 7 15 11 14 0 8 2 1 13 3 4 9 6
// 565,994,203 length ?? : 14 7 8 2 13 11 10 4 9 12 5 0 3 6 1 15
//	template<class heap>
	void thread_search() {
		MPI_Status status;
		MPI_Request request;
//		int world_size; // = tnum!
//		MPI_Comm_size(MPI_COMM_WORLD, &world_size);

//		int id = thread_id.fetch_add(1);
		// closed list is waaaay too big for my computer.
		// original 512927357
		// TODO: Must optimize these numbers
		//  9999943
		// 14414443
		//129402307
//		HashTable<typename D::PackedState, Node> closed(512927357 / tnum);
		HashTableMPI<typename D::PackedState, Node> closed(closedlistsize);

//	printf("closedlistsize = %u\n", closedlistsize);

//		Heap<Node> open(100, overrun);
		Heap<Node> open(openlistsize, overrun);
//		Pool<Node> nodes(2048);

		// If the buffer is locked when the thread pushes a node,
		// stores it locally and pushes it afterward.
		// TODO: Array of dynamic sized objects.
		// This array would be allocated in heap rather than stack.
		// Therefore, not the best optimized way to do.
		// Also we need to fix it to compile in clang++.
		std::vector<std::vector<Node>> outgo_buffer;
		outgo_buffer.resize(tnum);

		std::vector<Node> tmp;
//		tmp.reserve(10); // TODO: ad hoc random number

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

//		printf("id = %d\n", id);

		unsigned int over_incumbent_count = 0;
		unsigned int no_work_iteration = 0;

		double init_time = walltime();

		printf("id = %d\n", id);

//		while (thread_id < tnum) {
//			;
//		}
		int loop_count = 0;
		while (!tot_terminate) {
			++loop_count;
			if (loop_count % 100000 == 0) {
				printf("loop: %d, expd: %d, gend: %d\n", loop_count, expd_here,
						gend_here);
			}

			//////////////////////////////////////////////////
			// TERMINATION DETECTION
			//////////////////////////////////////////////////
			// TODO: need Mattern, Friedemann. "Algorithms for distributed termination detection." Distributed computing (1987)
			int has_received = 0;
			MPI_Iprobe(MPI_ANY_SOURCE, MPI_MSG_TERM, MPI_COMM_WORLD,
					&has_received, MPI_STATUS_IGNORE); // TODO: Iprobe
			if (has_received != 0) {
				printf("Received termination message!\n");
				return;
			}

			Node *n;

			if (this->isTimed) {
				double t = walltime() - init_time;
//				printf("t = %f\n", t);
				if (t > this->timer) {
//					closed.destruct_all(nodes);
					terminate[id] = true;
					break;
				}
			}

#ifdef ANALYZE_LAP
			startlapse(lapse); // income buffer
#endif

			//////////////////////////////////////////////////
			// TODO: FROM HERE IS THE INCOME QUEUE OPERATION
			//////////////////////////////////////////////////
//			vector<Node>income_node;[
			has_received = 0;
			MPI_Iprobe(MPI_ANY_SOURCE, MPI_MSG_NODE, MPI_COMM_WORLD,
					&has_received, &status); // TODO: Iprobe
			while (has_received != 0) {
				int income_size = 0;
				MPI_Get_count(&status, MPI_BYTE, &income_size);

//				printf("%d recieved %d bytes\n", id, income_size);
				//			tmp.resize(income_size / sizeof(Node));

				unsigned char *d = new unsigned char[income_size];
				MPI_Recv(d, income_size, // TODO: this should be as big as possible
						MPI_BYTE, MPI_ANY_SOURCE, MPI_MSG_NODE, // TODO: this should be NODE and TERMINATE
						MPI_COMM_WORLD, &status);

				//			for (int i = 0; i < income_size; ++i) {
				//				printf("%d:  %d\n", i, (int) d[i]);
				//			}

				Node* tmpn = new Node();
				charsToNode(d, *tmpn);
				open.push(tmpn);
//				printf("f,g = %d %d\n", tmpn->f, tmpn->g);
				delete[] d;

				has_received = 0;
				MPI_Iprobe(MPI_ANY_SOURCE, MPI_MSG_NODE, MPI_COMM_WORLD,
						&has_received, &status); // TODO: Iprobe
			}

//			int size = tmp.size();
//			for (int i = 0; i < size; ++i) {
//				open.push(&tmp[i]); // Not sure optimal or not.
//			}
//			tmp.clear();
//			printf("%d has %d nodes in OPEN\n", id, open.getsize());

//			if (!income_buffer[id].isempty()) {
//				terminate[id] = false;
//				if (income_buffer[id].size() >= income_threshold) {
//					++force_income;
//					income_buffer[id].lock();
//					tmp = income_buffer[id].pull_all_with_lock();
//					income_buffer[id].release_lock();
//					uint size = tmp.size();
//#ifdef ANALYZE_INCOME
//					if (max_income_buffer_size < size) {
//						max_income_buffer_size = size;
//					}
//					dbgprintf("size = %d\n", size);
//#endif // ANALYZE_INCOME
//					for (int i = 0; i < size; ++i) {
//						dbgprintf ("pushing %d, ", i);
//						open.push(tmp[i]); // Not sure optimal or not. Vector to Heap.
//					}
//					tmp.clear();
//				} else if (income_buffer[id].try_lock()) {
//					tmp = income_buffer[id].pull_all_with_lock();
////					printf("%d", __LINE__);
//					income_buffer[id].release_lock();
//
//					uint size = tmp.size();
//#ifdef ANALYZE_INCOME
//					if (max_income_buffer_size < size) {
//						max_income_buffer_size = size;
//					}
//					dbgprintf("size = %d\n", size);
//#endif // ANALYZE_INCOME
//					for (int i = 0; i < size; ++i) {
//						dbgprintf ("pushing %d, ", i);
//						open.push(tmp[i]); // Not sure optimal or not.
//					}
//					tmp.clear();
//				}
//			}
			//////////////////////////////////////////////////
			// TODO: UNTIL HERE IS THE INCOME QUEUE OPERATION
			//////////////////////////////////////////////////

#ifdef ANALYZE_LAPSE
			endlapse(lapse, "incomebuffer");
			startlapse(&lapse); // open list
#endif
#ifdef OUTSOURCING
			open_sizes[id] = open.getsize();
#endif

			// TODO: not sure this gonna cause problem.
//			if (open.isemptyunder(incumbent.load())) {
			if (open.isemptyunder(100000000)) {
				dbgprintf ("open is empty.\n");
				terminate[id] = true;
				if (hasterminated() && incumbent != initmaxcost) {
					printf("terminated\n");
					break;
				}
				++no_work_iteration;
				continue; // ad hoc
			}
			n = static_cast<Node*>(open.pop());
//			printf("%d: f,g = %d, %d\n", id, n->f, n->g);
//			printf("%d\n", n->key());

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
//			if (n->packed == NULL) {
//				std::cout << "null packed state" << std::endl;
//			}
			Node *duplicate = closed.find(n->key());
//			closed.find(n->key());
			if (duplicate) {
				if (duplicate->f <= n->f) {
					dbgprintf ("Discarded\n");
					delete n;
//					nodes.destruct(n);
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

//			for (int t = 0; t < 16; ++t) {
//				printf("%d ", state.tiles[t]);
//			}
//			printf("\n");

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
//				if (state.tiles[1] == 0) {
//					printf("isgoal ERROR\n");
//					continue;
//				}
				std::vector<typename D::State> newpath;

//				for (Node *p = n; p; p = p->parent) {
//					typename D::State s;
//					this->dom.unpack(s, p->packed);
//					newpath.push_back(s); // This triggers the main loop to terminate.
//				}
				int length = newpath.size();
				printf("Goal! length = %d\n", length);
				printf("cost = %u\n", n->g);

				if (incumbent > n->g) {
					// TODO: this should be changed to match non-unit cost domains.
					incumbent = n->g;
//					LogIncumbent* li = new LogIncumbent(walltime() - wall0,
//							incumbent);
//					logincumbent[id].push_back(*li);
					path = newpath;
				}

				// TODO: this termination cause suboptimal solution.
				for (int i = 0; i < tnum; ++i) {
					MPI_Send(NULL, 0, MPI_BYTE, i, MPI_MSG_TERM,
							MPI_COMM_WORLD);
				}
				break; // TODO: this shouldn't be break.

				continue;
			}
#ifdef OUTSOURCING
			if (n->thrown == 0) {
				closed.add(n);
			}
#else
			closed.add(n);
#endif
			expd_here++;
			//		printf("expd: %d\n", id);

#ifdef ANALYZE_LAPSE
			startlapse(&lapse);
#endif

//			buffer<Node>* buffers;

			if (loop_count % 100000 == 0) {
				print_state(state);
			}

			// TODO: this is only applicable for STRIPS planning.
//			std::vector<unsigned int> ops = this->dom.ops(state);
			unsigned int nops = this->dom.nops(state);

			for (int i = 0; i < nops; i++) {
//				for (int i = 0; i < this->dom.nops(state); i++) {
				// op is the next blank position.
				int op = this->dom.nthop(state, i);
//				int op = this->dom.nthop(state, i);
				if (op == n->pop) {
					//					printf("erase %d \n", i);
					continue;
				}

//				printf("gend: %d\n", id);

				useless += uselessCalc(useless);

//				int moving_tile = 0;
//				int blank = 0; // Make this available for Grid pathfinding.
				int moving_tile = state.tiles[op];
				int blank = state.blank; // Make this available for Grid pathfinding.
				Edge<D> e = this->dom.apply(state, op);
				Node* next = wrap(state, n, e.cost, e.pop);

				///////////////////////////
				/// TESTS
				/// Compare nodes, n & next
				/// 1. f value does increase (or same)
				/// 2. g increases by cost
				///
				/// Compare states
				/// 1. operation working fine
				///////////////////////////
//				if (n->f > next->f) {
//					// heuristic was calculating too big.
//					printf("!!!ERROR: f decreases: %u %u\n", n->f, next->f);
//					unsigned int nh = n->f - n->g;
//					unsigned int nxh = next->f - next->g;
//					printf("h = %u %u\n", nh, nxh);
//					printf("cost = %d\n", e.cost);
//				}
//				if (static_cast<unsigned int>(n->g + e.cost) != static_cast<unsigned int>(next->g)) {
//					printf("!!!ERROR: g is wrong: %u + %d != %u\n", n->g, e.cost, next->g);
//				}

//				if (next->f > incumbent.load()) {
////					printf("needless\n");
//					++over_incumbent_count;
//					continue;
//				}
				gend_here++;
				//printf("mv blank op = %d %d %d \n", moving_tile, blank, op);
//				print_state(state);

				// TODO: Make dist hash available for Grid pathfinding.
				// TODO: Make Zobrist hash appropriate for 24 threads.
				next->zbr = z.inc_hash(n->zbr, moving_tile, blank, op,
						state.tiles, state);
//				next->zbr = z.inc_hash(n->zbr, moving_tile, blank, op,
//						0, state);

//				next->zbr = z.inc_hash(state);

				unsigned int zbr = next->zbr % tnum;
//				printf("zbr, zbr_tnum = (%u, %u)\n", next->zbr, zbr);

				// If the node belongs to itself, just push to this open list.
				if (zbr == id) {
//				if (false) {
					// TODO: for debug, let's disable this optimization for now.
					++self_push;
					open.push(next);
//				}
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


				} else {
					outgo_buffer[zbr].push_back(*next);
#ifdef ANALYZE_OUTGO
					int size = outgo_buffer[zbr].size();
					if (size > max_outgo_buffer_size) {
						max_outgo_buffer_size = size;
					}
#endif // ANALYZE_OUTGO
				}

//				printf("%d have nodes to send: ", id);
//				for (int i = 0; i < tnum; ++i) {
//					printf("%d ", outgo_buffer[i].size());
//				}
//				printf("\n");
				//////////////////////////////////////////////////
				// TODO: FROM HERE IS THE OUTGO QUEUE OPERATION
				//////////////////////////////////////////////////
				for (int i = 0; i < tnum; ++i) {
//					if (i != id && outgo_buffer[i].size() > 0) {
					while (outgo_buffer[i].size() > 0) {
						Node send = outgo_buffer[i].back();
//						printf("%d have %d bytes to send to %d...", id,
//								send.size(), i);

						unsigned char* d = new unsigned char[send.size()]; // TODO: not deallocated
						nodeToChars(&send, d);
						MPI_Isend(d, send.size(), MPI_BYTE, i, MPI_MSG_NODE,
								MPI_COMM_WORLD, &request);
//						outgo_buffer[i].clear();
						outgo_buffer[i].pop_back();


//						printf("done!\n");
//						if (income_buffer[i].try_lock()) {
//							// acquired lock
//							// TODO: some error occuring here.
//							income_buffer[i].push_all_with_lock(
//									outgo_buffer[i]);
//							income_buffer[i].release_lock();
//							outgo_buffer[i].clear();
//						}
					}
				}
				//////////////////////////////////////////////////
				// TODO: FROM HERE IS THE OUTGO QUEUE OPERATION
				//////////////////////////////////////////////////

				this->dom.undo(state, e);
			}
#ifdef ANALYZE_LAPSE
			endlapse(lapse, "expand");
#endif
		}

//		if (this->isTimed) {

		for (int i = 0; i < tnum; ++i) {
			terminate[i] = true;
		}
		tot_terminate = true;
//		while (!tot_terminate) {
//			;
//		}
		printf("terminated %d\n", id);
//		}

		// Solved (maybe)

		this->wtime = walltime();
		this->ctime = cputime();

//		printf("useless = %d\n", useless);
//		printf("no_work_iteration = %u\n", no_work_iteration);

		this->open_sizes[id] = open.getsize();

		// From here, you can dump every comments as it would not be included in walltime & cputime.

		//		path =
		dbgprintf ("pathsize = %lu\n", path.size());

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

		this->self_pushes[id] = self_push;

		dbgprintf ("END\n");
		return;
	}

	std::vector<typename D::State> search(typename D::State &init) {
		MPI_Init(NULL, NULL);

		printf("search\n");
		MPI_Comm_rank(MPI_COMM_WORLD, &id);

		this->init = init;

		if (id == 0) {
			// wrap a new node.
			Node* n = new Node;
			{
				n->g = 0;
				n->f = this->dom.h(init);
				n->pop = -1;
				//			n->parent = 0;
				this->dom.pack(n->packed, init);
			}
			//		dbgprintf("zobrist of init = %d", z.hash_tnum(init.tiles));

			unsigned int s = n->size();
			unsigned char *d = new unsigned char[s];
			nodeToChars(n, d);

//			for (int i = 0; i < s; ++i) {
//				printf("%d:  %d\n", i, (int) d[i]);
//			}

			MPI_Send(d, s, MPI_BYTE, 0, MPI_MSG_NODE, MPI_COMM_WORLD);
			delete[] d;
			//		income_buffer[0].push(n);
			//		income_buffer[z.hash_tnum(init.tiles)].push(n);

		}

		wall0 = walltime();
#ifdef OUTSOURCING
		for (int i = 0; i < tnum; ++i) {
			fvalues[i] = n->f;
		}
#endif

		printf("start\n");

//		thread_search<Heap>();
		thread_search();
		MPI_Finalize();

//		for (int i = 0; i < tnum; ++i) {
////			printf("%d\n", i);
//			pthread_create(&(t[i]), NULL,
//					(void*(*)(void*))&HDAstarCombMPI::thread_helper, this);
//				}
//		for (int i = 0; i < tnum; ++i) {
//			pthread_join(t[i], NULL);
//		}

		for (int i = 0; i < tnum; ++i) {
			this->expd += expd_distribution[i];
			this->gend += gend_distribution[i];
			this->push += self_pushes[i];
			this->dup += duplicates[i];
		}

		if (this->isTimed) {
			return path;
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
//#ifdef ANALYZE_INCUMBENT_TRACE
//		for (int i = 0; i < tnum; ++i) {
//			for (int j = 0; j < this->logincumbent[i].size(); ++j) {
//				printf("incumbent %f %d %d\n", logincumbent[i][j].walltime,
//						i, logincumbent[i][j].incumbent);
//			}
//		}
//
//#endif

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

		printf("self pushes =");
		for (int id = 0; id < tnum; ++id) {
			printf(" %d", self_pushes[id]);
		}
		printf("\n");

//		printf("self_pushes: %u\n", self_pushes);

		return path;
	}
//
//	static void* thread_helper(void* arg) {
//		return static_cast<HDAstarCombMPI*>(arg)->thread_search<Heap<Node> >(arg);
////		return static_cast<HDAstarComb*>(arg)->thread_search<NaiveHeap<Node> >(arg);
//	}

	inline Node *wrap(typename D::State &s, Node *p, int c, int pop) {
		Node *n = new Node();
		n->g = c;
		if (p)
			n->g += p->g;
		n->f = n->g + this->dom.h(s);
//		printf("h = %d\n", this->dom.weight_h(s));
		n->pop = pop;
//		n->parent = p;
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

//	template<class H>
//	inline
//	unsigned int inc_hash(unsigned int previous, const int number,
//			const int from, const int to, const char* const newBoard, typename D::State state) {
//		printf("grid inc\n");
//		return z.inc_hash(state);
//	}
//
//	template<>
//	inline
//	unsigned int inc_hash<typename Grid>(const unsigned int previous, const int number,
//			const int from, const int to, const char* const newBoard, typename D::State state) {
//		printf("tile inc\n");
//		return z.inc_hash(previous, number, from, to,
//				newBoard);
//	}

	void print_state(typename D::State state) {
		for (int i = 0; i < D::Ntiles; ++i) {
			printf("%d ", state.tiles[i]);
		}
		printf("\n");
	}

// MAX / AVERAGE
	double load_balance(std::vector<unsigned int> distribution) {
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

	double analyze_distribution(std::vector<unsigned int> distribution) {
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

//	void setTimer(double timer) {
//		isTimed = true;
//		this->timer = timer;
//	}

	void unsetTimer() {
		this->isTimed = false;

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
		// TODO: delay = DELAY
		for (int i = 0; i < 0; ++i) {
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

public:

	std::vector<unsigned int> getExpansions() {
		return expd_distribution;
	}
	std::vector<unsigned int> getGenerations() {
		return gend_distribution;
	}
	std::vector<unsigned int> getSelfPushes() {
		return self_pushes;
	}
	std::vector<unsigned int> getDuplicates() {
		return duplicates;
	}

};

#endif /* HDASTAR_HPP_ */
