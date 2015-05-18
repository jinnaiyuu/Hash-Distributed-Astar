#ifndef PDB_HPP_
#define PDB_HPP_

#include "utils.hpp"

#include <vector>
#include <utility>

/**
 * pattern database to store abstract state and its
 *
 */
class PDB {
public:
	PDB() {
	}

	void addPattern(std::vector<unsigned int> pattern, int h) {
		std::pair<std::vector<unsigned int>, int > n(pattern, h);
		database.push_back(n);
	}

	int heuristic(const std::vector<unsigned int>& state) const {
		// 1. find abstract state representation
		// 2. look up database.
		// TODO: can be optimized to trie structure.
		for (int i = 0; i < database.size(); ++i) {
			if (isContainedSortedVectors(database[i].first, state)) {
				return database[i].second;
			}
		}
		return 0;
	}


private:
	// TODO: trie structure?
	// for now let's implement this as a vector.
	// not sure what makes it slow/fast before implementing.
	// std::vector<unsigned int> -> int

	// Database:
	// pair: abstract state <-> heuristic
	std::vector<std::pair<std::vector<unsigned int>, int> > database;
};

#endif
