#ifndef ACTION_TABLE_HPP_
#define ACTION_TABLE_HPP_

#include "action.hpp"
#include <vector>

// should this be an ActionTable?

/**
 * This class is a table of actions.
 * It enable strips class to
 */
class ActionTable {
public:
	ActionTable(){};

	std::vector<Action> table;

};

#endif
