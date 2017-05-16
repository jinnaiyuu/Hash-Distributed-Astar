// Copyright 2012 Ethan Burns. All rights reserved.
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file.
#include "tiles24.hpp"
#include <string>
#include <cstdio>
#include <sys/stat.h>
#include <unistd.h>

Tiles24::Tiles24(FILE *in) {
	unsigned int w, h;

	if (fscanf(in, " %u %u", &w, &h) != 2)
		throw Fatal("Failed to read width and height");

	if (w != Width && h != Height)
		throw Fatal("Width and height instance/compiler option mismatch");

	if (fscanf(in, " starting positions for each tile:") != 0)
		throw Fatal("Failed to read the starting position label");

	for (int t = 0; t < Ntiles; t++) {
		int p;
		int r = fscanf(in, " %u", &p);
		if (r != 1)
			throw Fatal("Failed to read the starting positions: r=%d", r);
		// THIS IS THE PROBLEM....
		init[t] = p;
//		init[p] = t;
	}

	if (fscanf(in, " goal positions:") != 0)
		throw Fatal("Failed to read the goal position label");

	for (int t = 0; t < Ntiles; t++) {
		int p;
		if (fscanf(in, " %u", &p) != 1)
			throw Fatal("Failed to read the goal position");
		if (p != t)
			throw Fatal("Non-canonical goal positions");
	}

//	initmd();
	initoptab();
}

/**
 * not optimized
 */
Tiles24::Tiles24(FILE *in, int line) {
	for (int i = 0; i < line; ++i) {
		for (int t = 0; t < Ntiles; t++) {
			int p;
			int r = fscanf(in, " %u", &p);
			if (r != 1)
				throw Fatal("Failed to read the starting positions: r=%d", r);
			// Solved: THIS IS THE PROBLEM
			init[t] = p;
//			init[p] = t;
		}
	}
//	initmd();
//	printf("initoptab\n");
	initoptab();

	// Here read pattern database from the file.

	if (access("pat24.1256712.tab", F_OK) == -1) {
		printf("no pat24.1256712.tab file\n");
	}
	if (access("pat24.34891314.tab", F_OK) == -1) {
		printf("no pat24.34891314.tab file\n");
	}


	FILE* infile;
	infile = fopen("pat24.1256712.tab", "rb");
//	infile = fopen("pat24.156101112.tab", "rb");
//	infile = fopen("pat24.34891314.tab", "rb");
	readfile(h0, infile);
	fclose(infile);
	printf("pattern 1 2 5 6 7 12 read in\n");
//	printf("pattern 1 5 6 10 11 12 read in\n");

	infile = fopen("pat24.34891314.tab", "rb");
//	infile = fopen("pat24.234789.tab", "rb");
//	infile = fopen("pat24.1256712.tab", "rb");
	readfile(h1, infile);
	fclose(infile);
	printf("pattern 3 4 8 9 13 14 read in\n");
//	printf("pattern 2 3 4 7 8 9 read in\n");


	closed_hash_tbl.resize(Ntiles);
	for (int i = 0; i < Ntiles; ++i) {
		closed_hash_tbl[i].resize(Ntiles);
		for (int j = 0; j < Ntiles; ++j) {
			closed_hash_tbl[i][j] = random();
		}
	}
}

void Tiles24::initoptab() {
	for (int i = 0; i < Ntiles; i++) {
		optab[i].n = 0;
		if (i >= Width)
			optab[i].ops[optab[i].n++] = i - Width;
		if (i % Width > 0)
			optab[i].ops[optab[i].n++] = i - 1;
		if (i % Width < Width - 1)
			optab[i].ops[optab[i].n++] = i + 1;
		if (i < Ntiles - Width)
			optab[i].ops[optab[i].n++] = i + Width;
		assert(optab[i].n <= 4);
	}
}

unsigned char Tiles24::h0[TABLESIZE]; /* heuristic tables for pattern databases */
unsigned char Tiles24::h1[TABLESIZE];

// Read pattern database files and store them up to the array.
void Tiles24::readfile(unsigned char table[TABLESIZE], FILE* infile) {
	int s[6]; /* positions of each pattern tile */
	int index; /* direct access index */
	for (s[0] = 0; s[0] < Ntiles; s[0]++) /* generate each possible permutation */
	{
		for (s[1] = 0; s[1] < Ntiles; s[1]++) {
			if (s[1] == s[0])
				continue;
			for (s[2] = 0; s[2] < Ntiles; s[2]++) {
				if (s[2] == s[0] || s[2] == s[1])
					continue;
				for (s[3] = 0; s[3] < Ntiles; s[3]++) {
					if (s[3] == s[0] || s[3] == s[1] || s[3] == s[2])
						continue;
					for (s[4] = 0; s[4] < Ntiles; s[4]++) {
						if (s[4] == s[0] || s[4] == s[1] || s[4] == s[2]
								|| s[4] == s[3])
							continue;
						for (s[5] = 0; s[5] < Ntiles; s[5]++) {
							if (s[5] == s[0] || s[5] == s[1] || s[5] == s[2]
									|| s[5] == s[3] || s[5] == s[4])
								continue;
							index = ((((s[0] * 25 + s[1]) * 25 + s[2]) * 25
									+ s[3]) * 25 + s[4]) * 25 + s[5];
							table[index] = getc(infile); // TODO: infile
						}
					}
				}
			}
		}
	}
} /* read moves and store in array */
