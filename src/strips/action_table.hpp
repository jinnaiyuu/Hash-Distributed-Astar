#ifndef ACTION_TABLE_HPP_
#define ACTION_TABLE_HPP_

#include "action.hpp"
#include <assert.h>
#include <vector>
#include <iostream>
#include <algorithm>

// should this be an ActionTable?

/**
 * This class is a table of actions.
 * It enable strips class to
 */
class ActionTable {
public:
	ActionTable(){
		table = new std::vector<Action>();
	};
	ActionTable(const ActionTable& other) {
		table = other.table;
	}

	~ActionTable() {
		delete table;
	}

	void addAction(Action a) {
		if (table->size() <= a.action_key) {
			table->resize(a.action_key + 1);
		}
		table->at(a.action_key) = a;
	}

	Action getAction(unsigned int key) const {
		assert(table->at(key).action_key == key);
		return table->at(key);
	}

	std::vector<Action> getActionsWhichAdds(unsigned int pred) const {
		std::vector<Action> ret;
		for (int i = 0; i < table->size(); ++i) {
			Action a = table->at(i);
			if (std::find(a.adds.begin(), a.adds.end(), pred) != a.adds.end()) {
				ret.push_back(a);
			}
		}
		return ret;
	}

	std::vector<Action> getActionsWhichDeletes(unsigned int pred) const {
		std::vector<Action> ret;
		for (int i = 0; i < table->size(); ++i) {
			Action a = table->at(i);
			if (std::find(a.deletes.begin(), a.deletes.end(), pred) != a.deletes.end()) {
				ret.push_back(a);
			}
		}
		return ret;
	}

	unsigned int getSize() {
		return table->size();
	}


	void printAllActions() {
		for (int i = 0; i < table->size(); ++i) {
			table->at(i).print();
		}
	}


private:

	std::vector<Action>* table;

};

#endif
