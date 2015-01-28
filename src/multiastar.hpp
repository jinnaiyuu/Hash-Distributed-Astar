// Copyright 2012 Ethan Burns. All rights reserved.
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file.
#include "search.hpp"
#include "utils.hpp"
#include "hashtbl.hpp"
#include "heap.hpp"
#include "pool.hpp"

template<class D> class MultiAstar : public SearchAlg<D> {

	struct Node {
		char f, g, pop;
		int openind;
		Node *parent;
		typename D::PackedState packed;
		HashEntry<Node> hentry;

		bool pred(Node *o) {
			if (f == o->f)
				return g > o->g;
			return f < o->f;
		}

 		void setindex(int i) { }

		const typename D::PackedState &key() { return packed; }

		HashEntry<Node> &hashentry() { return hentry; }
	};

	HashTable<typename D::PackedState, Node> closed;
	Heap<Node> open;
	std::vector<typename D::State> path;
	Pool<Node> nodes;

	unsigned int n_threads;

	typename D::State init;

public:

	// closed might be waaaay too big for my memory....
	// original 512927357
	// now      200　000　000
	MultiAstar(D &d, unsigned int n_threads) :
		SearchAlg<D>(d), closed(193877777), open(120), n_threads(n_threads){ }

	std::vector<typename D::State> search(typename D::State &init) {
		this->init = init;
		pthread_t t[n_threads];
		for (int i = 0; i < n_threads; ++i) {
			pthread_create(&t[n_threads], NULL,
					(void*(*)(void*))&MultiAstar::thread_helper, this);
		}
		printf("running\n");

		for (int i = 0; i < n_threads; ++i) {
			pthread_join(t[n_threads], NULL);
		}
		printf("ran\n");
		this->wtime = walltime();
		this->ctime = cputime();

		return path;
	}

	void *thread_run(void* arg) {
		SearchAlg<D> *each_astar = NULL;
		each_astar = new Astar<D>(this->dom);
//		Tiles::State init = tiles.initial();
		typename D::State init_copy = init;
		std::vector<typename D::State> path_ = each_astar->search(init_copy);
		this->path = path_;
		return 0;
	}

	static void* thread_helper(void* arg) {
		return static_cast<MultiAstar*>(arg)->thread_run(arg);
//		thread_run();
	}

};
