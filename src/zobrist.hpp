/*
 * zobrist.h
 *
 *  Created on: Jun 10, 2014
 *      Author: yuu
 */

#ifndef ZOBRIST_H_
#define ZOBRIST_H_

class Zobrist {
public:
	Zobrist(int size = 16) {
		initZobrist();
	}

	char hash(int* board) {
		int h = 0;
		for (int i = 0; i < 16; ++i) {
			h = (h ^ table[i + 16 * board[i]]);
		}
		return h;
	}

	char incremental_hash(char* board, int op) {

	}

private:
	void initZobrist() {
		for (int i = 0; i < 16; ++i) {
			for (int j = 0; j < 16; ++j) {
				zbr[i + 16 * j] = rand();
				printf("table[%d][%d] = %d\n", i, j, zbr[i + 16 * j]);
			}
		}
		for (int i = 0; i < 16; ++i) {
			for (int j = 0; j < 16; ++j) {
				for (int k = 0; k < 16; ++k) {
					zbrincr
				}
			}
		}
	}
	// TODO: Ad hoc ...or optimization
	char zbr[16][16];
	int zbrincr[16][16][16];
};
#endif /* ZOBRIST_H_ */
