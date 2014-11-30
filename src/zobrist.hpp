/*
 * zobrist.h
 *
 *  Created on: Jun 10, 2014
 *      Author: yuu
 */

#ifndef ZOBRIST_H_
#define ZOBRIST_H_

template<int size>
class Zobrist {
public:
	enum ABST {SINGLE = 0, PAIR = 1, LINE = 2, BLOCK = 3};

// Should delete compatibility for performance.
	Zobrist(int tnum_ = 1, ABST abst = SINGLE) :
			tnum(tnum_) {
		initZobrist(abst);
	}


	/**
	 * @param number: The number slided
	 * @param from : Where the number is in parent node
	 * @param to : Where the number is in child node
	 * @return the value to XOR to the Zobrist value of parent.
	 */

	unsigned char inc_hash(unsigned char previous, const int number,
			const int from, const int to, const char* const newBoard) {
		return (previous ^ inc_zbr[number][from][to]) % tnum;
	}

	unsigned char hash_tnum(const char* const board) {
		return hash(board) % tnum;
	}

//#define RANDOM_ZOBRIST_INITIALIZATION
private:
	void initZobrist(ABST abst) {
		gen = std::mt19937(rd());
		dis = std::uniform_int_distribution<>(0, tnum - 1);
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
		}

		for (int i = 1; i < size; ++i) { // num
			for (int j = 0; j < size; ++j) { // from
				for (int k = 0; k < size; ++k) { // to
					inc_zbr[i][j][k] = zbr[i][j] ^ zbr[i][k];
				}
			}
		}
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
		for (int i = 1; i < size; ++i) {
			for (int j = 0; j < size; j += 4) {
				int r = random();
				zbr[i][j] = r;
				zbr[i][j + 1] = r;
				zbr[i][j + 2] = r;
				zbr[i][j + 3] = r;
			}
		}
	}

	void block() {
		int js[4] = { 0, 2, 8, 10 };
		for (int i = 1; i < size; ++i) {
			for (int j = 0; j < 4; ++j) {
				int r = random();
				zbr[i][js[j]] = r; // zbr[number][place]
				zbr[i][js[j] + 1] = r;
				zbr[i][js[j] + 4] = r;
				zbr[i][js[j] + 4 + 1] = r;
			}
		}
	}

	void two() {
		for (int i = 1; i < size; ++i) {
			for (int two = 0; two < 1; ++ two) {
				int r = random();
				for (int j = 0; j < size / 2; ++j) {
					zbr[i][j + two * size / 2] = r;
				}
			}
		}
	}


	int mdist(int number, int place) {
		int width = 4; // Hard coding
		int row = number / width, col = number % width;
		int grow = place / width, gcol = place % width;
		int sum = abs(gcol - col) + abs(grow - row);
	}

	// The method to return zobrist value for the very first node.
	unsigned char hash(const char* const board) {
		unsigned char h = '\0';
		for (int i = 0; i < 16; ++i) {
			h = (h ^ zbr[board[i]][i]);
		}
		return h;
	}

	int random() {
		return dis(gen);
	}

	// Currently hard coding, set to 16.
	int tnum;
	unsigned char zbr[size][size];
	// inc_zbr is the incremental XOR value for zobrist hash function.
	// The value to XOR when the number moved from a to b is
	// inc_zbr[number][a][b] or inc_zbr[number][b][a]
	unsigned char inc_zbr[size][size][size];


	std::random_device rd;
	std::mt19937 gen;
	std::uniform_int_distribution<> dis;
};

#endif /* ZOBRIST_H_ */
