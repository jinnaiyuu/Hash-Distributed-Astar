#ifndef ACTION_TABLE_HPP_
#define ACTION_TABLE_HPP_

#include "action.hpp"
#include <assert.h>
#include <vector>
#include <iostream>

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
