/*
 * zobrist.h
 *
 *  Created on: Jun 10, 2014
 *      Author: yuu
 */

#ifndef ZOBRIST_H_
#define ZOBRIST_H_

#include <cmath>
#include <climits>
#include <cstdlib>
#include <math.h>
#include <random>

#include "dist_hash.hpp"

template<typename D, int size>
class Zobrist: public DistributionHash<D> {
public:
	enum ABST {
		SINGLE = 0,
		PAIR = 1,
		LINE = 2,
		BLOCK = 3,
		TWO = 4,
		ABSTRACTION = 123,
		FAR_TILES = 1712,
		FOURBLOCK_24 = 4024,
		LINE_24 = 2401,
		BLOCK_24 = 2402,
		ODD_24 = 2403,
		FOURABSTRACTION_24 = 1234,
		FIVEABSTRACTION_24 = 12345,
		SZ4_ABST12345_15 = 6,
		SZ8_ABST12345_15 = 5,
		SZ2_ABST123456_24 = 7,
		SZ5_ABST123456_24 = 8,
		SPARSEST_CUT = 9,
		SPARSEST_CUT2 = 10,
	};

// Should delete compatibility for performance.
	Zobrist(unsigned int abst = 0, unsigned int rand_seed = 0) {
		initZobrist((ABST) abst, rand_seed);
//		dump_table();
	}

	unsigned int dist_h(const typename D::State& s) const {
		return hash(s.tiles);
	}

	/**
	 * @param number: The number slided
	 * @param from : Where the number is in parent node
	 * @param to : Where the number is in child node
	 * @return the value to XOR to the Zobrist value of parent.
	 */
	unsigned int inc_hash(const unsigned int previous, const int number,
			const int from, const int to, const char* const newBoard,
			const typename D::State s) const {
		return (previous ^ inc_zbr[number][from][to]);
	}

	unsigned int inc_hash(typename D::State s) const {
		return 0;
	}

//#define RANDOM_ZOBRIST_INITIALIZATION
private:
	void initZobrist(ABST abst, unsigned int rand_seed = 0) {
		gen = std::mt19937(rand_seed);
//		unsigned int max = std::numeric_limits<hashlength>::max();
//		unsigned int max = UINT_MAX;
		dis = std::uniform_int_distribution<>(INT_MIN, INT_MAX);
// Not sure I should initialize it by time as it randomize the results for each run.
#ifdef RANDOM_ZOBRIST_INITIALIZATION
		srand(time(NULL));
#endif
		for (int j = 0; j < size; ++j) {
			zbr[0][j] = 0;
		}

		// TODO: Here, we can implement some kind of tricks for load balancing.
		// 1. Abstraction
		// 2. Something from chess programming
		switch (abst) {
		case SINGLE:
			single();
			break;
		case PAIR:
			pair();
			break;
		case LINE:
			line();
			break;
		case BLOCK:
			block();
			break;
		case TWO:
			two();
			break;
		case ABSTRACTION:
			abstraction();
			break;
		case FAR_TILES:
			far_tiles_abstraction();
			break;
		case FOURBLOCK_24:
			four24();
			break;
		case LINE_24:
			line24();
			break;
		case BLOCK_24:
			block24();
			break;
		case ODD_24:
			odd24();
			break;
		case FOURABSTRACTION_24:
			fourabstraction();
			break;
		case FIVEABSTRACTION_24:
			fiveabstraction();
			break;
		case SZ4_ABST12345_15:
			block(5);
			break;
		case SZ8_ABST12345_15:
			two(5);
			break;
		case SZ2_ABST123456_24:
			two(6);
			break;
		case SZ5_ABST123456_24:
			four24(6);
			break;
		case SPARSEST_CUT:
			sparsest_cut1();
			break;
		case SPARSEST_CUT2:
			sparsest_cut2();
			break;
		default:
			printf("ERRRRRRRORRRRR\n");
			break;
		}

		for (int i = 1; i < size; ++i) { // num
			for (int j = 0; j < size; ++j) { // from
				for (int k = 0; k < size; ++k) { // to
					inc_zbr[i][j][k] = zbr[i][j] ^ zbr[i][k];
				}
			}
		}

//		dump_table();
	}

	void single() {
		for (int i = 1; i < size; ++i) {
			for (int j = 0; j < size; ++j) {
				zbr[i][j] = random();
			}
		}
	}

	void pair() {
		for (int i = 1; i < size; ++i) {
			for (int j = 0; j < size; j += 2) {
				int r = random();
				zbr[i][j] = r;
				zbr[i][j + 1] = r;
			}
		}
	}

