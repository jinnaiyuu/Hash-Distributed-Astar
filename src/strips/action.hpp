#ifndef ACTION_HPP_
#define ACTION_HPP_

#include <vector>
#include <string>
#include <iostream>

class Action {
public:
	Action() {
	}

	Action(std::string name, unsigned int action_key,
			std::vector<unsigned int> precs, std::vector<unsigned int> adds,
			std::vector<unsigned int> deletes, unsigned int action_cost = 1) :
			name(name), action_key(action_key), preconditions(precs), adds(
					adds), deletes(deletes), action_cost(action_cost) {
	}

	void print() {
		std::cout << "name : " << name << std::endl;
		std::cout << "key  : " << action_key << std::endl;
		std::cout << "precs: ";
		for (int p = 0; p < preconditions.size(); ++p) {
			std::cout << preconditions[p] << " ";
		}
		std::cout << std::endl;
		std::cout << "adds : ";
		for (int p = 0; p < adds.size(); ++p) {
			std::cout << adds[p] << " ";
		}
		std::cout << std::endl;
		std::cout << "dels : ";
		for (int p = 0; p < deletes.size(); ++p) {
			std::cout << deletes[p] << " ";
		}
		std::cout << std::endl;
	}

	// these are set public.
	std::string name; // interface for human
	unsigned int action_key;

	std::vector<unsigned int> preconditions;
	std::vector<unsigned int> adds; // propositions to add.
	std::vector<unsigned int> deletes; // propositions to delete.

	unsigned int action_cost;
};

#endif
