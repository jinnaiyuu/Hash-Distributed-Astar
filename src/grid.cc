// Copyright 2012 Ethan Burns. All rights reserved.
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file.
#include "grid.hpp"
#include <string>
#include <iostream>
#include <cstring>

//	unsigned int initx;
//	unsigned int inity;
//
//	unsigned int goalx;
//	unsigned int goaly;
//
//	unsigned int width;
//	unsigned int height;

Grid::Grid(std::istream &in, unsigned int scale) : scale(scale) {
	char line[5000];
	char c;

	in >> width;
	in >> height;
	printf("w ,h = %u, %u\n", width, height);

	in >> line;
	if (strcmp(line, "Board:") != 0) {
		std::cerr << "Parse error: expected \"Board:\"" << std::endl;
		exit(EXIT_FAILURE);
	}
	c = in.get(); // new-line
	if (c != '\n') {
		std::cerr << std::endl << "Parse error: [" << c << "]" << std::endl;
		exit(EXIT_FAILURE);
	}

	obstacles.resize(width * height);
	for (int h = 0; h < height; h += 1) {
		for (int w = 0; w < width; w += 1) {
			c = in.get();
			if (c == '#')
				obstacles[width * h + w] = true;
			else
				obstacles[width * h + w] = false;
		}
		c = in.get(); // new-line
		if (c != '\n') {
//			cerr << endl << "Parse error: [" << c << "]" << endl;
			printf("parse error\n");
			exit(EXIT_FAILURE);
		}
	}

	// Cost (Unit/Life)
	in >> line;

	// Movement (Four-way/Eight-way)
	in >> line;

	// NOTE: *our* y-coordinates are the opposite of the ones
	// stored in the map file.  The map file uses the bottom of
	// the grid as y=0, we use the top.
	in >> initx;
	in >> inity;
//	inity = height - 1 - inity;
	in >> goalx;
	in >> goaly;

	// Make the tile bigger by scaling where the goal is.
//	printf("scale =  %d\n", scale);
//	goalx = goalx + width * (scale - 1);
//	goaly = goaly + height * (scale - 1);
	initx = initx * scale;
	inity = inity * scale;
	goalx = goalx * scale;
	goaly = goaly * scale;

	total_width = width * scale;
	total_height = height * scale;

	printf("tot w, h = %u, %u\n", total_width, total_height);
//	goaly = height - 1 - goaly;

	goal.x = goalx;
	goal.y = goaly;
//	for (unsigned int j = 0; j < height; ++j) {
//		for (unsigned int i = 0; i < width; ++i) {
//			if (obstacles[width * j + i]) {
//				printf("#");
//			} else {
//				printf(" ");
//			}
//		}
//		printf("\n");
//	}
	printf("init = (%u,%u)\n", initx, inity);
	printf("goal = (%u,%u)\n", goal.x, goal.y);
}
