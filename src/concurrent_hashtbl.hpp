// Copyright 2012 Ethan Burns. All rights reserved.
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file.
#ifndef _CONCURRENT_HASHTBL_HPP_
#define _CONCURRENT_HASHTBL_HPP_

#include <vector>
#include <stdio.h>
#include <pthread.h>
//#include "pool.hpp"

/*template<class Node> struct HashEntry {
	unsigned long hash;
	Node *next;
};*/

// HashTable implements a simple single-sized hash table.
template<class Key, class Node> class ConcurrentHashTable {

	std::vector<Node*> buckets;
	std::vector<pthread_mutex_t> ms;
public:

	ConcurrentHashTable(unsigned int sz, unsigned int division) :
			buckets(sz, 0), ms(division) { // Should this be prime?
		for (unsigned int i = 0; i < division; ++i) {
			pthread_mutex_init(&(ms[i]), NULL);
		}
	}

	// find looks up the given key in the hash table and returns
	// the data value if it is found or else it returns 0.
	Node *find(Key &key) {
		unsigned long h = key.hash();
		unsigned int ind = h % buckets.size();

		Node *p;
		for (p = buckets[ind]; p && !p->key().eq(key); p = p->hashentry().next)
			;
		return p;
	}

	// add adds a value to the hash table.
	void add(Node *n) {
//		pthread_mutex_lock(&m);
		unsigned long hash = n->key().hash();
		unsigned int ind = hash % buckets.size();
		n->hashentry().hash = hash;
		pthread_mutex_lock(&(ms[ind % ms.size()]) );
//		printf("locked\n");
		n->hashentry().next = buckets[ind];
		buckets[ind] = n;
		pthread_mutex_unlock(&(ms[ind % ms.size()]));
	}
};

#endif	// _CONCURRENT_HASHTBL_HPP_
