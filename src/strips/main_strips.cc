// Copyright 2012 Ethan Burns. All rights reserved.
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file.
#include <cstring>

#include <stdio.h>
#include <execinfo.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <iostream>
#include <fstream>

//#define DEBUG
#ifndef DEBUG
#define dbgprintf   1 ? (void) 0 : (void)
#else // #ifdef DEBUG
#define dbgprintf   printf
#endif // #ifdef DEBUG
//#define OUTSOURCING
//#define SEMISYNC

#define ANALYZE_INCOME
#define ANALYZE_OUTGO
#define ANALYZE_DUPLICATE
#define ANALYZE_DISTRIBUTION
//#define ANALYZE_FTRACE
#define ANALYZE_INCUMBENT_TRACE
//#define ANALYZE_GLOBALF
//#define ANALYZE_LAPSE
#ifdef OUTSOURCING
#define ANALYZE_OUTSOURCING
#endif
#ifdef SEMISYNC
#define ANALYZE_SEMISYNC
#endif

#include "../strips/strips.hpp"

#include "../astar.hpp"
#include "../hdastar.hpp"
#include "../strips/zobrist.hpp"

//
//#include "astar_heap.hpp"
//#include "hdastar_heap.hpp"

//	 expd 32334 length 46   : 14 1 9 6 4 8 12 5 7 2 3 0 10 11 13 15
//	 expd 909442 length 53  : 13 14 6 12 4 5 1 0 9 3 10 2 15 11 8 7
//	 expd 5253685 length 57 : 5 12 10 7 15 11 14 0 8 2 1 13 3 4 9 6
//	 expd 565,994,203       : 14 7 8 2 13 11 10 4 9 12 5 0 3 6 1 15

void handler(int sig) {
	void *array[10];
	size_t size;
	// get void*'s for all entries on the stack
	size = backtrace(array, 10);
	// print out all the frames to stderr
	fprintf(stderr, "Error: signal %d:\n", sig);
	backtrace_symbols_fd(array, size, STDERR_FILENO);
	exit(1);
}

int main(int argc, const char *argv[]) {

	try {
		fflush(stdout);
		std::ifstream domain(argv[2]);
		std::ifstream instance(argv[3]);

		Strips strips(domain, instance);
//		return 0;

		unsigned int h = 0;
		for (unsigned int i = 0; i < argc; ++i) {
			if (sscanf(argv[i], "h-%u", &h) == 1) {
				break;
			}
		}

		// if PDB heuristic
		if (h == 3) {
			unsigned int pdb = 0;
			for (unsigned int i = 0; i < argc; ++i) {
				if (sscanf(argv[i], "pdb-%u", &pdb) == 1) {
					break;
				}
			}
			strips.set_pdb(pdb);
		}

		strips.set_heuristic(h);


		SearchAlg<Strips> *search = NULL;
		unsigned int n_threads = 1;

		if (strcmp(argv[1], "astar") == 0) {
			unsigned int open = 100;
			double weight = 1.0;
			unsigned int incumbent = 100000;
			unsigned int closed = 4477457;
			search = new Astar<Strips>(strips, open, weight, incumbent, closed);
		} else if (sscanf(argv[1], "hdastar-%u", &n_threads) == 1) {
			unsigned int abst = 0;
			for (unsigned int i = 0; i < argc; ++i) {
				if (sscanf(argv[i], "abst-%u", &abst) == 1) {
					break;
				}
			}

			search = new HDAstar<Strips, StripsZobrist<Strips> >(strips,
					n_threads, 1000000, // income threshould
					10000000, // outgo threshould
					abst, // abstraction
					0, // overrun
					4477457, // closed list size
					100, // open list size
					10000000 // max cost
					);

		} else {
			std::cout << "no algorithm" << std::endl;
			assert(false);
		}

		Strips::State init = strips.initial();

		dfheader(stdout);
		dfpair(stdout, "algorithm", "%s", argv[1]);
		dfpair(stdout, "initial heuristic", "%d", strips.h(init));
		double wall0 = walltime(), cpu0 = cputime();
		std::vector<Strips::State> path = search->search(init);

		double wtime = search->wtime - wall0, ctime = search->ctime - cpu0;

		strips.print_plan(path);

		dfpair(stdout, "total wall time", "%g", wtime);
		dfpair(stdout, "total cpu time", "%g", ctime);
		dfpair(stdout, "total nodes expanded", "%lu", search->expd);
		dfpair(stdout, "total nodes generated", "%lu", search->gend);
		dfpair(stdout, "solution length", "%u", (unsigned int) path.size());

		dffooter(stdout);
	} catch (const Fatal &f) {
		fputs(f.msg, stderr);
		fputc('\n', stderr);
		return 1;
	}

	return 0;
}

