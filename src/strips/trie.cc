#include "trie.hpp"
#include "utils.hpp"
#include <iostream>

unsigned int Trie::nPredicates;

Trie::Node* Trie::Node::findChild(unsigned int c) {
	for (int i = 0; i < mChildren.size(); i++) {
		Node* tmp = mChildren.at(i);
		if (tmp->precondition() == c) {
			return tmp;
		}
	}

	return NULL;
}

// TODO: store precondition values to parent nodes rather than child nodes.
/**
 * @param propositions are ordered.
 *
 */
std::vector<Trie::Node*> Trie::Node::findMatchingChildren(
		const std::vector<unsigned int>& propositions) {
	std::vector<Trie::Node*> intersection;
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
	while (cit < mChildren.size() && pit < propositions.size()) {
//		if (mChildren[cit] == NULL) {
//			std::cout << "trie.c: nullptr" << std::endl;
//		}
//		Trie::Node* m = mChildren[cit];
//		if (m == NULL) {
//			std::cout << "nullptr" << std::endl;
//		}

		if (mChildren[cit]->precondition() == propositions[pit]) {
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
	delete root;
}

/**
 * Method to build efficient trie.
 * Efficient trie is a tree with minimal number of edges.
 * Trie access is being the overhead of this planner.
 * This method is to improve the planner by solving this bottle neck.
 *
 * Algorithm:
 * 1. List all preconditions for actions and rank preconditions with the number of appearance.
 * 2. Add node of the most frequent precondition P.
 * 3. Under the node, build trie with all actions with precondition P.
 * 4. Delete all actions with the precondition P.
 * 5. For the rest of the actions, add a node of the most frequent precondition P'.
 * 6. repeat 2~5.
 *
 */
void Trie::Node::buildTrie(
		const std::vector<std::pair<std::vector<unsigned int>, unsigned int> >& actions,
		const std::vector<unsigned int>& parents_keys) {
	std::vector<unsigned int> prec_freq(nPredicates, 0); // frequencies of each predicates.
	for (int a = 0; a < actions.size(); ++a) {
		for (int i = 0; i < actions[a].first.size(); ++i) {
			++prec_freq[actions[a].first[i]];
		}
		if (actions[a].first == parents_keys) {
			this->setWordMarker();
			this->addAction(actions[a].second);
//			std::cout << "a:" << actions[a].second;
		}
	}
	std::sort(this->mActions.begin(), this->mActions.end());

	for (int i = 0; i < parents_keys.size(); ++i) {
		prec_freq[parents_keys[i]] = 0;
	}

	std::vector<unsigned int>::iterator max = std::max_element(
			prec_freq.begin(), prec_freq.end());
	unsigned int which_max = std::distance(prec_freq.begin(), max);

//	std::cout << "most freq = " << which_max << " with " << *max << " times"
//			<< std::endl;

	if (*max == 0) {
		return;
	}

	std::vector<std::pair<std::vector<unsigned int>, unsigned int> > actions_with_max;
	std::vector<std::pair<std::vector<unsigned int>, unsigned int> > actions_without_max;
	for (int a = 0; a < actions.size(); ++a) {
		if (isContainedSortedVectors(which_max, actions[a].first)) {
			actions_with_max.push_back(actions[a]);
		} else {
			actions_without_max.push_back(actions[a]);
		}
	}

	// create child nodes
	Node* max_n = new Node();
	max_n->setPrecondition(which_max);
	this->appendChild(max_n);
	std::vector<unsigned int> new_ps = parents_keys;
	new_ps.push_back(which_max);
	std::sort(new_ps.begin(), new_ps.end()); // parents_keys are sorted
	max_n->buildTrie(actions_with_max, new_ps);

	this->buildTrie(actions_without_max, parents_keys);

}

void Trie::buildTrie(
		const std::vector<std::pair<std::vector<unsigned int>, unsigned int> >& actions) {
//	for (int i = 0; i < actions.size(); ++i) {
//		for (int j = 0; j < actions[i].first.size(); ++j) {
//			std::cout << actions[i].first[j] << " ";
//		}
//		std::cout << ": " << actions[i].second << std::endl;
//	}


	std::vector<unsigned int> ps;
//	ps.clear();
	root->buildTrie(actions, ps);
}

// XXX: this method is going to be depricated.
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
//	std::vector<Node*> matchingChildren = root->findMatchingChildren(p);

//	std::cout << "returns ";
//	for (int i = 0; i < matchingChildren.size(); ++i) {
//		std::cout << matchingChildren[i]->precondition() << " ";
//	}
//	std::cout << std::endl;

	// TODO: check whether this sorting method is optimal or not.
	//       I guess it is not optimal, and we do not need to order the actions.
	//       also, all actions appears at most one time. therefore no need to check its uniqueness.

//	for (int i = 0; i < matchingChildren.size(); ++i) {
//		searchNodes(matchingChildren[i], p, actions);
//	}

	for (int i = 0; i < root->children().size(); ++i) {
		if (isContainedSortedVectors(root->children()[i]->precondition(), p)) {
			searchNodes(root->children()[i], p, actions);
		}
	}

	if (root->wordMarker()) {
//		actions = uniquelyMergeSortedVectors(actions, root->action());
		for (int i = 0; i < root->action().size(); ++i) {
			actions.push_back(root->action()[i]);
		}
//		actions.insert(actions.end(), root->action().begin(), root->action().end());
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
		// TODO: uniquely merge sorted vectors.
		for (int i = 0; i < c->action().size(); ++i) {
			actions.push_back(c->action()[i]);
		}
//		actions.insert(actions.end(), c->action().begin(), c->action().end());
//		actions = uniquelyMergeSortedVectors(actions, c->action());
	}
	// if c is inner node, then search deeper

//	std::vector<Node*> matchingChildren = c->findMatchingChildren(p);
//
//	for (int i = 0; i < matchingChildren.size(); ++i) {
//		searchNodes(matchingChildren[i], p, actions);
//	}
	for (int i = 0; i < c->children().size(); ++i) {
		if (isContainedSortedVectors(c->children()[i]->precondition(), p)) {
			searchNodes(c->children()[i], p, actions);
		}
	}

}

void Trie::printTree() const {
	root->printNode(0);
}
