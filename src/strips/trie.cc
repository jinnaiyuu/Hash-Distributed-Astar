#include "trie.hpp"


Trie::Node* Trie::Node::findChild(unsigned int c) {
	for (int i = 0; i < mChildren.size(); i++) {
		Node* tmp = mChildren.at(i);
		if (tmp->precondition() == c) {
			return tmp;
		}
	}

	return NULL;
}

Trie::Trie() {
	root = new Node();
}

Trie::~Trie() {
	// Free memory
	for (int i = 0; i < root->children().size(); ++i) {
		delete root->children().at(i);
	}
	delete root;
}

void Trie::addAction(const Action& a) {
	Node* current = root;
	if (a.preconditions.size() == 0) {
		current->setWordMarker(); // an empty word
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
		if (i == a.preconditions.size() - 1)
			current->setWordMarker();
	}
}

std::vector<unsigned int> Trie::searchPossibleActions(
		const std::vector<unsigned int>& p) {
	Node* current = root;
	std::vector<unsigned int> actions;

	if (current->wordMarker()) {
		actions.insert(actions.end(), current->action().begin(), current->action().end());
	}
	for (int i = 0; i < current->children().size(); ++i) {
		Node* c = current->children().at(i);
		unsigned int precond = c->precondition();
		// TODO: should be optimized.
		if (find(p.begin(), p.end(), precond) != p.end()) { // matched
			searchNodes(c, p, actions);
		}
	}
	// no need to be sorted. for debugging.
	std::sort(actions.begin(), actions.end());

	return actions;
}

void Trie::searchNodes(Node* c, const std::vector<unsigned int>& p,
		std::vector<unsigned int> actions) {
	if (c->wordMarker()) {
		actions.insert(actions.end(), c->action().begin(), c->action().end());
	}
	// if c is inner node, then search deeper
	for (int i = 0; i < c->children().size(); ++i) {
		Node* cc = c->children().at(i);
		int precond = c->precondition();
		// TODO: should be optimized.
		if (find(p.begin(), p.end(), precond) != p.end()) { // matched
			searchNodes(cc, p, actions);
		}
	}
}

void Trie::printTree() {

}
