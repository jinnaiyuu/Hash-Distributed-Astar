// Copyright 2012 Ethan Burns. All rights reserved.
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file.
#include "tsp.hpp"
#include <string>
#include <iostream>
#include <cstring>

Tsp::Tsp(std::istream &in) {

	in >> number_of_cities;

	printf("cities = %u\n", number_of_cities);

	double* x = new double[number_of_cities];
	double* y = new double[number_of_cities];

	for (int t = 0; t < number_of_cities; ++t) {
		in >> x[t];
		in >> y[t];
	}

//	for (int t = 0; t < number_of_cities; t++) {
//		printf("x,y = %f, %f\n", x[t], y[t]);
//	}

	distances.resize(number_of_cities * number_of_cities);

	for (int from = 0; from < number_of_cities; from++) {
		for (int to = 0; to < number_of_cities; to++) {
			distances[from * number_of_cities + to] =
					static_cast<unsigned int>(md(x[from], y[from], x[to], y[to]) * 10000);
		}
	}

//	for (int t = 0; t < number_of_cities; t++) {
//		for (int f = 0; f < number_of_cities; f++) {
//			printf("%u ", distances[f * number_of_cities + t]);
//		}
//		printf("\n");
//	}

	delete x;
	delete y;
}

