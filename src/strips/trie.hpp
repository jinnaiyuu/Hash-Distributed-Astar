#ifndef TRIE_HPP_
#define TRIE_HPP_

#include "utils.hpp"
#include "action.hpp"
#include "action_table.hpp"

#include <iostream>
#include <vector>
#include <algorithm>

/**
 * Interface:
 * Prefix tree to list all possible actions for each states.
 * Each inner node contains information for preconditions.
 * Each leaf holds information of possible actions.
 *
 * Implementation:
 * Preconditions of the action are the characters for prefix tree.
 * thus the preconditions should be in
 *
 */
class Trie {
	class Node {
	public:
		Node() {
			mMarker = false;
		}
		~Node() {
//			for (int i = 0; i < mChildren.size(); ++i) {
//				delete mChildren.at(i);
//			}
		}
		unsigned int precondition() {
			return mPrecondition;
		}
		void setPrecondition(unsigned int c) {
			mPrecondition = c;
		}
		bool wordMarker() {
			return mMarker;
		}
		void setWordMarker() {
			mMarker = true;
		}
		Node* findChild(unsigned int c);
		std::vector<Node*> findMatchingChildren(
				const std::vector<unsigned int>& propositions);

		void appendChild(Node* child) {
			mChildren.push_back(child);
			std::sort(mChildren.begin(), mChildren.end(), PointerCompare());
		}
		std::vector<Node*>& children() {
			return mChildren;
		}

		void addAction(unsigned int action_key) {
			if (!isContainedSortedVectors(action_key, mActions)) {
				mActions.push_back(action_key);
			}
		}

		std::vector<unsigned int>& action() {
			return mActions;
		}

		struct PointerCompare {
			bool operator()(const Node* l, const Node* r) {
				return *l < *r;
			}
		};

		bool operator <(const Node& str) const {
			return (mPrecondition < str.mPrecondition);
		}

		void buildTrie(
				const std::vector<
						std::pair<std::vector<unsigned int>, unsigned int> >& actions,
				const std::vector<unsigned int>& parents_keys);
		void printNode(unsigned int depth);

	private:
		unsigned int mPrecondition;
		bool mMarker;

//		std::vector<unsigned int> mChildrenPreconditions;
		std::vector<Node*> mChildren;

		std::vector<unsigned int> mActions; // should this be just a number or actions?
		void buildTrie(const ActionTable& table,
				const std::vector<unsigned int>& action_keys,
				unsigned int n_predicates);
	};

public:
	Trie();
	Trie(const Trie& other);
	~Trie(); // this gonna take some time.
//	void buildTrie(const ActionTable& table,
//			const std::vector<unsigned int>& actions,
//			unsigned int n_predicates);
	void buildTrie(
			const std::vector<std::pair<std::vector<unsigned int>, unsigned int> >& actions);

	void addAction(const Action& a);
	void addRegressionAction(const Action& a);
	std::vector<unsigned int> searchPossibleActions(
			const std::vector<unsigned int>& p) const;
	std::vector<unsigned int> searchPossibleActionsBackward(
			const std::vector<unsigned int>& p) const;

	void printTree() const;
	unsigned int getSize() const {
		return nActions;
	}

	void setNPredicates(unsigned int np) {
		nPredicates = np;
	}

private:
	void searchNodes(Node* current, const std::vector<unsigned int>& p,
			std::vector<unsigned int>& actions) const;
	Node* root;
	unsigned int nActions;
	static unsigned int nPredicates;
};

#endif
