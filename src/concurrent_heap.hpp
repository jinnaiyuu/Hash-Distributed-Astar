// Copyright 2012 Ethan Burns. All rights reserved.
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file.
#ifndef _CONCURRENT_HEAP_HPP_
#define _CONCURRENT_HEAP_HPP_

#include "heap.hpp"
#include "fatal.hpp"
#include <vector>
#include <queue>
#include <limits>
#include <cassert>
#include <pthread.h>

#define MAX_PRIORITY 100000000
//#define MIN_EXPD 100

// HeapElm is Node for here.

// TODO: not sure this implementation is optimal or not.
//       In the
/*struct HeapPriority {
 int priority;
 unsigned int which_heap;
 bool operator<(const HeapPriority& lhs, const HeapPriority& rhs)
 {
 return lhs.priority < rhs.priority;
 }
 };*/

template<class HeapElm> class ConcurrentHeap {

	unsigned int n_heaps;
	std::vector<Heap<HeapElm> > heaps;
	std::vector<pthread_mutex_t> ms;

	bool is_push_synchronous;
	unsigned int is_pop_synchronous;

	std::vector<int> fs;
	std::vector<int> priorities; // might be a good idea to implement in terms of heap.

	// Optimization for search vs. coordination overhead.
	const unsigned int min_pop;
	std::vector<unsigned int> which_heap;
	std::vector<unsigned int> n_poped;
//	int best_heap;

	int fill, min;

public:
	ConcurrentHeap(unsigned int sz, unsigned int n_heaps,
			unsigned int n_threads, bool synchronous_push,
			unsigned int synchronous_pop, unsigned int min_expd) :
			n_heaps(n_heaps), heaps(n_heaps, Heap<HeapElm>(sz)), ms(n_heaps), is_push_synchronous(
					synchronous_push), is_pop_synchronous(synchronous_pop), fs(
					n_heaps, MAX_PRIORITY), priorities(n_heaps, MAX_PRIORITY), min_pop(
							min_expd), which_heap(n_threads), n_poped(n_threads, min_expd), fill(0), min(
					0) {
		for (unsigned int i = 0; i < n_heaps; ++i) {
			pthread_mutex_init(&(ms[i]), NULL);
		}
	}

	static const char *kind(void) {
		return "2d bucketed multiheap";
	}

	// interfaces

	// TODO: Check the hash value of the heap element and push to the heap according to the value.
	void push(HeapElm *n, unsigned int thread_id = 1) {
		unsigned int key = n->zbr % n_heaps;
//		printf("push key = %u\n", key);

		// TODO: implement try_lock

		if (is_push_synchronous) {
			// synchronous push
			pthread_mutex_lock(&(ms[key]));
//		printf("locked");
//		printf("pushed");
		} else {
			// asynchronous push
			// access to other heap if the best heap is occupied.
			if (pthread_mutex_trylock(&(ms[key % n_heaps])) != 0) {
//				printf("locked push: %u\n", key % n_heaps);
				// For the first try, shift with the number of thread_id.
				// This will resolve further contentions.
				key += thread_id * 19; // Random prime
				while (pthread_mutex_trylock(&(ms[key % n_heaps])) != 0) {
					++key;
				}
			}
			key = key % n_heaps;
		}
		assert(0 <= key && key < n_heaps);
		heaps[key].push(n);
		pthread_mutex_unlock(&(ms[key]));

		// TODO:
		//       Priority can be kept loosely.
		//       As it is parallel search after all, completely strict priority is impossible.
		//       Loose priority can improve node per second on search.
		// Iteas are:
		// 1. Minimum searching nodes. (idea from PBNF)
		// 2. Expand other heap if the best heap is occupied.
		priorities[key] = heaps[key].getpriority(); // TODO: can be optimized?
		++fill;

//		printf("unlocked");
//		fs[key] = heaps[key].minf();

//		printf("push done\n");

	}

	//       1. see the top nodes for each heaps.
	// 	     2. Get the lock for the best node.
	//       3. If locked, then get from the second best heap.
	HeapElm *pop(unsigned int thread_id) {
		unsigned int test = 0;
//		printf("pop tnum = %u\n", thread_id);
		HeapElm *n = nullptr;
		unsigned int key = thread_id % n_heaps; // usually thread_id < n_heaps
		unsigned int min_priority = MAX_PRIORITY;

		// Change the rounding way for each threads so that collision would decrease.
//		while (n == nullptr) {
//		printf("loop = %u\n", ++test);
//		printf("key = %u\n", key);
//		printf("total size = %u\n", fill);

		/*		printf("sizes = ");
		 for (unsigned int i = 0; i < n_heaps; ++i) {
		 printf("%u ", heaps[i].getsize());
		 }
		 printf("\n");*/
//		printf("ispop = %u\n", is_pop_synchronous);
		if (is_pop_synchronous == 0 || is_pop_synchronous == 1) {
			for (unsigned int i = thread_id; i < thread_id + n_heaps; ++i) {
				unsigned int h = i % n_heaps;
//				printf("priorities[%u] = %u\n", h, priorities[h]);
				if (min_priority > priorities[h]) {
					min_priority = priorities[h];
					key = h;
				}
			}
			if (is_pop_synchronous == 1) {
				// Synchronous
				pthread_mutex_lock(&(ms[key]));
				if (heaps[key].isempty()) { // Only occurs first 0.1 seconds.
//			printf("EMPTY\n");
					priorities[key] = MAX_PRIORITY;
					pthread_mutex_unlock(&(ms[key]));
					return NULL;
				} else {
					n = heaps[key].pop();
					if (!heaps[key].isempty()) {
						priorities[key] = heaps[key].getpriority();
					} else {
						priorities[key] = MAX_PRIORITY;
					}
					pthread_mutex_unlock(&(ms[key]));
				}
			} else {
				// Asynchronous
				if (pthread_mutex_trylock(&(ms[key % n_heaps])) != 0) {
//					printf("locked pop: %u\n", key);
					// For the first try, shift with the number of thread_id.
					// This will resolve further contentions.
					key += thread_id;
					while (pthread_mutex_trylock(&(ms[key % n_heaps])) != 0) {
						++key;
					}
				}
				key = key % n_heaps;
				if (heaps[key].isempty()) {
					//			printf("EMPTY\n");
					priorities[key] = MAX_PRIORITY;
					pthread_mutex_unlock(&(ms[key]));
					return NULL;
				} else {
					n = heaps[key].pop();
					if (!heaps[key].isempty()) {
						priorities[key] = heaps[key].getpriority();
					} else {
						priorities[key] = MAX_PRIORITY;
					}
					pthread_mutex_unlock(&(ms[key]));
				}
			}
		} else if (is_pop_synchronous == 2){
//			printf("sync_2\n");
			// Minimum expansion for heap
			// a thread continues to expand from one heap.
			// a thread switches heaps if
			//    1. poped more than min_pop (idea from PBNF)
			//    2. locked
			//    3. the heap is empty (only for the first 0.1 second)
			if (n_poped[thread_id] < min_pop &&
					!heaps[which_heap[thread_id]].isempty() &&
					 (pthread_mutex_trylock(&(ms[which_heap[thread_id]])) == 0) ) {
//				printf("O\n");
				++n_poped[thread_id];
				key = which_heap[thread_id];
			} else {
//				printf("X\n");
				for (unsigned int i = thread_id; i < thread_id + n_heaps; ++i) {
					unsigned int h = i % n_heaps;
					if (min_priority > priorities[h]) {
						min_priority = priorities[h];
						key = h;
					}
				}
				if (!heaps[key % n_heaps].isempty() &&
						pthread_mutex_trylock(&(ms[key % n_heaps])) != 0) {
//					printf("locked pop: %u\n", key);
					// For the first try, shift with the number of thread_id.
					// This will resolve further contentions.
					key += thread_id * 19;
					while (!heaps[key % n_heaps].isempty() &&
							pthread_mutex_trylock(&(ms[key % n_heaps])) != 0) {
						++key;
					}
				}
				key = key % n_heaps;
				which_heap[thread_id] = key;
				n_poped[thread_id] = 0;
			}
			// At this point thread has a lock.
//			printf("pop\n");
			if (heaps[key].isempty()) {
//				printf("empty pop\n");
				return NULL;
			}
			n = heaps[key].pop();
			if (!heaps[key].isempty()) {
				priorities[key] = heaps[key].getpriority();
			} else {
				priorities[key] = MAX_PRIORITY;
			}
			pthread_mutex_unlock(&(ms[key]));

		}
		// here it got a key for

		/*
		 if (!heaps[key].isempty()) {
		 n = heaps[key].pop();
		 if (!heaps[key].isempty()) {
		 priorities[key] = heaps[key].getpriority(); // TODO: can be optimized
		 } else {
		 priorities[key] = MAX_PRIORITY;
		 }
		 fill--;
		 pthread_mutex_unlock(&(ms[key]));
		 } else {
		 priorities[key] = MAX_PRIORITY;
		 n = nullptr;
		 pthread_mutex_unlock(&(ms[key]));
		 return nullptr;
		 }
		 */

		dbgprintf("f = %d\n", min);
//		printf("pop done\n");
		return n;
	}

	void pre_update(HeapElm*n) {
		printf("WARNING: pre_update\n");
		/*		if (n->openind < 0)
		 return;
		 assert ((unsigned int) n->f < qs.size());
		 qs[n->f].rm(n, n->g);*/
		fill--;
	}

	void post_update(HeapElm *n) {
		printf("WARNING: post_update\n");
		assert(n->openind < 0);
		push(n);
	}

	bool isempty(void) {
		return fill == 0;
	}

// If the min value is equal or bigger than the incumbent,
// return false. This will be used for termination detection.
// Why minus 1? Because the total length contains the initial state.
// Path contains n nodes and n+1 edge. - 1
	bool isemptyunder(int incumbent) {
		if (fill == 0) {
			return true;
		} else {
			for (unsigned int i = 0; i < n_heaps; ++i) {
				if (heaps[i].minf() < incumbent - 1) {
					return false;
				}
			}
//			printf("TERMINATION\n");
			return true;
		}
	}

	bool mem(HeapElm *n) {
		return n->openind >= 0;
	}

	int minf() {
//		printf("minf\n");
		for (unsigned int i = 0; i < n_heaps; ++i) {

		}
	}

	int getsize() {
		return fill;
	}

	void clear(void) {
		printf("WARNING: post_update\n");
//		qs.clear();
		min = 0;
	}
};

#endif	// _HEAP_HPP_
