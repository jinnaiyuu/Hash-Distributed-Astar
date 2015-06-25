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

//#define ANALYZE_ORDER

//#define ANALYZE_OPENLIST_SIZE

#include "../tiles.hpp"
#include "../tiles24.hpp"
#include "../grid.hpp"
#include "../tsp.hpp"
#include "../msa/msa.hpp"

#include "../grid_hash.hpp"
#include "../tsp_hash.hpp"
#include "../msa/zobrist.hpp"
#include "../msa/hyperplane.hpp"

#include "../idastar.hpp"
#include "../astar.hpp"
#include "../pastar.hpp"
#include "../multiastar.hpp"
#include "../hdastar_comb.hpp"
#include "../oshdastar.hpp"

#include "../astar_heap.hpp"
#include "../hdastar_heap.hpp"

#include "../hdastar_shared_closed.hpp"
#include "../ppastar.hpp"

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
			sscanf(argv[2], "%d", &pnum);

			Tiles tiles(stdin, pnum);
			printf("pnum = %d\n", pnum);

//			tiles.set_weight(2);
//			printf("weight = 2\n");
			// For multiastar.
			unsigned int n_threads = 0;

			SearchAlg<Tiles> *search = NULL;

			double estm_send_ratio = 0;
			double estm_dup_ratio = 0;
			double timer = 1.0;

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
			} else if (sscanf(argv[1], "multiastar-%u", &n_threads) == 1) {
				search = new MultiAstar<Tiles>(tiles, n_threads);
			} else if (strcmp(argv[1], "hdastar") == 0) {
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
				search = new HDAstarComb<Tiles, Zobrist<Tiles, 16> >(tiles,
						std::stoi(argv[3]), 1000000, 1000000,
						std::stoi(argv[4]), 0, closedlistsize);

			} else if (strcmp(argv[1], "hdastar-est") == 0) {
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

				////////////////////////////////
				/// Auto-Selection
				////////////////////////////////
				HDAstarComb<Tiles, Zobrist<Tiles, 16> > *subsearch =
						new HDAstarComb<Tiles, Zobrist<Tiles, 16> >(tiles,
								std::stoi(argv[3]), 1000000, 1000000,
								std::stoi(argv[4]));

				Tiles::State g = tiles.initial();
				subsearch->setTimer(timer);
				subsearch->search(g);
				// TODO: those gets are not so optimal.
				//       these operations are included in system execution time.
				std::vector<unsigned int> gends = subsearch->getGenerations();
				std::vector<unsigned int> pushes = subsearch->getSelfPushes();
				std::vector<unsigned int> expds = subsearch->getExpansions();
				std::vector<unsigned int> dups = subsearch->getDuplicates();
				int tnum = std::stoi(argv[3]);
				unsigned int gend = 0;
				unsigned int push = 0;
				unsigned int expd = 0;
				unsigned int dup = 0;
				for (int i = 0; i < tnum; ++i) {
					gend += gends[i];
					push += pushes[i];
					expd += expds[i];
					dup += dups[i];
				}


				estm_send_ratio = 1.0 - (double) push / (double) gend;
				estm_dup_ratio = (double) dup / (double) expd;
				delete subsearch;

				search = new HDAstarComb<Tiles, Zobrist<Tiles, 16> >(tiles,
						std::stoi(argv[3]), 1000000, 1000000,
						std::stoi(argv[4]), 0, closedlistsize);
//				search->setTimer(timer);
				// HDA* with shared closed list
//			} else if (strcmp(argv[1], "hdastar-shared") == 0) {
//				// Set the size of closed list.
//				unsigned int closedlistsize = 0;
//				unsigned int division = 0;
//				for (unsigned int i = 0; i < argc; ++i) {
//					if (sscanf(argv[i], "closed-%u-%u", &closedlistsize,
//							&division) == 2) {
//						break;
//					}
//				}
//				if (!closedlistsize || !division) {
//					printf("set closedlistsize as closed-%%u-%%u\n");
//					exit(1);
//				}
//				// Arguments of HDAstarComb
//				// Domain, n_threads,
//				// incomebuffermax, outgobuffermax,
//				// abstraction, closed list size
//				search = new HDAstarSharedClosed<Tiles, Zobrist<Tiles, 16> >(
//						tiles, std::stoi(argv[3]), 1000000, 1000000,
//						std::stoi(argv[4]), 0, closedlistsize, division);
//
//			} else if (strcmp(argv[1], "ppastar") == 0) {
//				// Set the size of closed list.
//				unsigned int openlistsize = 0;
//				unsigned int openlistdivision = 0;
//				unsigned int open_synchronous_push = true;
//				unsigned int open_synchronous_pop = -1;
//				unsigned int min_expd = 100;
//				unsigned int closedlistsize = 0;
//				unsigned int closedlistdivision = 0;
//				unsigned int hash_method = 0;
//
//				for (unsigned int i = 0; i < argc; ++i) {
////					if (sscanf(argv[i], "open-%u-%u", &openlistsize, &openlistdivision) == 2) {}
//					if (sscanf(argv[i], "open-%u-%u-%u-%u-%u", &openlistsize,
//							&openlistdivision, &open_synchronous_push,
//							&open_synchronous_pop, &min_expd) == 5) {
//					}
//					if (sscanf(argv[i], "closed-%u-%u", &closedlistsize,
//							&closedlistdivision) == 2) {
//					}
//					if (sscanf(argv[i], "hash-%u", &hash_method) == 1) {
//					}
//				}
//
//				if (!(openlistsize && openlistdivision && closedlistsize
//						&& closedlistdivision)) {
//					printf("set openlistsize as open-%%u-%%u\n");
//					printf("set closedlistsize as closed-%%u-%%u\n");
//					exit(1);
//				}
//				printf("syncpop = %u\n", open_synchronous_pop);
//				// Arguments of HDAstar
//				// Domain, n_threads,
//				// incomebuffermax, outgobuffermax,
//				// abstraction, closed list size
//				if (hash_method == 0) { // Zobrist
//					search = new PPAstar<Tiles, Zobrist<Tiles, 16> >(tiles,
//							std::stoi(argv[3]), 1000000, 1000000,
//							std::stoi(argv[4]), 0, openlistsize,
//							openlistdivision, open_synchronous_push,
//							open_synchronous_pop, min_expd, closedlistsize,
//							closedlistdivision);
//				} else {
//					search = new PPAstar<Tiles, TrivialHash<Tiles, 16> >(tiles,
//							std::stoi(argv[3]), 1000000, 1000000, 0, 0,
//							openlistsize, openlistdivision,
//							open_synchronous_push, open_synchronous_pop,
//							min_expd, closedlistsize, closedlistdivision);
//				}
//
//			} else if (strcmp(argv[1], "hdastar_trivial") == 0) {
//				if (argc >= 5) {
//					search = new HDAstarComb<Tiles, TrivialHash<Tiles, 16> >(tiles,
//							std::stoi(argv[3]), std::stoi(argv[4]),
//							std::stoi(argv[5]));
//				} else {
//					search = new HDAstarComb<Tiles, TrivialHash<Tiles, 16> >(tiles,
//							std::stoi(argv[3])); // Completely Asynchronous
//				}
//			} else if (strcmp(argv[1], "hdastar_random") == 0) {
//				if (argc >= 5) {
//					search = new HDAstarComb<Tiles, RandomHash<Tiles> >(tiles,
//							std::stoi(argv[3]), std::stoi(argv[4]),
//							std::stoi(argv[5]));
//				} else {
//					search = new HDAstarComb<Tiles, RandomHash<Tiles> >(tiles,
//							std::stoi(argv[3])); // Completely Asynchronous
//				}
//			} else if (strcmp(argv[1], "hdastar_overrun") == 0) {
//				if (argc >= 5) {
//					search = new HDAstarComb<Tiles, Zobrist<Tiles, 16> >(tiles,
//							std::stoi(argv[3]), std::stoi(argv[4]),
//							std::stoi(argv[5]), std::stoi(argv[6]), 1);
//				} else {
//					search = new HDAstarComb<Tiles, Zobrist<Tiles, 16> >(tiles,
//							std::stoi(argv[3])); // Completely Asynchronous
//				}
//			} else if (strcmp(argv[1], "oshdastar") == 0) {
//				if (argc == 6) {
//					search = new OSHDAstar<Tiles, Zobrist<Tiles, 16> >(tiles,
//							std::stoi(argv[3]), std::stoi(argv[4]),
//							std::stoi(argv[5]));
//				} else {
//					search = new OSHDAstar<Tiles, Zobrist<Tiles, 16> >(tiles,
//							std::stoi(argv[3]));
//				}
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

		} else if (strcmp(argv[1], "24") == 0) { // 24 puzzle
//#ifdef PUZZLE
			printf("24 Puzzle\n");
			argv++;
			argc--;
			// 15 Puzzle:  14 1 9 6 4 8 12 5 7 2 3 0 10 11 13 15
			// 24 Puzzle:
			// 1 2 0 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25
			sscanf(argv[2], "%d", &pnum);
//			printf("pnum = %d\n", pnum);
			dfpair(stdout, "problem number", "%02d", pnum);
			Tiles24 tiles(stdin, pnum);
			SearchAlg<Tiles24> *search = NULL;

			double timer = 1.0;
			double estm_send_ratio, estm_dup_ratio = 0.0;

			if (strcmp(argv[1], "astar") == 0) {
				search = new Astar<Tiles24>(tiles);
			} else if (strcmp(argv[1], "hdastar-est") == 0) {
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
				// abstraction, closed list siz

				////////////////////////////////
				/// Auto-Selection
				////////////////////////////////
				HDAstarComb<Tiles24, Zobrist<Tiles24, 25>> *subsearch =
						new HDAstarComb<Tiles24, Zobrist<Tiles24, 25>>(tiles,
								std::stoi(argv[3]), 1000000, 1000000,
								std::stoi(argv[4]), 0, 4477457);

				Tiles24::State g = tiles.initial();
				subsearch->setTimer(timer);
				subsearch->search(g);
				// TODO: those gets are not so optimal.
				//       these operations are included in system execution time.
				// TODO: load balance would be a better estimation. try it.
				std::vector<unsigned int> gends = subsearch->getGenerations();
				std::vector<unsigned int> pushes = subsearch->getSelfPushes();
				std::vector<unsigned int> expds = subsearch->getExpansions();
				std::vector<unsigned int> dups = subsearch->getDuplicates();
				int tnum = std::stoi(argv[3]);
				unsigned int gend = 0;
				unsigned int push = 0;
				unsigned int expd = 0;
				unsigned int dup = 0;
				for (int i = 0; i < tnum; ++i) {
					gend += gends[i];
					push += pushes[i];
					expd += expds[i];
					dup += dups[i];
				}

				estm_send_ratio = 1.0 - (double) push / (double) gend;
				estm_dup_ratio = (double) dup / (double) expd;
				delete subsearch;

				search = new HDAstarComb<Tiles24, Zobrist<Tiles24, 25> >(tiles,
						std::stoi(argv[3]), 1000000, 1000000,
						std::stoi(argv[4]), 0, closedlistsize);
//				search->setTimer(timer);

			} else if (strcmp(argv[1], "hdastar") == 0) {
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
				// abstraction, overrun, closed list size
				search = new HDAstarComb<Tiles24, Zobrist<Tiles24, 25> >(tiles,
						std::stoi(argv[3]), 1000000, 1000000,
						std::stoi(argv[4]), 0, closedlistsize);

			} else if (strcmp(argv[1], "hdastar_overrun") == 0) {
				search = new HDAstarComb<Tiles24, Zobrist<Tiles24, 25> >(tiles,
						std::stoi(argv[3]), 1000000, 1000000, 0,
						std::stoi(argv[4]));
			} else {
				throw Fatal("Unknown algorithm: %s", argv[1]);
			}

			Tiles24::State init = tiles.initial();
			dfheader(stdout);
			dfpair(stdout, "algorithm", "%s", argv[1]);
			printf("#tiles	");
			for (int i = 0; i < 25; i++) {
				printf("%d ", init.tiles[i]);
			}
			printf("\n");

			dfpair(stdout, "initial heuristic", "%d", tiles.h(init));
			double wall0 = walltime(), cpu0 = cputime();
			// HERE!
			std::vector<Tiles24::State> path = search->search(init);

			///////////////////////////
			/// auto selection
			///////////////////////////
			double send_ratio = 1.0
					- (double) search->push / (double) search->gend;
			double dup_ratio = (double) search->dup / (double) search->expd;
			printf("estimated_send_ratio: %f : %f sec\n", estm_send_ratio,
					timer);
			printf("estimated_dup_ratio: %f : %f sec\n", estm_dup_ratio, timer);
			printf("actual_send_ratio: %f\n", send_ratio);
			printf("actual_dup_ratio: %f\n", dup_ratio);
//			printf("actual_expd: %d\n", search->expd);

			double wtime = walltime() - wall0, ctime = cputime() - cpu0;

			dfpair(stdout, "total wall time", "%g", wtime);
			dfpair(stdout, "total cpu time", "%g", ctime);
			dfpair(stdout, "total nodes expanded", "%lu", search->expd);
			dfpair(stdout, "total nodes generated", "%lu", search->gend);
			dfpair(stdout, "solution length", "%u", (unsigned int) path.size());
//#endif
		} else if (strcmp(argv[1], "grid") == 0) {
			printf("Grid\n");
			argv++;
			argc--;

			// Here pnum is the scaling parameter. If two then make the instances size twice as big.

			unsigned int scale = 1;
			sscanf(argv[2], "%d", &scale);

			Grid grid(std::cin, scale);
			SearchAlg<Grid> *search = NULL;
			unsigned int max_f = (grid.get_height() + grid.get_width()) * scale;

			double timer = 1.0;
			double estm_send_ratio, estm_dup_ratio = 0.0;

			if (strcmp(argv[1], "astar") == 0) {
				search = new Astar<Grid>(grid, max_f);
			} else if (strcmp(argv[1], "hdastar") == 0) {

				unsigned int closedlistsize = 59969537;
				unsigned int abstraction = 1;
				for (unsigned int i = 0; i < argc; ++i) {
					if (sscanf(argv[i], "closed-%u", &closedlistsize) == 1) {
					}
					if (sscanf(argv[i], "abstraction-%u", &abstraction) == 1) {
					}

				}
				if (!closedlistsize) {
					printf("set closedlistsize as closed-%%u\n");
					exit(1);
				}
				printf("abst = %u\n", abstraction);

				search = new HDAstarComb<Grid, GridHash<Grid>>(grid,
						std::stoi(argv[3]), 1000000, 1000000, abstraction, 0,
						closedlistsize, max_f);

			} else if (strcmp(argv[1], "hdastar-est") == 0) {

				unsigned int closedlistsize = 59969537;
				unsigned int abstraction = 1;
				for (unsigned int i = 0; i < argc; ++i) {
					if (sscanf(argv[i], "closed-%u", &closedlistsize) == 1) {
					}
					if (sscanf(argv[i], "abstraction-%u", &abstraction) == 1) {
					}

				}
				if (!closedlistsize) {
					printf("set closedlistsize as closed-%%u\n");
					exit(1);
				}
				printf("abst = %u\n", abstraction);


				////////////////////////////////
				/// Auto-Selection
				////////////////////////////////
				HDAstarComb<Grid, GridHash<Grid>> *subsearch =
						new HDAstarComb<Grid, GridHash<Grid>>(grid,
								std::stoi(argv[3]), 1000000, 1000000,
								abstraction, 0, 59969537, max_f);

				Grid::State g = grid.initial();
				subsearch->setTimer(timer);
				subsearch->search(g);
				// TODO: those gets are not so optimal.
				//       these operations are included in system execution time.
				std::vector<unsigned int> gends = subsearch->getGenerations();
				std::vector<unsigned int> pushes = subsearch->getSelfPushes();
				std::vector<unsigned int> expds = subsearch->getExpansions();
				std::vector<unsigned int> dups = subsearch->getDuplicates();
				int tnum = std::stoi(argv[3]);
				unsigned int gend = 0;
				unsigned int push = 0;
				unsigned int expd = 0;
				unsigned int dup = 0;
				for (int i = 0; i < tnum; ++i) {
					gend += gends[i];
					push += pushes[i];
					expd += expds[i];
					dup += dups[i];
				}


				estm_send_ratio = 1.0 - (double) push / (double) gend;
				estm_dup_ratio = (double) dup / (double) expd;
				delete subsearch;


				search = new HDAstarComb<Grid, GridHash<Grid>>(grid,
						std::stoi(argv[3]), 1000000, 1000000, abstraction, 0,
						closedlistsize, max_f);

			} else {
				printf("command line input parse error\n");
				exit(1);
			}
			Grid::State init = grid.initial();
			dfheader(stdout);
			dfpair(stdout, "algorithm", "%s", argv[1]);
			dfpair(stdout, "initial heuristic", "%d", grid.h(init));
			double wall0 = walltime(), cpu0 = cputime();
			// HERE!
			std::vector<Grid::State> path = search->search(init);

			double wtime = walltime() - wall0, ctime = cputime() - cpu0;


			///////////////////////////
			/// auto selection
			///////////////////////////
			double send_ratio = 1.0
					- (double) search->push / (double) search->gend;
			double dup_ratio = (double) search->dup / (double) search->expd;
			printf("estimated_send_ratio: %f : %f sec\n", estm_send_ratio,
					timer);
			printf("estimated_dup_ratio: %f : %f sec\n", estm_dup_ratio, timer);
			printf("actual_send_ratio: %f\n", send_ratio);
			printf("actual_dup_ratio: %f\n", dup_ratio);

			dfpair(stdout, "total wall time", "%g", wtime);
			dfpair(stdout, "total cpu time", "%g", ctime);
			dfpair(stdout, "total nodes expanded", "%lu", search->expd);
			dfpair(stdout, "total nodes generated", "%lu", search->gend);
			dfpair(stdout, "solution length", "%u", (unsigned int) path.size());

		} else if (strcmp(argv[1], "tsp") == 0) {
			printf("Tsp\n");
			argv++;
			argc--;
			// Here pnum is the scaling parameter. If two then make the instances size twice as big.

			Tsp tsp(std::cin);

			unsigned int heuristic = 0;
			sscanf(argv[2], "%d", &heuristic);
			tsp.set_heuristic(heuristic);

			SearchAlg<Tsp> *search = NULL;

			if (strcmp(argv[1], "astar") == 0) {
				search = new AstarHeap<Tsp>(tsp, 1500000);
			} else if (strcmp(argv[1], "hdastar") == 0) {
				search = new HDAstarHeap<Tsp, TspHash<Tsp> >(tsp,
						std::stoi(argv[3]), 1000000, 1000000,
						std::stoi(argv[4]), 0, 193877777, 1500000);
			}

			Tsp::State init = tsp.initial();
			dfheader(stdout);
			dfpair(stdout, "algorithm", "%s", argv[1]);
			dfpair(stdout, "initial heuristic", "%d", tsp.h(init));
			double wall0 = walltime(), cpu0 = cputime();
			// HERE!
			std::vector<Tsp::State> path = search->search(init);

//			double wtime = walltime() - wall0, ctime = cputime() - cpu0;
			double wtime = search->wtime - wall0, ctime = search->ctime - cpu0;

			dfpair(stdout, "total wall time", "%g", wtime);
			dfpair(stdout, "total cpu time", "%g", ctime);
			dfpair(stdout, "total nodes expanded", "%lu", search->expd);
			dfpair(stdout, "total nodes generated", "%lu", search->gend);
			dfpair(stdout, "solution length", "%u", (unsigned int) path.size());

		} else if (strcmp(argv[1], "msa") == 0) {
			printf("MSA\n");
			argv++;
			argc--;
			// Here pnum is the scaling parameter. If two then make the instances size twice as big.

			std::ifstream pam("PAM250");
//			pam.open("PAM250");

			MSA msa(pam, std::cin);
			pam.close();

//			unsigned int heuristic = 0;
//			sscanf(argv[2], "%d", &heuristic);
//			msa.set_heuristic(heuristic);

			SearchAlg<MSA> *search = NULL;

			MSA::State init = msa.initial();

			unsigned int max_h = init.h * 1.5;

			printf("max_h = %u\n", max_h);


			double timer = 1.0;
			double estm_send_ratio, estm_dup_ratio = 0.0;


			if (strcmp(argv[1], "astar") == 0) {
				search = new AstarHeap<MSA>(msa, max_h);
			} else if (strcmp(argv[1], "wastar_heap") == 0) {
				double weight = 1;
				for (unsigned int i = 0; i < argc; ++i) {
					if (sscanf(argv[i], "w-%lf", &weight) == 1) {
					}
				}
				printf("w = %f\n", weight);
				search = new AstarHeap<MSA>(msa, max_h * weight, weight);
			} else if (strcmp(argv[1], "wastar") == 0) {
				double weight = 1;
				for (unsigned int i = 0; i < argc; ++i) {
					if (sscanf(argv[i], "w-%lf", &weight) == 1) {
					}
				}
				printf("w = %f\n", weight);
				search = new AstarHeap<MSA>(msa, max_h * weight, weight);
			} else if (strcmp(argv[1], "wa+astar") == 0) {
				double weight = 1;
				for (unsigned int i = 0; i < argc; ++i) {
					if (sscanf(argv[i], "w-%lf", &weight) == 1) {
					}
				}
				search = new AstarHeap<MSA>(msa, max_h * weight, weight);
				MSA::State init = msa.initial();
				double wall0 = walltime();
				std::vector<MSA::State> path = search->search(init);
				double wtime = search->wtime - wall0;
				unsigned int cost = search->incm;
				printf("precalc cost = %u\n", cost);
				dfpair(stdout, "precalc wall time", "%g", wtime);
				dfpair(stdout, "precalc nodes expanded", "%lu", search->expd);
				dfpair(stdout, "precalc nodes generated", "%lu", search->gend);
				delete search;
				search = new AstarHeap<MSA>(msa, max_h, 1.0, cost);

			} else if (strcmp(argv[1], "hdastar") == 0) {

				unsigned int closedlistsize = 9834497;
//				unsigned int closedlistsize = 45212177;
				for (unsigned int i = 0; i < argc; ++i) {
					if (sscanf(argv[i], "closed-%u", &closedlistsize) == 1) {
					}
				}

				search = new HDAstarHeap<MSA, MSAZobrist<MSA> >(msa,
						std::stoi(argv[2]), 1000000, 1000000,
						std::stoi(argv[3]), 0, closedlistsize, max_h, 1000000);

//				search = new HDAstar<MSA, MSAZobrist<MSA> >(msa,
//						std::stoi(argv[2]), 1000000, 1000000,
//						std::stoi(argv[3]), 0, closedlistsize, max_h, 1000000);

			} else if (strcmp(argv[1], "hdastar-est") == 0) {

				unsigned int closedlistsize = 9834497;
//				unsigned int closedlistsize = 45212177;
				for (unsigned int i = 0; i < argc; ++i) {
					if (sscanf(argv[i], "closed-%u", &closedlistsize) == 1) {
					}
				}

				////////////////////////////////
				/// Auto-Selection
				////////////////////////////////
				HDAstarHeap<MSA, MSAZobrist<MSA> > *subsearch =
						new HDAstarHeap<MSA, MSAZobrist<MSA> >(msa,
								std::stoi(argv[2]), 1000000, 1000000,
								std::stoi(argv[3]), 0, closedlistsize, max_h, 1000000);

				MSA::State g = msa.initial();
				subsearch->setTimer(timer);
				subsearch->search(g);
				// TODO: those gets are not so optimal.
				//       these operations are included in system execution time.
				std::vector<unsigned int> gends = subsearch->getGenerations();
				std::vector<unsigned int> pushes = subsearch->getSelfPushes();
				std::vector<unsigned int> expds = subsearch->getExpansions();
				std::vector<unsigned int> dups = subsearch->getDuplicates();
				int tnum = std::stoi(argv[3]);
				unsigned int gend = 0;
				unsigned int push = 0;
				unsigned int expd = 0;
				unsigned int dup = 0;
				for (int i = 0; i < tnum; ++i) {
					gend += gends[i];
					push += pushes[i];
					expd += expds[i];
					dup += dups[i];
				}


				estm_send_ratio = 1.0 - (double) push / (double) gend;
				estm_dup_ratio = (double) dup / (double) expd;
				delete subsearch;


				search = new HDAstarHeap<MSA, MSAZobrist<MSA> >(msa,
						std::stoi(argv[2]), 1000000, 1000000,
						std::stoi(argv[3]), 0, closedlistsize, max_h, 1000000);

//				search = new HDAstar<MSA, MSAZobrist<MSA> >(msa,
//						std::stoi(argv[2]), 1000000, 1000000,
//						std::stoi(argv[3]), 0, closedlistsize, max_h, 1000000);

			} else if (strcmp(argv[1], "hdastar-hwd") == 0) {
				// hyperplane work distribution
				// abst == # of threads
				search = new HDAstarHeap<MSA, Hyperplane<MSA> >(msa,
						std::stoi(argv[2]), 1000000, 1000000,
						std::stoi(argv[2]), 0, 45212177, max_h, 1000000);

			} else if (strcmp(argv[1], "wa+hdastar") == 0) {
				double weight = 1;
				for (unsigned int i = 0; i < argc; ++i) {
					if (sscanf(argv[i], "w-%lf", &weight) == 1) {
					}
				}
				search = new AstarHeap<MSA>(msa, max_h * weight, weight);
				MSA::State init = msa.initial();
				double wall0 = walltime();
				std::vector<MSA::State> path = search->search(init);
				double wtime = search->wtime - wall0;
				unsigned int cost = search->incm;
				printf("precalc cost = %u\n", cost);
				dfpair(stdout, "precalc wall time", "%g", wtime);
				dfpair(stdout, "precalc nodes expanded", "%lu", search->expd);
				dfpair(stdout, "precalc nodes generated", "%lu", search->gend);
				delete search;

				unsigned int closedlistsize = 9834497;
//				unsigned int closedlistsize = 45212177;
				for (unsigned int i = 0; i < argc; ++i) {
					if (sscanf(argv[i], "closed-%u", &closedlistsize) == 1) {
					}
				}

				search = new HDAstarHeap<MSA, MSAZobrist<MSA> >(msa,
						std::stoi(argv[2]), 1000000, 1000000,
						std::stoi(argv[3]), 0, closedlistsize, max_h, cost + 1);
			} else {
				throw Fatal("Unknown algorithm: %s", argv[1]);
			}

			dfheader(stdout);
			dfpair(stdout, "algorithm", "%s", argv[1]);
			dfpair(stdout, "initial heuristic", "%d", msa.h(init));
			double wall0 = walltime(), cpu0 = cputime();
			// HERE!
			std::vector<MSA::State> path = search->search(init);

//			double wtime = walltime() - wall0, ctime = cputime() - cpu0;
			double wtime = search->wtime - wall0, ctime = search->ctime - cpu0;


			///////////////////////////
			/// auto selection
			///////////////////////////
			double send_ratio = 1.0
					- (double) search->push / (double) search->gend;
			double dup_ratio = (double) search->dup / (double) search->expd;
			printf("estimated_send_ratio: %f : %f sec\n", estm_send_ratio,
					timer);
			printf("estimated_dup_ratio: %f : %f sec\n", estm_dup_ratio, timer);
			printf("actual_send_ratio: %f\n", send_ratio);
			printf("actual_dup_ratio: %f\n", dup_ratio);

			dfpair(stdout, "total wall time", "%g", wtime);
			dfpair(stdout, "total cpu time", "%g", ctime);
			dfpair(stdout, "total nodes expanded", "%lu", search->expd);
			dfpair(stdout, "total nodes generated", "%lu", search->gend);
			dfpair(stdout, "solution length", "%u", (unsigned int) path.size());
			msa.print_alignment(path);
		}
	} catch (const Fatal &f) {
		fputs(f.msg, stderr);
		fputc('\n', stderr);
		return 1;
	}

	return 0;
}

