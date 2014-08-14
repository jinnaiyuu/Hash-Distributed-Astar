// Copyright 2012 Ethan Burns. All rights reserved.
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file.
#include "tiles.hpp"
#include "idastar.hpp"
#include "astar.hpp"
#include "pastar.hpp"
#include "hdastar.hpp"
#include <cstring>

#include <stdio.h>
#include <execinfo.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

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
		if (argc != 3 && argc != 4)
			throw Fatal(
					"Usage: tiles <algorithm> <problem number> or \n"
							"tiles <parallel algorithm> <problem number> <thread number>");

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
			if (argc == 4) {
				search = new HDAstar<Tiles>(tiles, std::stoi(argv[3]));
			} else {
				search = new HDAstar<Tiles>(tiles);
			}
		} else
			throw Fatal("Unknown algorithm: %s", argv[1]);

		Tiles::State init = tiles.initial();

		dfheader(stdout);
		printf("#tiles	");
		for (int i = 0; i < 16; i++) {
			printf("%d ", init.tiles[i]);
		}
		printf("\n");

		dfpair(stdout, "problem number", "%02d", pnum);
		if (argv[3] == "") {
			dfpair(stdout, "thread number", "%02d", std::stoi(argv[3]));
		}
		dfpair(stdout, "initial heuristic", "%d", tiles.h(init));
		double wall0 = walltime(), cpu0 = cputime();

		std::vector<Tiles::State> path = search->search(init);

		double wtime = walltime() - wall0, ctime = cputime() - cpu0;
		dfpair(stdout, "total wall time", "%g", wtime);
		dfpair(stdout, "total cpu time", "%g", ctime);
		dfpair(stdout, "total nodes expanded", "%lu", search->expd);
		dfpair(stdout, "total nodes generated", "%lu", search->gend);
		dfpair(stdout, "solution length", "%u", (unsigned int) path.size());

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

