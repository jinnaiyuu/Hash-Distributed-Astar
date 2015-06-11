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

/**
 * @param propositions are ordered.
 *
 */
std::vector<Trie::Node*> Trie::Node::findMatchingChildren(const std::vector<unsigned int>& propositions) {
	std::vector<Node*> intersection;
	unsigned int cit = 0;
	unsigned int pit = 0;

//	std::cout << "findMatchingChildren" << std::endl;
//	std::cout << "children ";
//	for (int i = 0; i < mChildren.size(); ++i) {
//		std::cout << mChildren[i]->mPrecondition << " ";
//	}
//
//	std::cout << std::endl << "propositions ";
//	for (int i = 0; i < propositions.size(); ++i) {
//		std::cout << propositions[i] << " ";
//	}

	while(cit < mChildren.size() && pit < propositions.size()) {
		if(mChildren[cit]->mPrecondition == propositions[pit]) {
			intersection.push_back(mChildren[cit]);
			++cit;
			++pit;
		} else if (mChildren[cit]->mPrecondition < propositions[pit]) {
			++cit;
		} else {
			++pit;
		}
	}


//	std::cout << std::endl << "returns ";
//	for (int i = 0; i < intersection.size(); ++i) {
//		std::cout << intersection[i]->mPrecondition << " ";
//	}
//	std::cout << std::endl;

	return intersection;
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
	nActions = 0;
}

Trie::Trie(const Trie& other) {
//	std::cout << "copy operator" << std::endl;
	root = other.root;
}

Trie::~Trie() {
	// Free memory
//	for (int i = 0; i < root->children().size(); ++i) {
//		delete root->children().at(i);
//	}
//	delete root;
}

void Trie::addAction(const Action& a) {
	Node* current = root;
	++nActions;
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

// for regression planning, adds will be the preconditions.
// preconditions will be the add action.
void Trie::addRegressionAction(const Action& a) {
	Node* current = root;
	++nActions;

	if (a.adds.size() == 0) {
		current->setWordMarker(); // an empty word
//		std::cout << "empty preconditions" << std::endl;
		current->addAction(a.action_key);
		return;
	}

	for (int i = 0; i < a.adds.size(); i++) {
		Node* child = current->findChild(a.adds[i]);
		if (child != NULL) {
			current = child;
		} else {
			Node* tmp = new Node();
			tmp->setPrecondition(a.adds[i]);
			current->appendChild(tmp);
			current = tmp;
		}
		if (i == a.adds.size() - 1) {
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

//	std::cout << "searchPossibleActions" << std::endl;
	std::vector<Node*> matchingChildren = root->findMatchingChildren(p);

//	std::cout << "returns ";
//	for (int i = 0; i < matchingChildren.size(); ++i) {
//		std::cout << matchingChildren[i]->precondition() << " ";
//	}
//	std::cout << std::endl;

	for (int i = 0; i < matchingChildren.size(); ++i) {
		searchNodes(matchingChildren[i], p, actions);
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
//	std::cout << "searchNodes" << std::endl;


	if (c->wordMarker()) {
//		std::cout << "marked: " << c->action().size() << " actions added." << std::endl;
		// TODO: push all
		for (int i = 0; i < c->action().size(); ++i) {
			actions.push_back(c->action()[i]);
		}
	}
	// if c is inner node, then search deeper

	std::vector<Node*> matchingChildren = c->findMatchingChildren(p);

	for (int i = 0; i < matchingChildren.size(); ++i) {
		searchNodes(matchingChildren[i], p, actions);
	}
}



void Trie::printTree() const {
	root->printNode(0);
}
