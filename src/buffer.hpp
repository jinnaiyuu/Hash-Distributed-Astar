/*
 * buffer.hpp
 *
 *  Created on: Jul 21, 2014
 *      Author: yuu
 */

#ifndef BUFFER_HPP_
#define BUFFER_HPP_

#include <vector>
#include <pthread.h>

template<typename T>
class buffer {
private:
	std::vector<T*> buf;
	pthread_mutex_t m;

public:
	buffer() {
		pthread_mutex_init(&m, NULL);
	}

	void push(T* x) {
		pthread_mutex_lock(&m);
		buf.push_back(x);
//		printf("pushed %d\n",buf.back().num);
		pthread_mutex_unlock(&m);
	};

	bool try_push(T* x) {
		if (pthread_mutex_trylock(&m)) { // trylock return 0 when locked.
			return false;
		}
		buf.push_back(x);
		pthread_mutex_unlock(&m);
		return true;
	}

	T* pull() {
		T* ret;
		pthread_mutex_lock(&m);
		ret = buf.back();
		buf.pop_back();
//		printf("pulled %d\n", ret.num);
		pthread_mutex_unlock(&m);
		return ret;
	};

	void pull_all(T* ret[]) {
		pthread_mutex_lock(&m);
		std::copy(buf.begin(), buf.end(), ret);
		buf.clear();
		pthread_mutex_unlock(&m);
	};

	// TODO: Might be better to use move (C++11)
	std::vector<T*> pull_all() {
		pthread_mutex_lock(&m);
//		std::copy(buf.begin(), buf.end(), ret);
		std::vector<T*> ret(buf);
		buf.clear();
		pthread_mutex_unlock(&m);
		return ret;
	};

	bool isempty(){
		return !buf.size();
	}

	int size() {
		return buf.size();
	}

};

#endif /* BUFFER_HPP_ */