	void line() {
		const int width = sqrt(size);
		for (int i = 1; i < size; ++i) {
			for (int j = 0; j < size; j += width) {
				int r = random();
				for (int k = 0; k < width; ++k) {
					zbr[i][j + k] = r;
				}
			}
		}
	}

	void block(int abst = size) {
		int js[4] = { 0, 2, 8, 10 };
		for (int i = 1; i < abst; ++i) {
			for (int j = 0; j < 4; ++j) {
				int r = random();
				zbr[i][js[j]] = r; // zbr[number][place]
				zbr[i][js[j] + 1] = r;
				zbr[i][js[j] + 4] = r;
				zbr[i][js[j] + 4 + 1] = r;
			}
		}
	}

	void four24(int abst = size) {
		int one[6] = { 0, 1, 2, 5, 6, 7 };
		int two[6] = { 3, 4, 8, 9, 13, 14 };
		int three[6] = { 10, 11, 15, 16, 20, 21 };
		int four[6] = { 17, 18, 19, 22, 23, 24 };

		for (int i = 1; i < abst; ++i) {
			int r = random();
			for (int j = 0; j < 6; ++j) {
				zbr[i][one[j]] = r; // zbr[number][place]
			}
			r = random();
			for (int j = 0; j < 6; ++j) {
				zbr[i][two[j]] = r; // zbr[number][place]
			}
			r = random();
			for (int j = 0; j < 6; ++j) {
				zbr[i][three[j]] = r; // zbr[number][place]
			}
			r = random();
			for (int j = 0; j < 6; ++j) {
				zbr[i][four[j]] = r; // zbr[number][place]
			}
			zbr[i][12] = 0;
		}
	}

	// AAAAA
	// BBBBB
	// CCCCC
	// DDDDD
	// EEEEE
	void line24() {
		for (int i = 1; i < size; ++i) {
			for (int row = 0; row < 5; ++row) {
				int r = random();
				for (int column = 0; column < 5; ++column) {
					zbr[i][row * 5 + column] = r; // zbr[number][place]
				}
			}
		}
	}

	// AADDD
	// AAEDD
	// AEEEC
	// BBECC
	// BBBCC
	void block24() {
		int a[5] = { 0, 1, 5, 6, 10 };
		int b[5] = { 15, 16, 20, 21, 22 };
		int c[5] = { 14, 18, 19, 23, 24 };
		int d[5] = { 2, 3, 4, 8, 9 };
		int e[5] = { 7, 11, 12, 13, 17 };
		for (int i = 1; i < size; ++i) {
			int r = random();
			for (int j = 0; j < 5; ++j) {
				zbr[i][a[j]] = r; // zbr[number][place]
			}
			r = random();
			for (int j = 0; j < 5; ++j) {
				zbr[i][b[j]] = r; // zbr[number][place]
			}
			r = random();
			for (int j = 0; j < 5; ++j) {
				zbr[i][c[j]] = r; // zbr[number][place]
			}
			r = random();
			for (int j = 0; j < 5; ++j) {
				zbr[i][d[j]] = r; // zbr[number][place]
			}
			r = random();
			for (int j = 0; j < 5; ++j) {
				zbr[i][d[j]] = r; // zbr[number][place]
			}
		}
	}

	// AAECC
	// AAECC
	// ABECD
	// BBEDD
	// BBEDD
	void odd24() {
		int a[5] = { 0, 1, 5, 6, 10 };
		int b[5] = { 11, 15, 16, 20, 21 };
		int c[5] = { 3, 4, 8, 9, 13 };
		int d[5] = { 14, 18, 19, 23, 24 };
		int e[5] = { 2, 7, 12, 17, 22 };
		for (int i = 1; i < size; ++i) {
			int r = random();
			for (int j = 0; j < 5; ++j) {
				zbr[i][a[j]] = r; // zbr[number][place]
			}
			r = random();
			for (int j = 0; j < 5; ++j) {
				zbr[i][b[j]] = r; // zbr[number][place]
			}
			r = random();
			for (int j = 0; j < 5; ++j) {
				zbr[i][c[j]] = r; // zbr[number][place]
			}
			r = random();
			for (int j = 0; j < 5; ++j) {
				zbr[i][d[j]] = r; // zbr[number][place]
			}
			r = random();
			for (int j = 0; j < 5; ++j) {
				zbr[i][d[j]] = r; // zbr[number][place]
			}
		}
	}

