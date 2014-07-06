// Copyright 2012 Ethan Burns. All rights reserved.
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file.
#include "tiles.hpp"
#include <string>

Tiles::Tiles(FILE *in) {
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

	initmd();
	initoptab();
}

/**
 * not optimized
 */
Tiles::Tiles(FILE *in, int line) {

	for (int i = 0; i < line; ++i) {
		for (int t = 0; t < Ntiles; t++) {
			int p;
			int r = fscanf(in, " %u", &p);
			if (r != 1)
				throw Fatal("Failed to read the starting positions: r=%d", r);
			// TODO: THIS IS THE PROBLEM
			init[t] = p;
//			init[p] = t;
		}
	}
	initmd();
	initoptab();
}

void Tiles::initmd() {
	for (int t = 1; t < Ntiles; t++) {
		int grow = t / Width, gcol = t % Width;
		for (int l = 0; l < Ntiles; l++) {
			int row = l / Width, col = l % Width;
			md[t][l] = abs(col - gcol) + abs(row - grow);
		}
	}

	for (int t = 1; t < Ntiles; t++) {
		for (int d = 0; d < Ntiles; d++) {
			int newmd = md[t][d];

			for (int s = 0; s < Ntiles; s++)
				mdincr[t][d][s] = -100;	// some invalid value.

			if (d >= Width)
				mdincr[t][d][d - Width] = md[t][d - Width] - newmd;

			if (d % Width > 0)
				mdincr[t][d][d - 1] = md[t][d - 1] - newmd;

			if (d % Width < Width - 1)
				mdincr[t][d][d + 1] = md[t][d + 1] - newmd;

			if (d < Ntiles - Width)
				mdincr[t][d][d + Width] = md[t][d + Width] - newmd;
		}
	}
}

void Tiles::initoptab() {
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
