/*
 * runner.h
 *
 *  Created on: Jul 21, 2014
 *      Author: yuu
 */

#ifndef RUNNER_H_
#define RUNNER_H_

#include "buffer.hpp"
#include "heap.hpp"
#include "hashtbl.hpp"
#include "pool.hpp"

template<class D> class Runner {

private:
	buffer<Node>* outgo_buffer;
	Heap<Node> open;
	HashTable<typename D::PackedState, Node> closed;
	Pool<Node> nodes;

public:
	Runner(){};


};

#endif /* RUNNER_H_ */
