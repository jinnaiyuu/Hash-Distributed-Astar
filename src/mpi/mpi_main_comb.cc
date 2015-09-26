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

#include "../tiles.hpp"
//#include "../astar.hpp"
#include "hdastar_comb_mpi.hpp"

#include <mpi/mpi.h>

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
//	setvbuf(stdout, NULL, 0, _IONBF);
	try {
		fflush(stdout);
		// Markov primes.
		/*		printf("614657, 1336337, 4477457, 5308417, 8503057,"
		 "9834497, 29986577, 40960001, 45212177, 59969537, "
		 "65610001, 126247697, 193877777, 303595777, 384160001, "
		 "406586897, 562448657, 655360001");*/
		/*
		 * 288580489
		 if (!(3 <= argc && argc <= 7))
		 throw Fatal(
		 "Usage: tiles <domain (if 24)> <algorithm> <problem number> "
		 "<thread number> <income threshold> <outgo threshold> <abstraction>\n");
		 */
		int pnum = 0;

//		printf("pnum = %d\n", pnum);
		if (strcmp(argv[1], "15") == 0) {
//			printf("15 puzzle");
//			argv++;
//			argc--;
			/*
			 for (int i = 0; argv[2][i] != '\0'; ++i) {
			 pnum *= 10;
			 pnum += argv[2][i] - '0';
			 }
			 */
			argv++;
			argc--;

			FILE* instance;
			instance = fopen(argv[1], "r");

			argv++;
			argc--;
			sscanf(argv[2], "%d", &pnum);

			Tiles tiles(instance, pnum);
			printf("pnum = %d\n", pnum);

//			tiles.set_weight(2);
//			printf("weight = 2\n");
			// For multiastar.
			unsigned int n_threads = 0;

			SearchAlg<Tiles> *search = NULL;

			double estm_send_ratio = 0;
			double estm_dup_ratio = 0;
			double timer = 1.0;

			if (strcmp(argv[1], "astar") == 0) {
//				search = new Astar<Tiles>(tiles);
			} else if (strcmp(argv[1], "hdastar-mpi") == 0) {
//				MPI_Init(NULL, NULL);
				// Set the size of closed list.
				unsigned int closedlistsize = 0;
				for (unsigned int i = 0; i < argc; ++i) {
					if (sscanf(argv[i], "closed-%u", &closedlistsize) == 1) {
						break;
					}
				}
				if (!closedlistsize) {
					printf("set closedlistsize as closed-%%u\n");
					exit(1);
				}
				// Arguments of HDAstar
				// Domain, n_threads,
				// incomebuffermax, outgobuffermax,
				// abstraction, closed list size
				search = new HDAstarCombMPI<Tiles, Zobrist<Tiles, 16> >(tiles,
						std::stoi(argv[3]), 1000000, 1000000,
						std::stoi(argv[4]), 0, closedlistsize);
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
			/*
			 if (argc == 7) {
			 dfpair(stdout, "income buffer threshold", "%d",
			 std::stoi(argv[4]));
			 dfpair(stdout, "outgo buffer threshold", "%d",
			 std::stoi(argv[5]));
			 //				dfpair(stdout, "abstraction", "%d", std::stoi(argv[6]));
			 } else if (argc == 5) {
			 dfpair(stdout, "outsourcing f diff threshold", "%d",
			 std::stoi(argv[4]));
			 }
			 */
			dfpair(stdout, "initial heuristic", "%d", tiles.h(init));
			double wall0 = walltime(), cpu0 = cputime();
			std::vector<Tiles::State> path = search->search(init);
			double send_ratio = 1.0
					- (double) search->push / (double) search->gend;
			double dup_ratio = (double) search->dup / (double) search->expd;
//			double search->expd = (double) search->dup / (double) search->expd;

			printf("estimated_send_ratio: %f : %f sec\n", estm_send_ratio,
					timer);
			printf("estimated_dup_ratio: %f : %f sec\n", estm_dup_ratio, timer);
			printf("actual_send_ratio: %f\n", send_ratio);
			printf("actual_dup_ratio: %f\n", dup_ratio);
//			printf("actual_expd: %d\n", search->expd);

			double wtime = search->wtime - wall0, ctime = search->ctime - cpu0;
//			sleep(2);
			dfpair(stdout, "total wall time", "%g", wtime);
			dfpair(stdout, "total cpu time", "%g", ctime);
			dfpair(stdout, "total nodes expanded", "%lu", search->expd);
			dfpair(stdout, "total nodes generated", "%lu", search->gend);

			dfpair(stdout, "solution length", "%u", (unsigned int) path.size());
//			search->
			/*			assert(path.begin() != path.end());
			 for (auto iter = path.end() - 1; iter != path.begin() - 1; --iter) {
			 for (int i = 0; i < 16; ++i) {
			 printf("%2d ", iter->tiles[i]);
			 }
			 printf("\n");
			 }*/

			dffooter(stdout);

		}
	} catch (const Fatal &f) {
		fputs(f.msg, stderr);
		fputc('\n', stderr);
		return 1;
	}



	return 0;
}

