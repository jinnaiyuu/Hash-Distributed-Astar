#ifndef ACTION_TABLE_HPP_
#define ACTION_TABLE_HPP_

#include "action.hpp"
#include <vector>
#include <iostream>

// should this be an ActionTable?

/**
 * This class is a table of actions.
 * It enable strips class to
 */
class ActionTable {
public:
	ActionTable(){};

	void addAction(Action a) {
		table.push_back(a);
	}

	Action getAction(unsigned int key) const {
		return table[key];
	}

	unsigned int getSize() {
		return table.size();
	}

	void printAllActions() {
		for (int i = 0; i < table.size(); ++i) {
			table[i].print();
		}
	}

private:

	std::vector<Action> table;

};

#endif
