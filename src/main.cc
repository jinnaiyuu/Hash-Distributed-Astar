// Copyright 2012 Ethan Burns. All rights reserved.
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file.
#include "tiles.hpp"
#include "idastar.hpp"
#include "astar.hpp"
#include "pastar.hpp"
#include <cstring>

int main(int argc, const char *argv[]) {
	try {
		if (argc != 3)
			throw Fatal("Usage: tiles <algorithm> <problem number>");

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
		else if (strcmp(argv[1], "pastar") == 0)
			search = new Pastar<Tiles>(tiles);
		else
			throw Fatal("Unknown algorithm: %s", argv[1]);
	
		Tiles::State init = tiles.initial();


		dfheader(stdout);
		printf("#tiles	");
		for (int i = 0; i < 16; i++) {
			printf("%d ", init.tiles[i]);
		}
		printf("\n");

		dfpair(stdout, "problem number", "%02d", pnum);
//		printf("#pair problem number %02d \n", pnum);
		dfpair(stdout, "initial heuristic", "%d", tiles.h(init));
		double wall0 = walltime(), cpu0 = cputime();
	
		std::vector<Tiles::State> path = search->search(init);
	
		double wtime = walltime() - wall0, ctime = cputime() - cpu0;
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
