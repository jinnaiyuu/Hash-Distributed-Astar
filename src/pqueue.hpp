/*
 * pthread_queue.h
 *
 *  Created on: Jun 28, 2014
 *      Author: yuu
 */

#ifndef PQUEUE_H_
#define PQUEUE_H_

#include <queue>
#include <pthread.h>

template<typename T>
class p_queue {
private:
	std::queue<T> data;
	pthread_mutex_t m;
	pthread_cond_t cond;

public:
	p_queue() {
		pthread_mutex_init(&m, NULL);
		pthread_cond_init(&cond, NULL);
	}

	void push(T new_value) {
		pthread_mutex_lock(&m);
//		printf("pushl\n");
//		data.push(std::move(new_value));
		data.push(new_value);
		pthread_cond_signal(&cond);
		pthread_mutex_unlock(&m);
//		printf("pushul\n");
	}

	void wait_and_pop(T& value) {
//		printf("popl\n");
		pthread_mutex_lock(&m);
		while (empty()) {
			pthread_cond_wait(&cond, &m);
		}
		value = data.front();
		assert(value != NULL);
		data.pop();
		pthread_mutex_unlock(&m);
//		printf("popul\n");
	}

	bool empty() const {
//		pthread_mutex_lock(&m);
//		bool em = data.empty();
//		pthread_mutex_unlock(&m);
		return data.empty();
	}

	int size() const {
		return data.size();
	}

};

#endif /* PQUEUE_H_ */
