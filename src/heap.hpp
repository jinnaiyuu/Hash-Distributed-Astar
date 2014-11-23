// Copyright 2012 Ethan Burns. All rights reserved.
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file.
#ifndef _HEAP_HPP_
#define _HEAP_HPP_

#include "fatal.hpp"
#include <vector>
#include <limits>
#include <cassert>

template <class HeapElm> class Heap {

	struct Maxq {
		Maxq(void) : fill(0), max(0) { }

		void push(HeapElm *n, int p) {
			assert (p >= 0);
			if (bins.size() <= (unsigned int) p)
				bins.resize(p+1);

			if (p > max)
				max = p;

			// If you put p to 0, No h tie-breaking.
			n->openind = bins[p].size();
			bins[p].push_back(n);
			fill++;
		}

		HeapElm *pop(void) {
			for ( ; bins[max].empty(); max--) {
				if (max == 0) {
					printf("max==0\n");
					break;
				}
			}
			dbgprintf("g = %d\n", max);
			HeapElm *n = bins[max].back();
			bins[max].pop_back();
			n->openind = -1;
			fill--;
			return n;
		}

		void rm(HeapElm *n, unsigned long p) {
			assert (p < bins.size());
			std::vector<HeapElm*> &bin = bins[p];

			unsigned int i = n->openind;
			assert (i < bin.size());

			if (bin.size() > 1) {
				bin[i] = bin[bin.size() - 1];
				bin[i]->openind = i;
			}

			bin.pop_back();
			n->openind = -1;
			fill--;
		}

		bool empty(void) { return fill == 0; }

		int fill, max;
		std::vector< std::vector<HeapElm*> > bins;
	};

	int fill, min;
	std::vector<Maxq> qs;

public:
	Heap(unsigned int sz) : fill(0), min(0), qs(sz) { }

	static const char *kind(void) { return "2d bucketed"; }

	void push(HeapElm *n) {
		int p0 = n->f;
 
		// TODO: Need to halt if the number growing crazy.
		// TODO: ad hoc solution.
		if (p0 >= qs.size()) {
			printf("f going crazy.\n");
			return;
		}

		if (p0 < min)
			min = p0;

		qs[p0].push(n, n->g);
		fill++;
	}

	HeapElm *pop(void) {
		for ( ; (unsigned int) min < qs.size() && qs[min].empty() ; min++)
			;
		fill--;
		dbgprintf("f = %d\n", min);
		return qs[min].pop();		
	}

	void pre_update(HeapElm*n) {
		if (n->openind < 0)
			return;
		assert ((unsigned int) n->f < qs.size());
		qs[n->f].rm(n, n->g);
		fill--;
	}

	void post_update(HeapElm *n) {
		assert (n->openind < 0);
		push(n);
	}

	bool isempty(void) { return fill == 0; }

	// If the min value is equal or bigger than the incumbent,
	// return false. This will be used for termination detection.
	// Why minus 1? Because the total length contains the initial state.
	// Path contains n nodes and n+1 edge. - 1
	bool isemptyunder(int incumbent) { return (((incumbent - 1) <= min) || fill == 0);}

	bool mem(HeapElm *n) { return n->openind >= 0; }

	int minf() {return min;}

	void clear(void) {
		qs.clear();
		min = 0;
	}
};

#endif	// _HEAP_HPP_
