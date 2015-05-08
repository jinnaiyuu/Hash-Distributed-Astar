#ifndef ACTION_HPP_
#define ACTION_HPP_

#include <vector>
#include <string>

class Action {
public:
	Action(std::vector<unsigned int> adds, std::vector<unsigned int> deletes) {
		this->adds = adds;
		this->deletes = deletes;
	}

	std::vector<unsigned int> preconditions;
	std::vector<unsigned int> adds; // propositions to add.
	std::vector<unsigned int> deletes; // propositions to delete.

	unsigned int action_key;
	std::string name; // interface for human


};

#endif
