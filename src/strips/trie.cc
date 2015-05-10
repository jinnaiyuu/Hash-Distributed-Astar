#include "trie.hpp"
#include <iostream>

Trie::Node* Trie::Node::findChild(unsigned int c) {
	for (int i = 0; i < mChildren.size(); i++) {
		Node* tmp = mChildren.at(i);
		if (tmp->precondition() == c) {
			return tmp;
		}
	}

	return NULL;
}

void Trie::Node::printNode(unsigned int depth) {
	std::cout << std::string(depth * 2, ' ') << mPrecondition << "(";
	for (int i = 0; i < mActions.size(); ++i) {
		std::cout << mActions[i] << " ";
	}
	std::cout << ")" << std::endl;
	for (int i = 0; i < mChildren.size(); ++i) {
		mChildren[i]->printNode(depth + 1);
	}
}

Trie::Trie() {
	root = new Node();
}

Trie::~Trie() {
	// Free memory
//	for (int i = 0; i < root->children().size(); ++i) {
//		delete root->children().at(i);
//	}
	delete root;
}

void Trie::addAction(const Action& a) {
	Node* current = root;
	if (a.preconditions.size() == 0) {
		current->setWordMarker(); // an empty word
//		std::cout << "empty preconditions" << std::endl;
		current->addAction(a.action_key);
		return;
	}

	for (int i = 0; i < a.preconditions.size(); i++) {
		Node* child = current->findChild(a.preconditions[i]);
		if (child != NULL) {
			current = child;
		} else {
			Node* tmp = new Node();
			tmp->setPrecondition(a.preconditions[i]);
			current->appendChild(tmp);
			current = tmp;
		}
		if (i == a.preconditions.size() - 1) {
			current->setWordMarker();
			current->addAction(a.action_key);
		}
	}
}

/**
 * @param p: state propositions.
 *
 */
std::vector<unsigned int> Trie::searchPossibleActions(
		const std::vector<unsigned int>& p) const {
	std::vector<unsigned int> actions;

	for (int i = 0; i < root->children().size(); ++i) {
		Node* c = root->children().at(i);
		unsigned int precond = c->precondition();
		// TODO: should be optimized.
		if (find(p.begin(), p.end(), precond) != p.end()) { // matched
			searchNodes(c, p, actions);
		}
	}

	std::sort(actions.begin(), actions.end());
	actions.erase(unique(actions.begin(), actions.end()), actions.end());
	return actions;
}

/**
 * @param c: current node
 * @param p: state propositions
 * @param actions: return value.
 *
 */
void Trie::searchNodes(Node* c, const std::vector<unsigned int>& p,
		std::vector<unsigned int>& actions) const {
	if (c->wordMarker()) {
//		std::cout << "marked: " << c->action().size() << " actions added." << std::endl;
		for (int i = 0; i < c->action().size(); ++i) {
			actions.push_back(c->action()[i]);
		}
	}
	// if c is inner node, then search deeper
	for (int i = 0; i < c->children().size(); ++i) {
		Node* cc = c->children().at(i);
		int precond = cc->precondition();
		// TODO: should be optimized.
		if (find(p.begin(), p.end(), precond) != p.end()) { // matched
			searchNodes(cc, p, actions);
		}
	}
}

void Trie::printTree() {
	root->printNode(0);
}
