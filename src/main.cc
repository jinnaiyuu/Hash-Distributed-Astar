// Copyright 2012 Ethan Burns. All rights reserved.
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file.
#include <cstring>

#include <stdio.h>
#include <execinfo.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

//#define DEBUG
#ifndef DEBUG
#define dbgprintf   1 ? (void) 0 : (void)
#else // #ifdef DEBUG
#define dbgprintf   printf
#endif // #ifdef DEBUG

#define OUTSOURCING
#define SEMISYNC

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

//#define ANALYZE_ORDER

//#define ANALYZE_OPENLIST_SIZE

#include "tiles.hpp"
#include "idastar.hpp"
#include "astar.hpp"
#include "pastar.hpp"
#include "hdastar.hpp"
#include "oshdastar.hpp"

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
		if (!(3 <= argc && argc <= 7))
			throw Fatal(
					"Usage: tiles <algorithm> <problem number> "
					"<thread number> <income threshold> <outgo threshold> <abstraction>\n");

		int pnum = 0;
		for (int i = 0; argv[2][i] != '\0'; ++i) {
			pnum *= 10;
			pnum += argv[2][i] - '0';
		}
//		printf("pnum = %d\n", pnum);
		Tiles tiles(stdin, pnum);

		SearchAlg<Tiles> *search = NULL;
		if (strcmp(argv[1], "idastar") == 0)
			search = new Idastar<Tiles>(tiles);
		else if (strcmp(argv[1], "astar") == 0)
			search = new Astar<Tiles>(tiles);
		else if (strcmp(argv[1], "pastar") == 0) {
			if (argc == 4) {
				search = new Pastar<Tiles>(tiles, std::stoi(argv[3]));
			} else {
				search = new Pastar<Tiles>(tiles);
			}
		} else if (strcmp(argv[1], "hdastar") == 0) {
			if (argc >= 5) {
				search = new HDAstar<Tiles, Zobrist<16> >(tiles, std::stoi(argv[3]), std::stoi(argv[4]),
						std::stoi(argv[5]), std::stoi(argv[6]));
			} else {
				search = new HDAstar<Tiles, Zobrist<16> >(tiles, std::stoi(argv[3])); // Completely Asynchronous
			}
		} else if (strcmp(argv[1], "hdastar_trivial") == 0) {
			if (argc >= 5) {
				search = new HDAstar<Tiles, TrivialHash<16> >(tiles, std::stoi(argv[3]), std::stoi(argv[4]), std::stoi(argv[5]));
			} else {
				search = new HDAstar<Tiles, TrivialHash<16> >(tiles, std::stoi(argv[3])); // Completely Asynchronous
			}
		} else if (strcmp(argv[1], "hdastar_random") == 0) {
			if (argc >= 5) {
				search = new HDAstar<Tiles, RandomHash<16> >(tiles, std::stoi(argv[3]), std::stoi(argv[4]), std::stoi(argv[5]));
			} else {
				search = new HDAstar<Tiles, RandomHash<16> >(tiles, std::stoi(argv[3])); // Completely Asynchronous
			}
		} else if (strcmp(argv[1], "oshdastar") == 0) {
			if (argc == 6) {
				search = new OSHDAstar<Tiles, Zobrist<16> >(tiles, std::stoi(argv[3]), std::stoi(argv[4]), std::stoi(argv[5]));
			} else {
				search = new OSHDAstar<Tiles, Zobrist<16> >(tiles, std::stoi(argv[3]));
			}
		} else
			throw Fatal("Unknown algorithm: %s", argv[1]);

		Tiles::State init = tiles.initial();

		dfheader(stdout);
		dfpair(stdout, "algorithm", "%s", argv[1]);
		printf("#tiles	");
		for (int i = 0; i < 16; i++) {
			printf("%d ", init.tiles[i]);
		}
		printf("\n");

		dfpair(stdout, "problem number", "%02d", pnum);
		if (argc > 3 && strcmp(argv[3], "")) {
			dfpair(stdout, "thread number", "%02d", std::stoi(argv[3]));
		}
		if (argc == 7) {
			dfpair(stdout, "income buffer threshold", "%d", std::stoi(argv[4]));
			dfpair(stdout, "outgo buffer threshold", "%d", std::stoi(argv[5]));
			dfpair(stdout, "abstraction", "%d", std::stoi(argv[6]));
		} else if (argc == 5){
			dfpair(stdout, "outsourcing f diff threshold", "%d", std::stoi(argv[4]));
		}
		dfpair(stdout, "initial heuristic", "%d", tiles.h(init));
		double wall0 = walltime(), cpu0 = cputime();
		std::vector<Tiles::State> path = search->search(init);

		double wtime = search->wtime - wall0, ctime = search->ctime - cpu0;
		sleep(2);
		dfpair(stdout, "total wall time", "%g", wtime);
		dfpair(stdout, "total cpu time", "%g", ctime);
		dfpair(stdout, "total nodes expanded", "%lu", search->expd);
		dfpair(stdout, "total nodes generated", "%lu", search->gend);
		dfpair(stdout, "solution length", "%u", (unsigned int) path.size());

		assert(path.begin() != path.end());
		for (auto iter = path.end() - 1; iter != path.begin() - 1; --iter) {
			for (int i = 0; i < 16; ++i) {
				printf("%2d ", iter->tiles[i]);
			}
			printf("\n");
		}

		dffooter(stdout);
	} catch (const Fatal &f) {
		fputs(f.msg, stderr);
		fputc('\n', stderr);
		return 1;
	}

	return 0;
}