	void two(int abst = size) {
		for (int i = 1; i < abst; ++i) {
			for (int two = 0; two < 1; ++two) {
				int r = random();
				for (int j = 0; j < size / 2; ++j) {
					zbr[i][j + two * size / 2] = r;
				}
			}
		}
	}

	// This abstraction is divided by the position of tile 1, 2 and 3.
	// Other tiles do not matter.
	void abstraction() {
		for (int i = 1; i < size; ++i) {
			if (1 <= i && i <= 3) {
				for (int j = 0; j < size; ++j) {
					zbr[i][j] = random();
				}
			} else {
				for (int j = 0; j < size; ++j) {
					zbr[i][j] = 0;
				}
			}
		}
	}

	// This abstraction is divided by the position of tile 1, 2 and 3.
	// Other tiles do not matter.
	void abstractionToPerfectHashing() {
		int s = 1;
		for (int i = 1; i < size; ++i) {
			if (1 <= i && i <= 3) {
				for (int j = 0; j < size; ++j) {
					zbr[i][j] = j * s;
				}
				s *= size;
			} else {
				for (int j = 0; j < size; ++j) {
					zbr[i][j] = 0;
				}
			}
		}
	}

	// This abstraction is divided by the position of tile 1, 2, 3 and 4.
	// Other tiles do not matter.
	void fourabstraction() {
		for (int i = 1; i < size; ++i) {
			if (1 <= i && i <= 4) {
				for (int j = 0; j < size; ++j) {
					zbr[i][j] = random();
				}
			} else {
				for (int j = 0; j < size; ++j) {
					zbr[i][j] = 0;
				}
			}
		}
	}

	void fourabstractionToPerfectHashing() {
		int s = 1;
		for (int i = 1; i < size; ++i) {
			if (1 <= i && i <= 4) {
				for (int j = 0; j < size; ++j) {
					zbr[i][j] = j * s;
				}
				s *= 32;
			} else {
				for (int j = 0; j < size; ++j) {
					zbr[i][j] = 0;
				}
			}
		}
	}

	void fiveabstraction() {
		for (int i = 1; i < size; ++i) {
			if (1 <= i && i <= 5) {
				for (int j = 0; j < size; ++j) {
					zbr[i][j] = random();
				}
			} else {
				for (int j = 0; j < size; ++j) {
					zbr[i][j] = 0;
				}
			}
		}
	}

	void far_tiles_abstraction() {
		for (int i = 1; i < size; ++i) {
			if (i == 1 || i == 7 || i == 12) {
				for (int j = 0; j < size; ++j) {
					zbr[i][j] = random();
				}
			} else {
				for (int j = 0; j < size; ++j) {
					zbr[i][j] = 0;
				}
			}
		}
	}

	void sparsest_cut1() {
		unsigned int r;
		for (int i = 1; i < size; ++i) {
			r = random();
			for (int j = 0; j < 10; ++j) {
				zbr[i][j] = r;
			}
			r = random();
			for (int j = 10; j < size; ++j) {
				zbr[i][j] = r;
			}
		}
	}

	void sparsest_cut2() {
		unsigned int r;
		for (int i = 1; i < size; ++i) {
			r = random();
			for (int j = 0; j < 12; ++j) {
				zbr[i][j] = r;
			}
			r = random();
			for (int j = 10; j < size; ++j) {
				zbr[i][j] = r;
			}
		}
	}

	int mdist(int number, int place) const {
		int width = 4; // Hard coding
		int row = number / width, col = number % width;
		int grow = place / width, gcol = place % width;
		int sum = std::abs(gcol - col) + std::abs(grow - row);
	}

// The method to return zobrist value for the very first node.
	unsigned int hash(const char* const board) const {
		unsigned char h = '\0';
		for (int i = 0; i < 16; ++i) {
			h = (h ^ zbr[board[i]][i]);
		}
		return h;
	}

	unsigned int random() {
		return dis(gen) + INT_MAX;
	}

	void dump_table() {
		for (int i = 0; i < size; ++i) {
			for (int j = 0; j < size; ++j) {
				printf("(%d, %d) = %u\n", i, j, zbr[i][j]);
			}
		}
	}

	unsigned int zbr[size][size];
// inc_zbr is the incremental XOR value for zobrist hash function.
// The value to XOR when the number moved from a to b is
// inc_zbr[number][a][b] or inc_zbr[number][b][a]
	unsigned int inc_zbr[size][size][size];

	std::random_device rd;
	std::mt19937 gen;
	std::uniform_int_distribution<> dis;
};

#endif /* ZOBRIST_H_ */
