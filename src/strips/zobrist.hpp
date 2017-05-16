/*
 * zobrist.h
 *
 *  Created on: Jun 10, 2014
 *      Author: yuu
 */

#ifndef STRIPS_ZOBRIST_H_
#define STRIPS_ZOBRIST_H_

#include <cmath>
#include <climits>
#include <cstdlib>
#include <math.h>

#include "../dist_hash.hpp"
#include "action.hpp"
#include "action_table.hpp"
#include "utils.hpp"

template<typename D>
class StripsZobrist: public DistributionHash<D> {
public:
	// list abstraction strategies
	enum ABST {
	};

	// TODO: Should delete compatibility for performance.
	StripsZobrist(D &d, unsigned int abst = 0, unsigned int rand_seed = 0,
			double toStructure = 0.3) :
			d(d) {
		structures = this->d.get_structures();
		xor_groups = this->d.get_xor_groups();
		actionTable = this->d.getActionTable();

		initZobrist(abst, rand_seed, toStructure);
	}

	unsigned int dist_h(const typename D::State& s) const {
		std::vector<unsigned int> p = s.propositions;
		unsigned int c = 0;
		for (unsigned int i = 0; i < p.size(); ++i) {
			c = c ^ map[p[i]];
		}
		return c;
	}

	/**
	 * TODO: incremental hash
	 * @param number: The number slided
	 * @param from : Where the number is in parent node
	 * @param to : Where the number is in child node
	 * @return the value to XOR to the Zobrist value of parent.
	 */
	unsigned int inc_hash(const unsigned int previous, const int number,
			const int from, const int to, const char* const newBoard,
			const typename D::State s) const {
		std::vector<unsigned int> p = s.propositions;
		unsigned int c = 0;
		for (unsigned int i = 0; i < p.size(); ++i) {
			c = c ^ map[p[i]];
		}
		return c;
	}

	unsigned int inc_hash(typename D::State s) const {
		return 0;
	}

//#define RANDOM_ZOBRIST_INITIALIZATION
private:
	void initZobrist(unsigned int abst, unsigned int rand_seed,
			double toStructure) {

		map.resize(d.getGroundedPredicatesSize());
		std::fill(map.begin(), map.end(), 0);
		gen = std::mt19937(rand_seed);
		dis = std::uniform_int_distribution<>(INT_MIN, INT_MAX);
#ifdef RANDOM_ZOBRIST_INITIALIZATION
		srand(time(NULL));
#endif

		// abst = 3: Abstraction. using structure. 1 key for 1 abstract state.
		// abst = 2: only using structure. TODO: what if no structure
		// abst = 1: using structure.
		// abst = 0: Zobrist Hash

		// TODO: here, we have to come up with innovative method to build up structure.
		switch (abst) {
		case 0:
			zobrist();
			break;
		case 1:
			feature_based_structure();
			zobrist();
			break;
		case 2:
			if (structures.size() < 8) {
				feature_based_structure();
			} else {
				feature_based_structure();
				zobrist();
			}
			break;
		case 3:
			if (structures.size() < 8) {
				abstraction();
			} else {
				zobrist();
			}
			break;
		case 4:
			zobrist();
			action_based_structure(toStructure);
			break;

		}
	}

// Abstraction
// hash key for each abstract state.
	void abstraction() {
		// is there a great way to find the best abstraction here?
		unsigned int size = 1;
		for (unsigned int i = 0; i < xor_groups.size(); ++i) {
			size *= xor_groups[i].size();
			if (size > 16 * 16 * 16 * 16) { // ad hoc
				break;
			}
			for (unsigned int j = 0; j < xor_groups[i].size(); ++j) {
				map[xor_groups[i][j]] = random();
			}
		}
	}

// Structured Zobrist
// 1 key for each structure.
	void feature_based_structure() {
		for (unsigned int i = 0; i < structures.size(); ++i) {
			unsigned int r = random();
			for (unsigned int j = 0; j < structures[i].size(); ++j) {
				map[structures[i][j]] = r;
			}
		}

		for (unsigned int i = 0; i < xor_groups.size(); ++i) {
			for (unsigned int j = 0; j < xor_groups[i].size(); ++j) {
				unsigned int key = xor_groups[i][j];
				if (map[key] == 0) {
					map[key] = random();
				}
			}
		}
	}

	/**
	 * Generate structure based on Action.
	 * This structure focus on action to eliminate the communication overhead.
	 *
	 */
	void action_based_structure(double toStructure) {
		int couldNotFind = 0;
		int couldFind = 0;

		std::vector<bool> isStructured(map.size(), false); // True if the proposition is used for structure.

		int actionTableSize = actionTable->getSize();

		int todo = (int) ((double) actionTableSize * toStructure); // TODO: this number should be parameterized. Possibly the parameter used in AutoTuning.

		printf("actionTableSize = %d\n", actionTableSize);
		printf("toStructure = %f\n", toStructure);
		printf("todo = %d\n", todo);
		for (int i = 0; i < todo; ++i) {
//			int r = i;
			int r = random() % actionTableSize;
			Action a = actionTable->getAction(r);
			std::vector<unsigned int> add_del = uniquelyMergeSortedVectors(
					a.adds, a.deletes);
			bool couldFindh = false;

			// not sure there is a action like this.
			if (add_del.size() == 0) {
				continue;
			}

			for (int j = 0; j < add_del.size(); ++j) {
				if (!isStructured[add_del[j]]) {
					unsigned int xoring = 0;
					for (int k = 0; k < add_del.size(); ++k) {
						if (j != k) {
							xoring = xoring ^ map[add_del[k]];
						}
					}
					map[add_del[j]] = xoring;
					couldFindh = true;
					break;
				}
			}

			if (couldFindh) {
				for (int j = 0; j < add_del.size(); ++j) {
					isStructured[add_del[j]] = true;
				}
				++couldFind;
				couldNotFind = 0;
			} else {
				++couldNotFind;
			}
			// Safety net if there are no action to structure.
			if (couldNotFind > 50) {
				break;
			}
		}
		printf("Action-based Structure: %d, %f percent (<= %f)\n", couldFind,
				(double) couldFind / (double) actionTableSize, toStructure);
	}

	void zobrist() {
		for (unsigned int i = 0; i < map.size(); ++i) {
			if (map[i] == 0) {
				map[i] = random();
			}
		}
	}

// TODO: read automatic abstraction.
//	void abstraction(unsigned int abst) {
//		printf("abst = %u\n", abst);
//		if (abst == 0) {
//			++abst;
//		}
//		for (unsigned int i = 0; i < map.size(); ++i) {
//			if (i % abst == 0) {
//				map[i] = random();
//			} else {
//				map[i] = 0;
//			}
//		}
//	}

	unsigned int random() {
		return dis(gen) + INT_MAX;
	}

	D& d;
	std::vector<std::vector<unsigned int>> structures;
	std::vector<std::vector<unsigned int>> xor_groups;
	ActionTable* actionTable;

//	unsigned int structure;

// TODO: ebable some kind of abstraction.
	std::vector<unsigned int> map;

	std::random_device rd;
	std::mt19937 gen;
	std::uniform_int_distribution<> dis;
};

#endif /* ZOBRIST_H_ */
