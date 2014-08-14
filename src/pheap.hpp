/*
 * pheap.hpp
 *
 *  Created on: Jun 19, 2014
 *      Author: yuu
 */

#ifndef PHEAP_HPP_
#define PHEAP_HPP_

// Copyright 2012 Ethan Burns. All rights reserved.
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file.

#include "fatal.hpp"
#include <vector>
#include <limits>
#include <cassert>

//#include <boost/lockfree/queue.hpp>
#include "pqueue.hpp"
//#define DEBUG
#ifdef DEBUG
#define printl()
//#define printl() printf("%d:\n", __LINE__)
#else
#define printl()
#endif

template<class HeapElm> class PHeap {

	struct Maxq {
		Maxq(void) :
				fill(0), max(0) {
//			bins.resize(1);
//			bins[0] = new boost::lockfree::queue<HeapElm*>(1);
		}

		// p corresponds to g value for Astar.
		void push(HeapElm *n, int p) {
			assert(p >= 0);
//			printf("bins.size() = %u\n", bins.size());
			if (bins.size() <= (unsigned int) p) {
				bins.resize(p + 1);
			}
			if (p > max)
				max = p;

			// TODO: AD HOC
//			n->openind = bins[p].size();
			n->openind = bins[p].size();
//			printf("%d: p = %d\n", __LINE__, p);

			bins[p].push(n);

			fill++;
			printl();
		}

		HeapElm *pop(void) {
			for (; bins[max].empty(); max--) {
				if (max == 0) {
					break;
				}
			}
			// TODO: The element needs to be atomically pop

			HeapElm *n;
#ifdef DEBUG
			printf("pop : g = %d\n", max);
#endif
			bins[max].wait_and_pop(n);
//			bins[max]->pop(n);
			// NOT SURE
//			while (n == NULL) {
//				bins[max]->pop(n);
//			}
			n->openind = -1;
			fill--;

			printl();
			return n;
		}

		void rm(HeapElm *n, unsigned long p) {
			assert(p < bins.size());

			std::vector<HeapElm*> &bin = bins[p];

			unsigned int i = n->openind;
			assert(i < bin.size());

			if (bin.size() > 1) {
				bin[i] = bin[bin.size() - 1];
				bin[i]->openind = i;
			}

			bin.pop_back();
			n->openind = -1;
			fill--;
		}

		bool empty() {
			return fill == 0;
		}

		int fill, max;
		std::vector<p_queue<HeapElm*>> bins;
	};

	int fill, min;
	std::vector<Maxq> qs;

public:
	PHeap(unsigned int sz) :
			fill(0), min(0), qs(sz) {
	}

	static const char *kind(void) {
		return "2d bucketed";
	}

	void push(HeapElm *n) {
		int p0 = n->f;

		assert(p0 < qs.size());

		if (p0 < min)
			min = p0;
#ifdef DEBUG
		printf("				Push f = %d, g = %d\n", p0, n->g);
#endif
		qs[p0].push(n, n->g);
		fill++;
	}

	HeapElm *pop(void) {
		while(isempty()) {
#ifdef DEBUG
			printf("empty\n");
#endif
		}
		for (; (unsigned int) min < qs.size() && qs[min].empty(); min++)
			;
		while (min >= 100) {
			printf("lol");
			min = 0;
			for (; (unsigned int) min < qs.size() && qs[min].empty(); min++)
				;
		}
		fill--;
		// TODO: need to check if qs[min] is empty or not
#ifdef DEBUG
		printf("Pop: f = %d, ", min);
#endif
		HeapElm* e = qs[min].pop();
		while (e == NULL) {
			for (; (unsigned int) min < qs.size() && qs[min].empty(); min++)
				;
			e = qs[min].pop();
		}
		return e;
	}

	void pre_update(HeapElm*n) {
		if (n->openind < 0)
			return;
		assert((unsigned int ) n->f < qs.size());
		qs[n->f].rm(n, n->g);
		fill--;
	}

	void post_update(HeapElm *n) {
		assert(n->openind < 0);
		push(n);
	}

	bool isempty(void) {
		return fill == 0;
	}

	bool mem(HeapElm *n) {
		return n->openind >= 0;
	}

	void clear(void) {
		qs.clear();
		min = 0;
	}
};

#endif /* PHEAP_HPP_ */
