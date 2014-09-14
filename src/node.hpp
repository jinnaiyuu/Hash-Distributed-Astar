/*
 * node.hpp
 *
 *  Created on: Jul 26, 2014
 *      Author: yuu
 */

#ifndef NODE_HPP_
#define NODE_HPP_

template<class D>
class Node {
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

	void setindex(int i) {
	}

	const typename D::PackedState &key() {
		return packed;
	}

	HashEntry<Node> &hashentry() {
		return hentry;
	}
};

#endif /* NODE_HPP_ */
