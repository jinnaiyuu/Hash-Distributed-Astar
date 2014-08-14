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
        // Should delete compatibility for performance.
	Zobrist(int size = 16) {
		initZobrist();
	}

        // The method to return zobrist value for the very first node.
	char hash(const char* const board) {
		unsigned char h = '\0';
		for (int i = 0; i < 16; ++i) {
			h = (h ^ zbr[i][board[i]]);
		}
		return h;
	}

	// There might be more organized way to do.
	inline char hashinc(const char& num, const char& from, const char& to) {
		return zbr[from][num] ^ zbr[to][num];
	}

private:
	void initZobrist() {
		for (int i = 0; i < 16; ++i) {
			for (int j = 0; j < 16; ++j) {
				zbr[i][j] = rand();
//				printf("table[%d][%d] = %d\n", i, j, zbr[i][j]);
			}
		}
		/*
		for (int i = 0; i < 16; ++i) {
			for (int j = 0; j < 16; ++j) {
				for (int k = 0; k < 16; ++k) {
					zbrincr
				}
			}
		}
		*/

	}

	unsigned char zbr[16][16];
	// TODO: Ad hoc ...or optimization

        // the value to XOR to the zbr value. 
        // Slide 
  //	int zbrincr[16][16];
};
#endif /* ZOBRIST_H_ */
