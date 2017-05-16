// Copyright 2012 Ethan Burns. All rights reserved.
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file.
#ifndef _HEAP_HPP_
#define _HEAP_HPP_

#ifndef DEBUG
#define dbgprintf   1 ? (void) 0 : (void)
#else // #ifdef DEBUG
#define dbgprintf   printf
#endif // #ifdef DEBUG

#include "fatal.hpp"
#include <vector>
#include <limits>
#include <cassert>
#include <deque>

template<class HeapElm> class Heap {

	struct Maxq {
		Maxq(bool isFIFO) :
				fill(0), max(0), isFIFO(isFIFO) {
		}

		void push(HeapElm *n, int p) {
			assert(p >= 0);
			if (bins.size() <= (unsigned int) p)
				bins.resize(p + 1);

			if (p > max)
				max = p;

			// If you put p to 0, No h tie-breaking.
			n->openind = bins[p].size();
			bins[p].push_back(n);
			fill++;
		}

		HeapElm *pop(void) {
			for (; bins[max].empty(); max--) {
				if (max == 0) {
					printf("max==0\n");
					break;
				}
			}
			dbgprintf( "g = %d\n", max);

			HeapElm *n ;
			if (!isFIFO) {
				n = bins[max].back();
				bins[max].pop_back();
			} else {
				n = bins[max].front();
				bins[max].pop_front();
			}
			n->openind = -1;
			fill--;
			return n;
		}

		void rm(HeapElm *n, unsigned long p) {

			assert(p < bins.size());
			std::deque<HeapElm*> &bin = bins[p];

			unsigned int i = n->openind;
			assert(i < bin.size());

			if (bin.size() > 1) {
				bin[i] = bin[bin.size() - 1];
				bin[i]->openind = i;
			}

			if (!isFIFO) {
				bin.pop_back();
			} else {
				bin.pop_front();
			}
			n->openind = -1;
			fill--;
		}

		bool empty(void) {
			return fill == 0;
		}

		int getsize() {
			int sum = 0;
			for (int i = 0; i < bins.size(); ++i) {
				sum += bins[i].size();
			}
			return sum;
		}

		int getmax() {
			return max;
		}

		int fill, max;
		bool isFIFO;
		std::vector<std::deque<HeapElm*> > bins;
	};

	int fill, min;
	std::vector<Maxq> qs;

	int overrun;

public:
	Heap(unsigned int sz, int overrun_ = 0, bool isFIFO_ = false) :
			fill(0), min(0), qs(sz, Maxq(isFIFO_)), overrun(overrun_) {
		printf("overrun=%d\n", overrun);
	}

	static const char *kind(void) {
		return "2d bucketed";
	}

	void push(HeapElm *n) {
		int p0 = n->f;

		// TODO: Need to halt if the number growing crazy.
		// TODO: ad hoc solution.
		if (p0 >= qs.size()) {
			dbgprintf( "f going crazy.\n");
			return;
		}

		if (p0 < min)
			min = p0;

		qs[p0].push(n, n->g);
		fill++;
	}

	HeapElm *pop(void) {
//		int mmin = min;
		for (; (unsigned int) min < qs.size() && qs[min].empty(); min++) {
			;
		}
//		if (mmin != min) {
//			printf("min = %d\n", min);
//		}
		fill--;
		dbgprintf( "f = %d\n", min);
		return qs[min].pop();
	}

	void pre_update(HeapElm*n) {
		if (n->openind < 0)
			return;
		assert((unsigned int) n->f < qs.size());
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

	// If the min value is equal or bigger than the incumbent,
	// return false. This will be used for termination detection.
	// Why minus 1? Because the total length contains the initial state.
	// Path contains n nodes and n+1 edge. - 1
	bool isemptyunder(int incumbent) {
		return (((incumbent - 1 + overrun) <= min) || fill == 0);
	}

	bool mem(HeapElm *n) {
		return n->openind >= 0;
	}

	int minf() {
		return min;
	}

	int getpriority() {
		return min * 100000 - qs[min].getmax();
	}

	int getsize() {
//		int sum = 0;
//		dbgprintf("qs.size = %zu\n", qs.size() );
//		for (int i = 0; i < qs.size(); ++i) {
//			sum += qs[i].getsize();
//		}
//		return sum;
		return fill;
	}

	void clear(void) {
		qs.clear();
		min = 0;
	}

};

#endif	// _HEAP_HPP_
