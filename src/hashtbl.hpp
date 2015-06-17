// Copyright 2012 Ethan Burns. All rights reserved.
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file.
#ifndef _HASHTBL_HPP_
#define _HASHTBL_HPP_

#include <vector>
#include <stdio.h>
#include <pthread.h>
#include <iostream>
//#include "pool.hpp"

template<class Node> struct HashEntry {
	unsigned long hash;
	Node *next;
};

// HashTable implements a simple single-sized hash table.
template<class Key, class Node> class HashTable {

	std::vector<Node*> buckets;
//	pthread_mutex_t m;

public:

	HashTable(unsigned int sz) :
			buckets(sz, 0) {
//		pthread_mutex_init(&m, NULL);
	}

	// find looks up the given key in the hash table and returns
	// the data value if it is found or else it returns 0.
	// TODO: strange error occuring here.
	//       possibly null input?
	Node *find(Key &key) {
		unsigned long h = key.hash();
		unsigned int ind = h % buckets.size();

		Node *p;
		try {
			for (p = buckets[ind]; p != NULL;
					p = p->hashentry().next) {
				// p is not NULL
				Key k = p->key();
//				unsigned int = p->key().hash();
//				if (p->hashentry().hash == h) {
//					break;
//				}
				if(k.eq(key)) {
					 break;
				 }
//			if (p->key().eq(key)) {
//				std::cout << "eq" << std::endl;
//			} else {
//				std::cout << "uneq" << std::endl;
//			}
				;
			}
		} catch (...) {
			std::cout << "p->key() = " << p->key().hash() << std::endl;
			std::cout << "key = " << key.hash() << std::endl;

		}

		return p;
	}

	// add adds a value to the hash table.
	void add(Node *n) {
//		pthread_mutex_lock(&m);
		unsigned long hash = n->key().hash();
		unsigned int ind = hash % buckets.size();
		n->hashentry().hash = hash;
		n->hashentry().next = buckets[ind];
		buckets[ind] = n;
		//	pthread_mutex_unlock(&m);
	}
};

#endif	// _HASHTBL_HPP_
