// Copyright 2012 Ethan Burns. All rights reserved.
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file.
#include "strips.hpp"
#include "action.hpp"
#include "utils.hpp"
#include "dijkstra.hpp"
#include "dijkstra2.hpp"
#include "graph.hpp"
#include "../astar.hpp"
#include "../astar_heap.hpp"
#include "../search.hpp"

#include <string>
#include <iostream>
#include <algorithm>
#include <iterator>
using namespace std;

////////////////////////////////////////////
/// heuristics
////////////////////////////////////////////

int Strips::hmax(const State& s) const {
//	$h_max$ (a simplification of the delete relaxation; admissible).
//	Ignore all delete effects, then for each goal proposition, compute the
//	minimal cost of achieving that single proposition. Compute the max
//	over all goal propositions. This is clearly a lower bound on the
//	delete relaxation, and is also an admissible heuristic for the
//	original planning problem.
	double start = walltime();
	Strips relaxed;
	relaxed.setActionTable(actionTable);
	relaxed.setActionTrie(actionTrie);
	relaxed.setInitState(s.propositions);
	relaxed.set_heuristic(0); // goal count doesn't make sense as there is only a single goal.
	relaxed.setGroundedPredicates(g_predicates);
	relaxed.setIsDeleteRelaxed(true);
	double init = walltime();
	int max = 0;

//	std::cout << "init " << init - start << std::endl;
	for (int i = 0; i < goal_condition.size(); ++i) {
		double init_i = walltime();
//		std::cout << i << ": goal condition = " << g_predicates->at(goal_condition[i]).symbol << ": ";
		std::vector<unsigned int> g;
		g.push_back(goal_condition[i]);
		relaxed.setGoalCondition(g);
		// TODO: a bit adhoc, but wouldn't matter.
//		SearchAlg<Strips>* search = new Astar<Strips>(relaxed, 50, 1.0, 20000, 1973);
		SearchAlg<Strips>* search = new AstarHeap<Strips>(relaxed, 50, 1.0,
				20000, 1973);
		State copy = relaxed.initial();
//		std::cout << "start" << std::endl;
//		print_state(copy.propositions);
		std::vector<Strips::State> path = search->search(copy);
//		std::cout << "length = " << path.size() << std::endl;
		if (max < path.size() - 1) {
			max = path.size() - 1;
		}
		delete search;
//		std::cout << walltime() - init_i << ", " << std::endl;
	}
	double total = walltime();
//	std::cout << std::endl << "total " << total - start << std::endl;
//	dffooter(stdout);
//	std::cout << "max = " << max << std::endl;
	return max;
}

int Strips::pdb(const State& s) const {
	return pd->heuristic(s.propositions);
}

// TODO: implement true or false group. binary condition.
void Strips::buildPDB() {
	pd = new PDB();

	std::string name = domain_ + "_" + instance_ + ".pat";

	if (!built_pdb && pd->read_database(name)) {
//		pd->dump_all(name + "copy");
		return;
	}

	////////////////////////////////////////
	/// Construction of PDB
	////////////////////////////////////////
	std::cout << "generating PDB..." << std::endl;

	// instantiate PredicateArgs group for all objects.

	// unsigned ints arekeys for GroundedPredicates.
	std::vector<std::vector<unsigned int>> pdb_groups;

	unsigned int size = 1;
	bool full = false;

	for (int g = 0; g < xor_groups.size(); ++g) {
		bool has_goal = false;
		for (int c = 0; c < goal_condition.size(); ++c) {
			if (find(xor_groups[g].begin(), xor_groups[g].end(),
					goal_condition[c]) != xor_groups[g].end()) {
				has_goal = true;
				break;
			}
		}
		if (!has_goal) {
			continue;
		}

//		unsigned int rest = 1
//				<< (goal_condition.size() - pdb_groups.size() - 1);

		if (size * xor_groups[g].size() > 100000) {
			full = true;
			continue;
		}
		pdb_groups.push_back(xor_groups[g]);
		size *= xor_groups[g].size();
		std::cout << "size = " << size << std::endl;
	}

	if (size <= 1) {
		std::cout << "default back to goal count" << std::endl;
		set_heuristic(1);
		return;
	}

// TODO: for all goal_conditions which is out of groups,
//       add TRUE OR FALSE group to the patterns.
	for (int c = 0; c < goal_condition.size(); ++c) {
		if (size * 2 > 100000) {
			break;
		}

		bool is_grouped = false;
		for (int gs = 0; gs < pdb_groups.size(); ++gs) {
			if (find(pdb_groups[gs].begin(), pdb_groups[gs].end(),
					goal_condition[c]) != pdb_groups[gs].end()) {
				is_grouped = true;
				break;
			}
		}

		// TODO: not sure what if we have no xor groups with goal propositions.
		if (!is_grouped) {
			std::vector<unsigned int> g;
			g.push_back(goal_condition[c]);
			unsigned int t = TRUE_PREDICATE;
			g.push_back(t);
			pdb_groups.push_back(g);
			size *= 2;
		}
	}

	std::cout << size << " patterns to build" << std::endl;

	std::cout << "goal conditions = ";
	for (int c = 0; c < goal_condition.size(); ++c) {
		std::cout << "(" << g_predicates->at(goal_condition[c]).symbol << ") ";
	}
	std::cout << std::endl;

// TODO: groups should based on whether it contains the goal conditions.
//       therefore the max number of groups would be bounded by the #goal conditions.
//       for now, just trash groups without goal conditions.
	std::vector<std::vector<unsigned int> >::iterator it = pdb_groups.begin();
	while (it != pdb_groups.end()) {
		bool has_goal = false;
		for (int c = 0; c < goal_condition.size(); ++c) {
			if (find(it->begin(), it->end(), goal_condition[c]) != it->end()) {
				has_goal = true;
				break;
			}
		}
		if (!has_goal) {
//			std::cout << (it - groups.begin()) << " has no goal!" << std::endl;
			pdb_groups.erase(pdb_groups.begin() + (it - pdb_groups.begin()));
		} else {
			++it;
		}
	}

//	std::vector<unsigned int> ungroupeds;
// here, args_in_preds are the pattern we want. we need to add predicates which is out of groups.
	// TODO: obviously this is wasting time.
	for (int p = 0; p < g_feasible_predicates.size(); ++p) {
		bool grouped = false;
		for (int gs = 0; gs < pdb_groups.size(); ++gs) {
			for (int m = 0; m < pdb_groups[gs].size(); ++m) {
				if (pdb_groups[gs][m] == g_feasible_predicates[p]) {
					grouped = true;
					break;
				}
			}
			if (grouped) {
				break;
			}
		}
		if (!grouped) {
			ungroupeds.push_back(g_feasible_predicates[p]);
		}
	}

	std::sort(ungroupeds.begin(), ungroupeds.end());

	std::cout << "ungroupeds = ";
	for (int p = 0; p < ungroupeds.size(); ++p) {
		std::cout << "(" << g_predicates->at(ungroupeds[p]).symbol << ") ";
	}
	std::cout << std::endl;

// g_preds will be a list of grounded predicates to be balanced.
	for (int g = 0; g < pdb_groups.size(); ++g) {
		std::cout << g << " group: ";
		for (int p = 0; p < pdb_groups[g].size(); ++p) {
			if (pdb_groups[g][p] == TRUE_PREDICATE) {
				std::cout << "(true) ";
			} else {
				std::cout << "(" << g_predicates->at(pdb_groups[g][p]).symbol
						<< ") ";
			}
		}
		std::cout << std::endl;
	}

	std::cout << "XORing groups for Strucuterd Zobrist" << std::endl;
	for (int g = 0; g < xor_groups.size(); ++g) {
		std::cout << g << " group: ";
		for (int p = 0; p < xor_groups[g].size(); ++p) {
			std::cout << "(" << g_predicates->at(xor_groups[g][p]).symbol
					<< ") ";
		}
		std::cout << std::endl;
	}

//	unsigned int size = 1;
//	for (int g = 0; g < groups.size(); ++g) {
//		size *= groups[g].size();
//		if (size > 500000) {
//			break;
//		}
//	}
// TODO: pattern database can be expanded by adding more elements from

// 1. for all abstract states
// 2.     run A* search for each abstract state. (backward search?)
// 3.     put

//	if (size > 10000000) {
//		// TODO
//		std::cout << "too much patterns. need to reduce." << std::endl;
//	}

//	std::vector<unsigned int> args;

// iterate over size.
// for each pattern

	double pdb_start = walltime();

	std::vector<std::vector<unsigned int>> all_patterns_preds;

	for (int pat = 0; pat < size; ++pat) {
		std::vector<unsigned int> args_in_groups;
		std::vector<unsigned int> args_in_preds;
		args_in_groups.resize(pdb_groups.size());
		int arg_pat = pat;
		for (int a = 0; a < args_in_groups.size(); ++a) {
			args_in_groups[a] = arg_pat % pdb_groups[a].size();
			arg_pat /= pdb_groups[a].size();
		}

//		std::cout << "pat " << pat << ": ";
		for (int a = 0; a < args_in_groups.size(); ++a) {
			if (pdb_groups[a][args_in_groups[a]] == TRUE_PREDICATE) {
//				std::cout << "(true) ";
			} else {
//				std::cout << "(" << g_predicates->at(pdb_groups[a][args_in_groups[a]]).symbol << ") ";
			}
			args_in_preds.push_back(pdb_groups[a][args_in_groups[a]]);
		}
//		std::cout << endl;
		std::sort(args_in_preds.begin(), args_in_preds.end());

		all_patterns_preds.push_back(args_in_preds);
	}

	std::vector<unsigned int> true_false_preds;
	cout << "true_false_pred: ";
	for (int g = 0; g < pdb_groups.size(); ++g) {
		if (pdb_groups[g][pdb_groups[g].size() - 1] == TRUE_PREDICATE) {
			for (int gs = 0; gs < pdb_groups[g].size() - 1; ++gs) {
				true_false_preds.push_back(pdb_groups[g][gs]);
				cout << g_predicates->at(pdb_groups[g][gs]).symbol << " ";
			}
		}
	}
	cout << endl;
	std::sort(true_false_preds.begin(), true_false_preds.end());

// TODO: here, we got init state as args_in_preds.
//       we need to run search from this init state to the goal state, or the other way around.
//       efficient way is to trace back from the goal state. how do i do that?
// pattern: abstract

	// TODO: we already have action trie here.
	//       it isn't so hard to build all edges from these patterns & action trie.
	//       just make sure we're trying to run regression search, so go the opposite way.
	// Alg:  1. for all patterns, list all possible successor abstract states and store to dictionary.
	//       2. put that dictionary to Graph()
	//       3. run dijkstra
	std::vector<std::vector<unsigned int>> edge_dictionary;
	edge_dictionary.resize(all_patterns_preds.size());

	// TODO: this algorithm is a bit too slow. read Efficient Implementation of Pattern .....
	// pattern + ungroupeds
	// find all actions which deletes any of the predicates in pattern.
	for (int p = 0; p < all_patterns_preds.size(); ++p) {
		std::vector<unsigned int> abstnode = uniquelyMergeSortedVectors(
				all_patterns_preds[p], ungroupeds);
		std::vector<unsigned int> actions = actionTrie.searchPossibleActions(
				abstnode);
//		std::cout << p << ": " << actions.size() << " actions" << std::endl;
		for (int a = 0; a < actions.size(); ++a) {
			Action act = actionTable.getAction(actions[a]);
			std::vector<unsigned int> succ;
			// TODO: not sure why this is not detecting transitions for rovers.
//			for (int ads = 0; ads < act.adds.size(); ++ads) {
//				cout << act.adds[ads] << " ";
//			}
//			cout << endl;
			if (isAnyContainedSortedVectors(act.deletes, all_patterns_preds[p])
					|| isAnyContainedSortedVectors(act.adds,
							true_false_preds)) {
//				cout << act.name << " edge" << endl;
				succ = uniquelyMergeSortedVectors(abstnode, act.adds);
				succ = differenceSortedVectors(succ, act.deletes);
//				for (int succs = 0; succs < succ.size(); ++succs) {
//					if (succ[succs] != TRUE_PREDICATE) {
////						cout << "(" << g_predicates->at(succ[succs]).symbol
////								<< ") ";
//					} else {
////						std::cout << "(true)" << " ";
//					}
//				}
//				cout << endl;
				unsigned int succ_id = match_pattern(succ, pdb_groups);
				if (succ_id == p) {
//					std::cout << std::endl;
//					std::cout << "succ_id dupped " << succ_id << endl;
				} else {
//					std::cout << " -> " << succ_id << endl;
//					edge_dictionary[p].push_back(succ_id);
					edge_dictionary[succ_id].push_back(p);
				}
			} else {

			}
		}
	}

	for (int i = 0; i < edge_dictionary.size(); ++i) {
		std::sort(edge_dictionary[i].begin(), edge_dictionary[i].end());
		edge_dictionary[i].erase(
				std::unique(edge_dictionary[i].begin(),
						edge_dictionary[i].end()), edge_dictionary[i].end());
	}

//	for (int e = 0; e < edge_dictionary.size(); ++e) {
//		std::vector<unsigned int> epat = all_patterns_preds[e];
//		std::cout << e << " ";
//		for (int ps = 0; ps < epat.size(); ++ps) {
//			if (epat[ps] != TRUE_PREDICATE) {
//				std::cout << "(" << g_predicates->at(epat[ps]).symbol << ") ";
//			} else {
//				std::cout << "(true)" << " ";
//			}
//		}
//		std::cout << "-> " << endl;
//
//		for (int ss = 0; ss < edge_dictionary[e].size(); ++ss) {
//			std::vector<unsigned int> epat =
//					all_patterns_preds[edge_dictionary[e][ss]];
//			std::cout << "  ";
//			for (int ps = 0; ps < epat.size(); ++ps) {
//				if (epat[ps] != TRUE_PREDICATE) {
//					std::cout << "(" << g_predicates->at(epat[ps]).symbol
//							<< ") ";
//				} else {
//					std::cout << "(true)" << " ";
//				}
//			}
//			std::cout << endl;
//		}
//		std::cout << std::endl;
//	}

//	std::cout << "end" << std::endl;

	Graph graph(edge_dictionary);

	Dijkstra2<Graph>* search = new Dijkstra2<Graph>(graph, all_patterns_preds,
			pdb_groups);

	Graph::State g;
	g.id = match_pattern(goal_condition, pdb_groups);
	std::cout << "goal id = " << g.id << std::endl;
	search->search(g);
	std::vector<unsigned int> costs = search->get_costs();

	delete search;
	pd->setGroups(pdb_groups);
	pd->setPattern(costs);
	pd->dump_all(name);

	double pdb_end = walltime();
	std::cout << "pdb_walltime = " << pdb_end - pdb_start << std::endl;

//	pd->dump_all(name);

	return;

//	dijkstra(all_patterns_preds, pdb_groups);
//// input: patterns, ungroupeds, simplified actions,
//
////	pd.addPattern()
//
//	pd->setGroups(pdb_groups);
//

}

void Strips::dijkstra(std::vector<std::vector<unsigned int> > patterns,
		std::vector<std::vector<unsigned int> > groups) {
// 1. list transition actions which traverse patterns.
// 2. breadth first search to list all patterns.
//	std::cout << "Dijkstra init" << std::endl;
	std::cout << "Dijkstra" << std::endl;

	Dijkstra *search = new Dijkstra(*this, patterns, groups, ungroupeds);

	Strips::State init;
	init.propositions = goal_condition;
	init.h = 0;

	buildRegressionTree();

	search->search(init, 20.0 * 1);
//	print_plan(path);

	std::vector<unsigned int> costs = search->get_costs();

	// TODO: this should be done in O(1)
	for (int p = 0; p < costs.size(); ++p) {
//		std::cout << p << " " << costs[p] << std::endl;
		if (costs[p] != -1) {
			pd->addPattern(p, costs[p]);
		}
	}
	std::cout << search->expd << " nodes expanded" << std::endl;

	delete search;
}

void Strips::buildRegressionTree() {
// TODO: duplicated computation. wouldn't be so heavy i guess.
	std::vector<unsigned int> feasible_actions;
	listFeasibleActions(g_feasible_predicates, feasible_actions);

	for (int i = 0; i < feasible_actions.size(); ++i) {
		Action action = actionTable.getAction(feasible_actions[i]);
		// TODO: check if the action changes pattern. if not then discard.
		//       if any of add effect contains any of groups, then put that in the tree.

		// TODO: this is not working for now.
		if (isAnyNotContainedSortedVectors(action.adds, ungroupeds)) {
			rActionTrie.addRegressionAction(action);
		}
	}

	std::cout << "size of regression trie = " << rActionTrie.getSize()
			<< std::endl;
}

// return the number of pattern that matches the node.
unsigned int Strips::match_pattern(std::vector<unsigned int> p,
		const std::vector<std::vector<unsigned int>>& groups) {
	// TODO: build BDD (or perfect hash) for the patterns.
	unsigned int arg_pat = 0;
//		std::cout << "g: ";
	for (int gs = groups.size() - 1; gs >= 0; --gs) {
		for (int ps = 0; ps < groups[gs].size(); ++ps) {
//			std::cout << ps << " " << std::endl;
			if (isContainedSortedVectors(groups[gs][ps], p)) {
				arg_pat += ps;
				if (gs != 0) {
					arg_pat *= groups[gs].size();
				}
				break;
			}
			if (groups[gs][ps] == TRUE_PREDICATE) {
//					std::cout << gs << "," << ps << ": -2" << std::endl;
				arg_pat += ps;
				if (gs != 0) {
					arg_pat *= groups[gs].size();
				}
				break;
			}
		}
	}
	return arg_pat;
}

////////////////////////////////////////////
/// parser
////////////////////////////////////////////
Strips::Strips() {
}

/**
 * TODO: other PDDL specifications that I need to read
 * :requirements :typing :action-cost :adl
 * :constants
 * :functions
 *
 */
Strips::Strips(std::istream & d, std::istream & i) {
	d.seekg(0, std::ios_base::beg);
	i.seekg(0, std::ios_base::beg);

	std::string dt = readAll(d);
	std::string it = readAll(i);
	std::istringstream domain(dt);
	std::istringstream instance(it);

//	cout << domain.str() << endl;
//	cout << instance.str() << endl;

//	std::vector<Predicate> predicates; // uninstantiated, ungrounded
//	std::vector<Object> objects;

//	std::vector<unsigned int> g_feasible_predicates;

//	std::vector < Action >

	readDomainName(domain);
	readInstanceName(instance);

	readRequirements(domain);

// TODO: if (:requirements :typing) then
// TODO: learning types needed.
	if (typing) {
		std::cout << "parsing types..." << std::endl;
		types = readTypes(domain);

		std::cout << std::endl;
		for (int i = 0; i < types.size(); ++i) {
			if (types[i].second >= 0) {
				cout << types[i].first << " - " << types[types[i].second].first
						<< endl;
			} else {
				cout << types[i].first << endl;
			}
		}

		// Constants are objects with types independent to each instance.
		cout << endl;
		std::cout << "parsing constants..." << std::endl;
		getConstants(domain, constants, ":constants");
		// Constants will be included in init states.
//		for (int i = 0; i < constants.size(); ++i) {
//			cout << constants[i].first << " - " << constants[i].second << endl;
//		}
		if (constants.size() == 0) {
			cout << "no constants." << endl;
		}
		for (int i = 0; i < constants.size(); ++i) {
			if (!constants[i].first.empty() && !constants[i].second.empty()) {
				std::cout << constants[i].first << " ";
				Object obj;
				obj.key = objects.size();
				obj.symbol = constants[i].first;
				obj.type = matchStringIndex(types, constants[i].second);
				objects.push_back(obj);
			}
		}

	}
// 1. learn predicates from domain.pddl
	std::cout << "parsing predicates..." << std::endl;
	readPredicates(domain, predicates);
	std::cout << "generated " << predicates.size() << " lifted predicates."
			<< std::endl;

	// TODO: check if it is working fine.
// 2. learn objects instance.pddl.
	std::cout << "parsing objects..." << std::endl;
	readObjects(instance);

//	for (int i = 0; i < objects.size(); ++i) {
//		if (objects[i].type >= 0) {
//			cout << objects[i].symbol << " - " << types[objects[i].type].first
//					<< endl;
//		} else {
//			cout << objects[i].symbol << endl;
//		}
//	}
	std::cout << "generated " << objects.size() << " objects." << std::endl;

// 3. instantiate all predicates.
	std::cout << "grounding all predicates..." << std::endl;
	g_predicates = new std::vector<GroundedPredicate>();
	groundPredicates(predicates, objects, *g_predicates);
	std::cout << "generated " << g_predicates->size() << " grounded predicates."
			<< std::endl;

// 4. read initial state from instance.pddl.
	std::cout << "parsing initial state..." << std::endl;
	readInit(instance, *g_predicates);
	std::cout << init_state.size() << " predicates." << std::endl;

// 5. read goal condition from instance.pddl.
	std::cout << "parsing goal condition..." << std::endl;
	readGoal(instance, *g_predicates);
	std::cout << goal_condition.size() << " predicates." << std::endl;

// 6. read actions from domain.pddl.
	std::cout << "parsing actions..." << std::endl;
	readAction(domain, objects, predicates, *g_predicates);

	std::cout << "analyzing static predicates..." << std::endl;
	analyzeStaticLiftedPredicates();

	groundAction(domain, objects, predicates, *g_predicates);

	std::cout << "generated " << actionTable.getSize() << " grounded actions."
			<< std::endl;
//	actionTable.printAllActions();
//	for (int i = 0; i < actionTable.getSize(); ++i) {
//		Action a = actionTable.getAction(i);
//		std::cout << a.name << std::endl;
//		std::cout << "pred:  ";
//		for (int j = 0; j < a.preconditions.size(); ++j) {
//			std::cout << "(" << g_predicates->at(a.preconditions[j]).symbol << ") ";
//		}
//		std::cout << std::endl;
//		std::cout << "adds:  ";
//		for (int j = 0; j < a.adds.size(); ++j) {
//			std::cout << "(" << g_predicates->at(a.adds[j]).symbol << ") ";
//		}
//		std::cout << std::endl;
//		std::cout << "dels:  ";
//		for (int j = 0; j < a.deletes.size(); ++j) {
//			std::cout << "(" << g_predicates->at(a.deletes[j]).symbol << ") ";
//		}
//		std::cout << std::endl << std::endl;
//	}
// 7. list all feasible predicates to be true.
// init state, actions,
// TODO: this can be a bit more pruned by forward delete-reduction search.
//       if the predicate doesn't appear in delete-reduction brute force search, then
//       the predicate is not feasible.
	std::cout << "calculating feasible predicates..." << std::endl;
	listFeasiblePredicates(g_feasible_predicates);
	std::cout << g_feasible_predicates.size() << " predicates feasible."
			<< std::endl;

// 8. list all feasible actions.
	std::cout << "calculating feasible actions..." << std::endl;
	std::vector<unsigned int> feasible_actions;
	listFeasibleActions(g_feasible_predicates, feasible_actions);
	std::cout << feasible_actions.size() << " actions feasible." << std::endl;

	std::cout << "building action trie..." << std::endl;
	buildActionTrie(feasible_actions);
	std::cout << "done!" << std::endl;

// 9. list all feasible predicates again with feasible_actions.
	listFeasiblePredicatesWithActions(g_feasible_predicates, feasible_actions);
	std::cout << g_feasible_predicates.size() << " predicates feasible."
			<< std::endl;

//	std::cout << "feasible actions:" << endl;
//	for (int i = 0; i < feasible_actions.size(); ++i) {
//		Action a = actionTable.getAction(feasible_actions[i]);
//		std::cout << a.action_key << ": ";
//		std::cout << a.name << std::endl;
//		std::cout << "pred:  ";
//		for (int j = 0; j < a.preconditions.size(); ++j) {
//			std::cout << "(" << g_predicates->at(a.preconditions[j]).symbol
//					<< ") ";
//		}
//		std::cout << std::endl;
//		std::cout << "adds:  ";
//		for (int j = 0; j < a.adds.size(); ++j) {
//			std::cout << "(" << g_predicates->at(a.adds[j]).symbol << ") ";
//		}
//		std::cout << std::endl;
//		std::cout << "dels:  ";
//		for (int j = 0; j < a.deletes.size(); ++j) {
//			std::cout << "(" << g_predicates->at(a.deletes[j]).symbol << ") ";
//		}
//		std::cout << std::endl << std::endl;
//	}
	actionTrie.printTree();

	init_state.erase(
			remove_if(init_state.begin(), init_state.end(),
					[&](const unsigned int& x) {return this->predicates[this->g_predicates->at(x).lifted_key].isStatic;}),
					init_state.end());


//	for (int i = 0; i < init_state.size(); ++i) {
//		if (predicates[g_predicates->at(init_state[i]).lifted_key].isStatic) {
//
//		}
//	}
	std::vector<unsigned int> init_actions = actionTrie.searchPossibleActions(
			init_state);

	std::cout << "init state: ";
	for (int i = 0; i < init_state.size(); ++i) {
		std::cout << "(" << g_predicates->at(init_state[i]).symbol << ":"
				<< init_state[i] << ") ";
	}
	std::cout << std::endl;

	std::cout << "goal condition: ";
	for (int i = 0; i < goal_condition.size(); ++i) {
		std::cout << "(" << g_predicates->at(goal_condition[i]).symbol << ":"
				<< goal_condition[i] << ") ";
	}
	std::cout << std::endl;

//	std::cout << init_actions.size() << " initial actions." << std::endl;
//	for (int i = 0; i < init_actions.size(); ++i) {
//		actionTable.getAction(init_actions[i]).print();
//	}

//	std::cout << "building regression tree for PDB..." << std::endl;
//	buildRegressionTree(feasible_actions);

// this is only for HDA* or PDB, but we can just analyze them everytime.

	std::cout << "analyzing balances of predicates..." << std::endl;
	analyzeAllBalances(predicates);

	analyzeXORGroups();

	/**
	 * input PDDL structure
	 * domain:
	 *   :predicates
	 *     1. symbol
	 *     2. number of arguements
	 *   :action
	 *     1. symbol
	 *     2. number of parameters
	 *     3. precondition
	 *     4. add effect
	 *     5. delete effect
	 *
	 * instance:
	 *   :objects
	 *     1. symbol
	 *   :init
	 *     1. initial state
	 *   :goal
	 *     1. goal conditions
	 */

// overall output goal:
// 1. build proposition library (prefix tree)
// 2. build action library (action table)
// 3. set initial&goal state.
// all possible predicates
//
// 0. :requirements :strips (typing is not supported yet)
// 1. :predicates learn predicates from domain
// 2. instantiate all predicates from instance.pddl.
// 3. learn add&delete effects for each actions.
// 4. from initial state & add effect, list all possible predicates.
// 5. instantiate all actions.
// 6. from the list of all possible predicates, list all possible actions.
// 7. make prefix tree of preconditions&actions. (TODO: search how to make prefix tree)
}

void Strips::readDomainName(std::istream &domain) {
	domain.seekg(0, std::ios_base::beg);
	std::string text;

	while (domain >> text) {
		if (text.compare("domain") == 0) {
			break;
		}
	}
	domain >> text;
	domain_ = text;

	std::cout << "domain name = " << domain_ << std::endl;
}

void Strips::readInstanceName(std::istream &instance) {
	instance.seekg(0, std::ios_base::beg);
	std::string text;

	while (instance >> text) {
		if (text.compare("problem") == 0) {
			break;
		}
	}
	instance >> text;
	instance_ = text;

	std::cout << "instance name = " << instance_ << std::endl;
}

void Strips::readRequirements(std::istream &domain) {
	domain.seekg(0, std::ios_base::beg);
	std::string text;
	getBracket2(domain, ":requirements", 0, text);
	cout << "requirements: " << text << endl;
	if (text.find("typing") != std::string::npos) {
		typing = true;
	}
	if (text.find("action-cost") != std::string::npos) {
		action_costs = true;
	}
}

void Strips::readPredicates(std::istream &domain,
		std::vector<Predicate>& predicates) {
	domain.seekg(0, std::ios_base::beg);

	std::string text;
	getBracket2(domain, ":predicates", 0, text);

//	getText(domain, ":predicates", ":action", 0, text);
//	getText3(domain, ":predicates", ":action", 0, text);

//	replace(text, "(:predicates", "");
//	std::cout << "predicates text = " << text << std::endl;
//	std::cout << "###############################" << std::endl;

//	predicate_parser<std::string::const_iterator> g;

	std::vector<std::string> symbols;
	std::vector<unsigned int> argcs; // if no typing
	std::vector<std::vector<unsigned int>> arg_types; // if typings
	std::vector<std::string> forward_delimeds = split(text, '(');
	arg_types.resize(forward_delimeds.size());
	for (int i = 1; i < forward_delimeds.size(); ++i) {
//		cout << "read pred: " << i << ": " <<  forward_delimeds[i] << endl;
		std::vector<std::string> predicate = split(forward_delimeds[i], ')');
		std::vector<std::string> token = split(predicate[0], ' ');

//		token.erase(token.begin());

//		cout << "tokens: ";
//		for (int t = 0; t < token.size(); ++t) {
//			cout << token[t] << " ";
//		}
//		cout << endl;

		// tokens:
		if (!typing) {
			symbols.push_back(token[0]);
			argcs.push_back(token.size() - 1);
		} else {
			symbols.push_back(token[0]);
			string delim = "-";
			bool isType = false;
			unsigned int arg_num = 0;
			unsigned int arg_before = 0;
			for (int j = 1; j < token.size(); ++j) {
				if (!isType) {
					if (token[j].compare(delim) == 0) {
						isType = true;
					} else {
						++arg_num;
					}
				} else {
					int type = -1;
					for (int k = 0; k < types.size(); ++k) {
						if (types[k].first.compare(token[j]) == 0) {
							type = k;
							break;
						}
					}
					if (type == -1) {
						cout << "error: no matching type to " << token[i];
					}
					for (int k = arg_before; k < arg_num; ++k) {
						arg_types[i - 1].push_back(type);
					}
					arg_before = arg_num;
					isType = false;
				}
			}
			argcs.push_back(arg_num);
		}
	}

	predicates.resize(symbols.size());
	for (int i = 0; i < predicates.size(); ++i) {
		predicates[i].key = i;
		predicates[i].symbol = symbols[i];
		predicates[i].number_of_arguments = argcs[i];
		if (typing) {
			predicates[i].types_of_arguments = arg_types[i];
		}
	}

	for (int i = 0; i < predicates.size(); ++i) {
		std::cout << predicates[i].key << " " << predicates[i].symbol << "/"
				<< predicates[i].number_of_arguments << ": ";
		if (typing) {
			for (int j = 0; j < predicates[i].types_of_arguments.size(); ++j) {
				cout << types[predicates[i].types_of_arguments[j]].first << " ";
			}
		}
		cout << std::endl;
	}
}
/**
 * what should I implement for enabling types
 * 1. read types and make type graph.
 * 2. create predicates for types like is_type1, is_type2.
 * 3. for all actions, add precondition for the types of arguments.
 * 4. for all objects and their types, add initial state their propositions.
 *
 */
//void Strips::readTypes(std::istream &domain,
//		std::vector<Predicate>& predicates, std::vector<std::pair<std::string, std::string> >) {
//	if (!typing) {
//		return;
//	}
//	domain.seekg(0, std::ios_base::beg);
//
//	std::string text;
//	getBracket(domain, ":types", 0, text);
//	std::cout << "types = " << text << std::endl;
//// structure
//// type type type - type
//
//}
vector<pair<string, int>> Strips::readTypes(istream& domain) {
	vector<pair<string, int>> ret;
	vector<string> types_buf;

	vector<pair<string, string>> types;
	getConstants(domain, types, ":types");
	//	string text;
	for (int i = 0; i < types.size(); ++i) {
		vector<string>::iterator it;
		it = find(types_buf.begin(), types_buf.end(), types[i].second);
		if (it == types_buf.end()) {
			unsigned int w = -1;
			if (types[i].second != "") {
				pair<string, int> super(types[i].second, -1);
				types_buf.push_back(types[i].second);
				ret.push_back(super);
				w = ret.size() - 1;
			}

			pair<string, int> sub(types[i].first, w);
			types_buf.push_back(types[i].first);
			ret.push_back(sub);

		} else {
			unsigned int supertype = it - types_buf.begin();
			pair<string, int> t(types[i].first, supertype);
			types_buf.push_back(types[i].first);
			ret.push_back(t);
		}
	}
	return ret;
}

void Strips::readObjects(std::istream &instance) {
//	std::vector<std::string> strings;
	std::string text;
//	getText(instance, "(:objects", ":init", 0, text);
	getBracket2(instance, ":objects", 0, text);
//	text.substr(8);
	std::cout << "object text = " << text << endl;
	if (text.empty()) {
		return;
	}

	std::istringstream iss(text);

	vector<pair<string, string>> obs;
	getConstants(iss, obs, ":objects");

//	std::vector<std::string> tokens;
//	std::copy(std::istream_iterator<std::string>(iss),
//			std::istream_iterator<std::string>(), back_inserter(tokens));
//
//	for (int i = 1; i < tokens.size() - 1; ++i) {
//		strings.push_back(tokens[i]);
//	}
//	std::cout << "objs" << std::endl;
//	objects.resize(strings.size());
	for (int i = 0; i < obs.size(); ++i) {
		if (!obs[i].first.empty()) {
			if (obs[i].first.compare(":objects") == 0) {
				continue;
			}
//			std::cout << obs[i].first << " - " << obs[i].second << endl;
			Object obj;
			obj.key = objects.size();
			obj.symbol = obs[i].first;
			obj.type = matchStringIndex(types, obs[i].second);
			objects.push_back(obj);
		}
	}
	std::cout << std::endl;
}

void Strips::ground(Predicate& p, std::vector<unsigned int> argv,
		unsigned int key, GroundedPredicate& g, std::vector<Object>& obs) {
	g.key = key;
	g.symbol = p.symbol;
	for (int i = 0; i < argv.size(); ++i) {
		g.symbol.append(" " + obs[argv[i]].symbol);
	}
	g.lifted_key = p.key;
	g.lifted_symbol = p.symbol;
	g.arguments = argv;
}

void Strips::groundPredicates(std::vector<Predicate>& ps,
		std::vector<Object>& obs, std::vector<GroundedPredicate>& gs) {
	int gnum = 0; // TODO: should collapse with gs.size()
	for (int pnum = 0; pnum < ps.size(); ++pnum) {
		g_predicates_index.push_back(gnum);
		unsigned int argc = ps[pnum].number_of_arguments;
		std::vector<unsigned int> argv;
		if (argc == 0) { // grounded
			gs.resize(gs.size() + 1);
			ground(ps[pnum], argv, gnum, gs[gnum], obs);
			++gnum;
		} else {
			int newgnum = pow(obs.size(), argc);
//			std::cout << "instantiate " << newgnum << " preds" << std::endl;
			argv.resize(argc);
			for (unsigned int newnum = 0; newnum < newgnum; ++newnum) {
				gs.resize(gs.size() + 1);
				// build arguments list.
				for (unsigned int arg = 0; arg < argc; ++arg) {
					argv[arg] = (newnum / pow(obs.size(), argc - 1 - arg))
							% obs.size();
				}
				ground(ps[pnum], argv, gnum, gs[gnum], obs);
				++gnum;
			}
		}
	}

//	for (int gnum = 0; gnum < gs.size(); ++gnum) {
//		std::cout << gs[gnum].symbol << std::endl;
//	}
}

void Strips::readInit(std::istream &instance,
		std::vector<GroundedPredicate>& gs) {
	instance.seekg(0, std::ios_base::beg);

	std::string total_text;
	getBracket2(instance, ":init", 0, total_text);
//	getText(instance, ":init", ":goal", 0, total_text);
//	std::string line;
//	std::string init_ = ":init";
//	std::string goal_ = ":goal";
//	size_t pos;
//	std::vector<std::string> strings;
//
//// put total_text the text for inital state.
//	while (instance.good()) {
//		getline(instance, line); // get line from file
//		std::transform(line.begin(), line.end(), line.begin(), ::tolower);
//		pos = line.find(init_); // search
//		if (pos != std::string::npos) {
////			std::cout << "Found: " << line << std::endl;
//			total_text = line;
//			while (instance.good()) {
//				getline(instance, line); // get line from file
//				std::transform(line.begin(), line.end(), line.begin(),
//						::tolower);
//				pos = line.find(goal_); // search
//				if (pos != std::string::npos) {
//					break;
//				} else {
//					total_text.append(line);
//				}
//			}
//			break;
//		}
//	}

// parse the total_text.
	std::cout << "inital state: " << total_text << std::endl;

	std::vector<std::string> symbols; // parse into here.

// now we need to parse total text into vector of strings(symbols).
	std::vector<std::string> forward_delimeds = split(total_text, '(');
	for (int i = 1; i < forward_delimeds.size(); ++i) {
		std::vector<std::string> token = split(forward_delimeds[i], ')');

		symbols.push_back(token[0]);
	}

//	std::cout << "symbols" << endl;
	for (int i = 0; i < symbols.size(); ++i) {
//		std::cout << i << ": " << symbols[i] << endl;
		symbols[i] = trim(symbols[i]);

		for (int j = 0; j < gs.size(); ++j) {
//			std::cout << j << ": " << gs[j].symbol << std::endl;
			if (symbols[i].compare(gs[j].symbol) == 0) {
				// if two symbols are equal, then same predicates.
//				std::cout << " matches " << j << std::endl;
				init_state.push_back(j);
				break;
			}
		}
	}

	std::sort(init_state.begin(), init_state.end());
}

void Strips::readGoal(std::istream &instance,
		std::vector<GroundedPredicate>& gs) {
	instance.seekg(0, std::ios_base::beg);

	std::vector<std::string> strings;
	std::string text;
	getBracket2(instance, ":goal", 0, text);
	instance.clear();

// parse the total_text.
//	std::cout << "goal state: " << text << std::endl;

	std::vector<std::string> symbols; // parse into here.

// now we need to parse total text into vector of strings(symbols).
	std::vector<std::string> forward_delimeds = split(text, '(');
	for (int i = 1; i < forward_delimeds.size(); ++i) {
		std::vector<std::string> token = split(forward_delimeds[i], ')');

		symbols.push_back(token[0]);
	}

	for (int i = 0; i < symbols.size(); ++i) {
//		std::cout << i << ": " << symbols[i];
		symbols[i] = trim(symbols[i]);

		for (int j = 0; j < gs.size(); ++j) {
//			std::cout << j << ": " << gs[j].symbol << std::endl;
			if (symbols[i].compare(gs[j].symbol) == 0) {
				// if two symbols are equal, then same predicates.
//				std::cout << " matches " << j << std::endl;
				goal_condition.push_back(j);
				break;
			}
		}
	}

	std::sort(goal_condition.begin(), goal_condition.end());

}

/**
 * PDDL format
 * parameters:
 * precondition:
 * effect:       add&delete effect
 *
 */
void Strips::readAction(std::istream &domain, std::vector<Object> obs,
		std::vector<Predicate> ps, std::vector<GroundedPredicate> gs) {
	// TODO: this method is way too slow for action with more than 4 arguments.
	//       is there any way to speedup?
	// TODO	1: read type preconditions.
	//       it will vastly reduce the number of actions and speedup the instantiation.

// 1. read parameters.
// 2. read precondition (lifted).
// 3. read add&delete effect (lifted).
// 4. assign objects for parameters and ground preconditions & effects.
// 5. build Action.
//	std::vector<std::string> parameters;

	std::string text;

	// TODO: read constants in predicates.
	// 1. read parameters
	// 2. if literal does not start with ?, then it is constant.
	// 3. store object key for that.
	// 4. use that information in instantiation.
	unsigned int actionNumber = 0;
	unsigned int action_cost_buf = 0; // ad hoc implementation as getText DOES read :action-costs
	while (getBracket2(domain, ":action", actionNumber + action_cost_buf, text)) {
		if (text.find(":action-costs") != std::string::npos) {
			++action_cost_buf;
			continue;
		}
//		std::cout << "action text: " << text << std::endl;
		std::stringstream textstream(text);

		// action name
		std::vector<std::string> tokens;
		std::copy(std::istream_iterator<std::string>(textstream),
				std::istream_iterator<std::string>(), back_inserter(tokens));
		std::string actionname = tokens[2];
//		std::cout << "action name: " << actionname << std::endl;

		// parameters
		std::string parametersText = "";
		getBracketAfter(textstream, ":parameters", 0, parametersText);
//		std::cout << "param text" << parametersText << endl;

		//		parametersText = parametersText.substr(13);
//		getBracket(domain, ":parameters", actionNumber, parametersText);
		stringstream iss(parametersText);
		std::vector<std::pair<std::string, std::string>> param_text;
		std::vector<std::pair<std::string, int>> param;
//		std::cout << "read action " << endl;
//		std::cout << iss.str() << endl;
		getConstants(iss, param_text, "", true);

		for (int i = 0; i < param_text.size(); ++i) {
			param_text[i].first.erase(
					std::remove(param_text[i].first.begin(),
							param_text[i].first.end(), '('),
					param_text[i].first.end());
			if (!param_text[i].first.empty()) {
				pair<string, int> p(param_text[i].first,
						matchStringIndex(types, param_text[i].second));
				param.push_back(p);
			}
		}

//		cout << "params: " << endl;
//		for (int i = 0; i < param.size(); ++i) {
//			if (param[i].second >= 0) {
//				cout << param[i].first << " - " << types[param[i].second].first
//						<< endl;
//			} else {
//				cout << param[i].first << endl;
//			}
//		}

//		std::cout << parametersText << std::endl;
//		std::vector<std::string> parameters;
		std::vector<std::string> forward_delimeds; // = split(parametersText, '?');
//		for (int i = 1; i < forward_delimeds.size(); ++i) {
////			std::cout << "fd = " << forward_delimeds[i] << std::endl;
//			std::vector<std::string> token = split(forward_delimeds[i], ')');
//			parameters.push_back("?" + trim(token[0]));
//		}
//		std::cout << "parameters: ";
//		for (int i = 0; i < parameters.size(); ++i) {
//			std::cout << parameters[i];
//		}
//		std::cout << std::endl;

		// Read in lifted expression
		LiftedAction lf;
		lf.key = actionNumber;
		lf.n_arguments = param.size();
		lf.symbol = actionname;
		lf.param = param;

		std::string effectText = findRange(text, ":effect",
				"aaaaaaaaaaaaaaaaa");

		std::vector<std::string> effects;
		std::vector<std::pair<unsigned int, std::vector<unsigned int>>>addnums;
		std::vector<std::pair<unsigned int, std::vector<unsigned int>>>delnums;

		unsigned int action_cost = 1;
		if (this->action_costs) {
			action_cost = 0;
		}

		forward_delimeds = split(effectText, '(');
		for (int i = 1; i < forward_delimeds.size(); ++i) {
			//			std::cout << "fd = " << forward_delimeds[i] << std::endl;
			std::vector<std::string> token = split(forward_delimeds[i], ')');
			if (token[0].compare("and ") != 0) {
				effects.push_back(token[0]);
			}
			if (token[0].find("total-cost") != std::string::npos) {
//				std::cout << token[0] << " " << token[1] << std::endl;
				action_cost = atoi(token[1].c_str());
			}
		}

		bool isDel = false;
		bool isIncrease = false;
		for (int p = 0; p < effects.size(); ++p) {
			if (effects[p].find(" not ") != effects[p].npos) {
				isDel = true;
				continue;
			}
			// matches predicates to literal
			std::vector<std::string> lits = split(effects[p], ' ');
			for (int i = 0; i < lits.size(); ++i) {
				lits[i] = trim(lits[i]);
			}
//			cout << lits[0] << lits[1];
			for (int o = 0; o < ps.size(); ++o) {
				if (lits[0].compare(ps[o].symbol) == 0) {
//					cout << " matches " << ps[o].symbol << endl;
					std::pair<unsigned int, std::vector<unsigned int>> predicate;
					predicate.first = o;
					//　here we detect constants too.
					for (int i = 1; i < lits.size(); ++i) {
						int m = matchStringIndex(param, lits[i]);
						if (m >= 0) {
							predicate.second.push_back(m);
						} else {
							// object keys are given in parameters.size() + ob.
							// not sure how this is working.
							for (int ob = 0; ob < obs.size(); ++ob) {
								if (lits[i].compare(obs[ob].symbol) == 0) {
									predicate.second.push_back(
											param.size() + ob);
									break;
								}
							}
						}
					}
					if (isDel) {
						delnums.push_back(predicate);
					} else {
						addnums.push_back(predicate);
					}
					isDel = false;
					break;
				}
			}

		}

//		std::stringstream efst(effectText);
//		std::string inc;
//		getBracket2(efst, "increase", 0, inc);
//		std::cout << "inc: " << inc << std::endl;

		for (int as = 0; as < addnums.size(); ++as) {
			for (int ds = 0; ds < delnums.size(); ++ds) {
				if (addnums[as] == delnums[ds]) {
//					std::cout << "collision of effects!" << endl;
					delnums.erase(delnums.begin() + ds);
				}
			}
		}

		lf.adds = addnums;
		lf.dels = delnums;
		lf.action_cost = action_cost;

		// Preconditions
		effects.clear();
		std::stringstream t(text);
//		std::string precText;
//		getBracket(t, ":precondition", 0, precText);
//		getText(t, ":precondition", ":effect", 0, precText);
		std::string precText = findRange(text, ":precondition", ":effect");
		std::vector<std::pair<unsigned int, std::vector<unsigned int>>>precnums;
//		std::vector<std::pair<unsigned int, std::vector<unsigned int>>>typeprecnums;

		forward_delimeds = split(precText, '(');
		for (int i = 1; i < forward_delimeds.size(); ++i) {
			//			std::cout << "fd = " << forward_delimeds[i] << std::endl;
			std::vector<std::string> token = split(forward_delimeds[i], ')');
			if (token[0].compare("and ") != 0) {
				effects.push_back(token[0]);
			}
		}
//		for (int p = 0; p < effects.size(); ++p) {
//			std::cout << "LIFTED prec string = " << effects[p] << std::endl;
//		}

		for (int p = 0; p < effects.size(); ++p) {
			std::vector<std::string> lits = split(effects[p], ' ');
			for (int i = 0; i < lits.size(); ++i) {
				lits[i] = trim(lits[i]);
			}
//			cout << "preconditions: " << lits[0] << ", " << lits[1] << endl;
			for (int o = 0; o < ps.size(); ++o) {
				if (lits[0].compare(ps[o].symbol) == 0) {
//					cout << " matches " << ps[o].symbol << endl;
					std::pair<unsigned int, std::vector<unsigned int>> predicate;

					predicate.first = o;
					for (int i = 1; i < lits.size(); ++i) {
						int m = matchStringIndex(param, lits[i]);
						if (m >= 0) {
							predicate.second.push_back(m);
						} else {
							// object keys are given in parameters.size() + ob.
							for (int ob = 0; ob < obs.size(); ++ob) {
								if (lits[i].compare(obs[ob].symbol) == 0) {
									predicate.second.push_back(
											param.size() + ob);
//									std::cout << "const: " << lits[i] << ": "
//											<< ob << endl;
									break;
								}
							}

						}
					}
					precnums.push_back(predicate);
					break;
				}
			}

		}
		lf.precs = precnums;

//		for (int p = 0; p < precnums.size(); ++p) {
//			std::cout << "LIFTED prec num = " << precnums[p].first << std::endl;
//		}
//		for (int p = 0; p < addnums.size(); ++p) {
//			std::cout << "LIFTED add num = " << addnums[p].first << std::endl;
//		}

		lActions.push_back(lf);
//		std::cout << "instantiating " << lf.symbol << "/" << lf.n_arguments
//				<< ":";
//		for (int g = 0; g < param.size(); ++g) {
//			if (param[g].second >= 0) {
//				std::cout << " " << types[param[g].second].first;
//			}
//		}
//		std::cout << "..." << std::endl;
		// TODO: efficient way is to instantiate lifted actions.
		//       how can we do that?
//		cout << "actionNumber++" << endl;
		++actionNumber;
	}
//	cout << actionNumber << " lifted actions" << endl;
//	cout << gActionNumber << " grounded actions" << endl;

}

/**
 * From lifted actions, detect static predicates.
 * This method is to reduce the time of grounding actions.
 *
 * TODO: backchaining is the way to find all of feasible predicates.
 *       this method will be the prototype of that.
 */
void Strips::analyzeStaticLiftedPredicates() {
	// for all lifted predicates,
	//   if for all lifted actions,
	//       there is no actions to add the predicate.
	//   then the predicate is static.
//	bool isStatic

	// TODO: can optimize the loops.
	for (int i = 0; i < predicates.size(); ++i) {
		Predicate p = predicates[i];
		bool isStatic = true;
		for (int j = 0; j < lActions.size(); ++j) {
			LiftedAction lf = lActions[j];

			for (int k = 0; k < lf.adds.size(); ++k) {
				if (i == lf.adds[k].first) {
					isStatic = false;
					break;
				}
			}
		}
		predicates[i].isStatic = isStatic;
		if (predicates[i].isStatic) {
			cout << p.symbol << "/" << p.number_of_arguments << " is static."
					<< endl;
		}
	}
	cout << "done" << endl;

}

/**
 * PDDL format
 * parameters:
 * precondition:
 * effect:       add&delete effect
 *
 */
void Strips::groundAction(std::istream &domain, std::vector<Object> obs,
		std::vector<Predicate> ps, std::vector<GroundedPredicate> gs) {
	unsigned int gActionNumber = 0;

	for (int i = 0; i < lActions.size(); ++i) {
		LiftedAction lf = lActions[i];
		std::vector<std::pair<std::string, int>> param = lf.param;
		std::cout << "ground: " << lf.symbol << endl;

		// enumerate all objects matches argument typing.
		// TODO: check if the preconditions are feasible for the object.
		std::vector<std::vector<unsigned int> > possible_objs;
		possible_objs.resize(param.size());
		for (int p = 0; p < param.size(); ++p) {
			for (int o = 0; o < obs.size(); ++o) {
				if (isType(obs[o].type, param[p].second, types)) {
					possible_objs[p].push_back(o);
				}
			}
		}

		// TODO: here we check that the preconditions are feasible.
		// TODO: correct but far away from efficient.
		//
		// XXX: this algorithm is incorrect. it does not support predicates with 2 arity.
		//      ex. (successor ?a ?b) is hard to explain with this possible_obj method.
		//      -> checking static with 1 arity is a good way to reduce the search effect i guess.
		//         let's just check for 1 arity statics.
		//      freecell has many 2 arity statics. need to implement those to solve this domain.
		//
		// for all parameters,
		//   for all preconditions
		//     list all static preds which contains the parameter.
		//   if those are not in init state, then this object is unassignable.
//		for (int p = 0; p < possible_objs.size(); ++p) {
		for (int j = 0; j < lf.precs.size(); ++j) {
			unsigned int predicate_key = lf.precs[j].first;
			if (predicates[predicate_key].isStatic) {
				for (int k = 0; k < lf.precs[j].second.size(); ++k) {
					unsigned int argument = lf.precs[j].second[k];

					if (argument > possible_objs.size()) {
						// it is constant. then just assume it is feasible.
						continue;
					}
					// delete objects that does not have the predicate in init state.
					//					std::cout << "going to erase objs " << endl;
					//				std::cout << "argument " << argument << endl;
					//				std::cout << "possible_objs.size() = " << possible_objs.size() << endl;

					vector<bool> feasible_arg(possible_objs[argument].size(),
							false);

//					std::cout << predicates[predicate_key].symbol << ": ";
					for (int l = 0; l < init_state.size(); ++l) {
						GroundedPredicate g = g_predicates->at(init_state[l]);
//						vector<unsigned int> g_arg_to_action_arg = g.arguments;
//
//						for (int m = 0; m < g_arg_to_action_arg.size(); ++m) {
//							if (g_arg_to_action_arg[m] == argument) {
//								for (int n = 0; n < possible_objs[argument].size(); ++n) {
//									if () {
//
//									}
//								}
//							}
//						}
						// TODO: this argvalue is not working.
						int argvalue = g.argValue(predicate_key, k);
						if (argvalue >= 0) {
//							std::cout << g.symbol << "/" << argvalue << ", ";
							vector<unsigned int>::iterator it = find(
									possible_objs[argument].begin(),
									possible_objs[argument].end(), argvalue);
							unsigned int ps = it
									- possible_objs[argument].begin();
							feasible_arg[ps] = true;
						}
					}
//					std::cout << ": ";
					vector<unsigned int> new_possible_objs;
					for (int l = 0; l < feasible_arg.size(); ++l) {
						if (feasible_arg[l] == true) {
							new_possible_objs.push_back(
									possible_objs[argument][l]);
//							std::cout << possible_objs[argument][l] << " ";
						}
					}
//					std::cout << endl;
					possible_objs[argument] = new_possible_objs;

//					for (int pp = 0; pp < possible_objs[argument].size(); ++pp) {
//
//						// key
////						unsigned key = g_predicates_index[predicate_key] + possible_objs[argument][pp];
//
//						unsigned key = 0;
//						for (int k = 0; k < possible_objs.size(); ++k) {
//							key = key * obs.size() + possible_objs[k][pp];
//						}
//						key += g_predicates_index[prec.first];
//
//						if (find(init_state.begin(), init_state.end(), key)
//								== init_state.end()) {
//	//									std::cout << "erased obj" << endl;
//							possible_objs[argument].erase(
//									possible_objs[argument].begin() + pp);
//						}
//					}
				}

				// If
			}
		}
//		}

//		cout << "possible assignments: ";
//		for (int p = 0; p < possible_objs.size(); ++p) {
//			for (int pp = 0; pp < possible_objs[p].size(); ++pp) {
//				cout << obs[possible_objs[p][pp]].symbol << " ";
//			}
//		}
//		cout << endl;

		int param_max = 1; // pow(obs.size(), param.size());

		for (int p = 0; p < possible_objs.size(); ++p) {
			param_max *= possible_objs[p].size();
		}
//		std::cout << "param_max = " << param_max << std::endl;

		// TODO: This enumeration is not optimal as it enumerate then find if it matches typing.
		//       we can first check typing and then match typing.
		for (unsigned int pnum = 0; pnum < param_max; ++pnum) {
			std::vector<unsigned int> args;
			args.resize(param.size());
			// build arguments list.
//			bool valid_type = true;
			unsigned int p = pnum;
			for (int arg = param.size() - 1; arg >= 0; --arg) {
				unsigned int id = p % possible_objs[arg].size();
				args[arg] = obs[possible_objs[arg][id]].key;
				p /= possible_objs[arg].size();
//				unsigned int id = (pnum
//						/ pow(obs.size(), param.size() - 1 - arg)) % obs.size();
//				args[arg] = obs[possible_objs[arg][id]].key;
//				if (!isType(obs[id].type, param[arg].second, types)) {
//					valid_type = false;
//					break;
//				}
			}
//			if (!valid_type) {
////				std::cout << "invalid types" << endl;
//				continue;
//			}
//			std::cout << "valid types" << endl;

			std::vector<unsigned int> precnums;
			std::vector<unsigned int> addnums;
			std::vector<unsigned int> delnums;

			// TODO: These implementations can be optimized more.
			//       seems its closed to be able to parse freecell, but still slow.
			// TODO: build predicate table which return predicate key with lifted key and arguments.
			bool isFeasible = true;
			for (int i = 0; i < lf.precs.size(); ++i) {
				std::pair<unsigned int, std::vector<unsigned int>> prec =
						lf.precs[i];
				std::vector<unsigned int> prec_arg = getArguements(args,
						prec.second);
				unsigned key = 0;
				for (int k = 0; k < prec_arg.size(); ++k) {
					key = key * obs.size() + prec_arg[k];
				}
				key += g_predicates_index[prec.first];

				// If the predicate is static AND NOT feasible, this action is unfeasible.
				// If the predicate is static AND feasible, this action is feasible and this precondition is negligible.
				// If the predicate is not static, then push to precnums.
				if (predicates[prec.first].isStatic) {
					if (!isContainedSortedVectors(key, init_state)) {
//						std::cout << "unfeasible action. " << g_predicates->at(key).symbol << " unsat." << endl;
						isFeasible = false;
						break;
					}
				}
				if (!predicates[prec.first].isStatic) {
					precnums.push_back(key);
				}

//				for (int j = 0; j < g_predicates->size(); ++j) {
//					if (g_predicates->at(j).isEqual(prec.first, prec_arg)) {
//						precnums.push_back(g_predicates->at(j).key);
//						std::cout << "key = " << key << ":" << g_predicates->at(j).key << std::endl;
//						break;
//					}
//				}
			}
			if (!isFeasible) {
				continue;
			}

			for (int i = 0; i < lf.adds.size(); ++i) {
				std::pair<unsigned int, std::vector<unsigned int>> add =
						lf.adds[i];
				std::vector<unsigned int> add_arg = getArguements(args,
						add.second);
				unsigned key = 0;
				for (int k = 0; k < add_arg.size(); ++k) {
					key = key * obs.size() + add_arg[k];
				}
				key += g_predicates_index[add.first];
				addnums.push_back(key);
			}
			for (int i = 0; i < lf.dels.size(); ++i) {
				std::pair<unsigned int, std::vector<unsigned int>> del =
						lf.dels[i];
				std::vector<unsigned int> del_arg = getArguements(args,
						del.second);
				unsigned key = 0;
				for (int k = 0; k < del_arg.size(); ++k) {
					key = key * obs.size() + del_arg[k];
				}
				key += g_predicates_index[del.first];
				delnums.push_back(key);
			}
			std::string groundedName = lf.symbol;

			for (int i = 0; i < args.size(); ++i) {
				groundedName.append(" " + obs[args[i]].symbol);
			}

			std::sort(precnums.begin(), precnums.end());
			std::sort(addnums.begin(), addnums.end());
			std::sort(delnums.begin(), delnums.end());

			Action a(groundedName, gActionNumber, precnums, addnums, delnums);
			actionTable.addAction(a);
			++gActionNumber;

//			for (int p = 0; p < precnums.size(); ++p) {
//				std::cout << "prec num = " << precnums[p] << std::endl;
//			}
//			for (int p = 0; p < addnums.size(); ++p) {
//				std::cout << "add num = " << addnums[p] << std::endl;
//			}
//			for (int p = 0; p < delnums.size(); ++p) {
//				std::cout << "del num = " << delnums[p] << std::endl;
//			}
//			std::cout << "grounded name = " << groundedName << std::endl;

		}
	}
//	cout << actionNumber << " lifted actions" << endl;
//	cout << gActionNumber << " grounded actions" << endl;

}

void Strips::listFeasiblePredicates(std::vector<unsigned int>& gs) {
	for (int i = 0; i < init_state.size(); ++i) {
		gs.push_back(init_state[i]);
	}

	for (int a = 0; a < actionTable.getSize(); ++a) {
		Action act = actionTable.getAction(a);
		for (int p = 0; p < act.adds.size(); ++p) {
			gs.push_back(act.adds[p]);
		}
	}

	std::sort(gs.begin(), gs.end());
	gs.erase(unique(gs.begin(), gs.end()), gs.end());
}

// TODO: Check the validity of the predicates with backchaining.
/**
 * Backchaining:
 * 1. if the predicate in init_state, then true.
 * 2. if there is an action to add the predicate,
 * 3.   for all preconditions of the action, check those are all feasible. (recursive)
 * 4.   if all preconditions feasible, then true.
 * 5.   if not, then see other actions.
 * 6. if no action to add the predicate, then false.
 *
 */
void Strips::listFeasiblePredicates2(std::vector<unsigned int>& gs) {

	// lActions are already there.

	// -1: not calculated yet, 0: not feasible, 1: feasible
	vector<int> isFeasible(g_predicates->size(), -1);

	for (int i = 0; i < init_state.size(); ++i) {
		isFeasible[init_state[i]] = 1;
	}

	// TODO: backchaining
	//       seems like it is a difficult thing to implement.
	//       let's try out naive implementation first and see how it works.
	for (int i = 0; i < isFeasible.size(); ++i) {
		GroundedPredicate g = g_predicates->at(i);
		if (isFeasible[i] == -1) {
			for (int la = 0; la < lActions.size(); ++la) {
				vector<pair<unsigned int, vector<unsigned int> > > adds =
						lActions[la].adds;
				for (int ad = 0; ad < adds.size(); ++ad) {
					// XXX: second is not the object key. it is the arguments.
					if (g.isEqual(adds[ad].first, adds[ad].second)) {
						// if the action has add effect, check all preconditions are feasible.
						vector<pair<unsigned int, vector<unsigned int> > > precs =
								lActions[la].precs;
						for (int ps = 0; ps < precs.size(); ++ps) {

//							getArguements(precs[ps], lActions[la].param);
						}
					}
				}
			}
		}
	}

	for (int a = 0; a < actionTable.getSize(); ++a) {
		Action act = actionTable.getAction(a);
		for (int p = 0; p < act.adds.size(); ++p) {
			gs.push_back(act.adds[p]);
		}
	}

	std::sort(gs.begin(), gs.end());
	gs.erase(unique(gs.begin(), gs.end()), gs.end());
}

// When the action is feasible?
//  1. preconditions need to be feasible predicates.
//  2. add and delete effects have no predicates in common.
//  3.
void Strips::listFeasibleActions(std::vector<unsigned int> gs,
		std::vector<unsigned int>& actions) {
	for (int a = 0; a < actionTable.getSize(); ++a) {
		Action action = actionTable.getAction(a);
		bool isFeasible = true;

		//  1. preconditions need to be feasible predicates.
		for (int prec = 0; prec < action.preconditions.size(); ++prec) {
			if (std::find(gs.begin(), gs.end(), action.preconditions[prec])
					== gs.end()) {
				isFeasible = false;
				break;
			}
		}
		if (!isFeasible) {
			continue;
		}

		//  2. add and delete effects have no predicates in common.
		unsigned int intersection = howManyContainedSortedVectors(action.adds,
				action.deletes);
		if (intersection == 0) {
			actions.push_back(a);
			continue;
		} else {
//			std::cout << "contradictory action: " << "(" << action.name << ")"
//					<< std::endl;
		}

	}
}

void Strips::listFeasiblePredicatesWithActions(std::vector<unsigned int>& gs,
		std::vector<unsigned int>& actions) {
// 1. add all predicates in initial state.
// 2. for all actions in action trie, add all predicates listed in all add effects.
	gs.clear();
	for (int i = 0; i < init_state.size(); ++i) {
		gs.push_back(init_state[i]);
	}

	for (int a = 0; a < actions.size(); ++a) {
		Action act = actionTable.getAction(actions[a]);
		for (int p = 0; p < act.adds.size(); ++p) {
			gs.push_back(act.adds[p]);
		}
	}

	std::sort(gs.begin(), gs.end());
	gs.erase(unique(gs.begin(), gs.end()), gs.end());
}

void Strips::buildActionTrie(std::vector<unsigned> keys) {
	actionTrie.setNPredicates(g_predicates->size());
	std::vector<std::pair<std::vector<unsigned int>, unsigned int> > precs;
	for (int a = 0; a < keys.size(); ++a) {
//		std::cout << a << std::endl;
		Action action = actionTable.getAction(keys[a]);
//		action.print();
		std::pair<std::vector<unsigned int>, unsigned int> p(
				action.preconditions, keys[a]);
		precs.push_back(p);
	}
	actionTrie.buildTrie(precs);
}

/**
 * Greedy algorithm to find balances.
 *
 * TODO: this algorithm is too slow to solve some domains (e.g. airport).
 *       1. get rid of static predicates.
 *
 */
std::vector<unsigned int> Strips::analyzeBalance(unsigned int p,
		const std::vector<unsigned int>& predicates,
		const std::vector<LiftedActionArg>& laas) {

	std::string prefix((predicates.size() - 1) * 4, ' ');

	if (predicates.size() == predargs.size()) {
//		std::cout << prefix << "failed" << std::endl;
		return std::vector<unsigned int>();
	}

	if (predicates.size() > 4) {
		return std::vector<unsigned int>();
	}

// list all actions which adds p.
// TODO: would this be predicates not p? let's try it.
	std::vector<LiftedActionArg> actions_add_p;
	for (int a = 0; a < laas.size(); ++a) {
		std::vector<unsigned int> adds = laas[a].lf.addsInst;
		for (int ps = 0; ps < predicates.size(); ++ps) {
			if (find(adds.begin(), adds.end(), predicates[ps]) != adds.end()) {
				// found
				actions_add_p.push_back(laas[a]);
				break;
			}
		}
	}

//	std::cout << prefix << actions_add_p.size() << " actions: ";
//	for (int a = 0; a < actions_add_p.size(); ++a) {
//		std::cout << "(" << actions_add_p[a].lf.symbol << "-"
//				<< actions_add_p[a].instantiated_arg << ") ";
//	}
//	std::cout << std::endl;

// Check if ALL actions should hold the balance.
	bool allActionsValid = true;
	for (int a = 0; a < actions_add_p.size(); ++a) {
		// check delete effect that contains the argument referred as lfa.instantiated_arg
		std::vector<unsigned int> dels = actions_add_p[a].lf.delsInst;
		if (howManyContainedSortedVectors(dels, predicates) != 1) {
			allActionsValid = false;
			break;
		}
	}

// if all actions valid, then the set "predicates" are balanced.
	if (allActionsValid) {
		// Here, we got the return we really want to.
//		std::cout << prefix << "  group found!: ";
		for (int g = 0; g < predicates.size(); ++g) {
			if (predargs[predicates[g]].group_key != -1) {
//				std::cout << prefix << "  reserved" << std::endl;
				return predicates;
			}
		}
		// if all the group_keys == -1, then we can make new group
		for (int g = 0; g < predicates.size(); ++g) {
			predargs[predicates[g]].group_key = n_groups;
		}
		++n_groups;
//		for (int g = 0; g < predicates.size(); ++g) {
//			std::cout << "(" << predargs[predicates[g]].pred.symbol << "-"
//					<< predargs[predicates[g]].instantiated_arg << ") ";
//		}
//
//		std::cout << std::endl;
		return predicates;
	}

	std::vector<std::vector<unsigned int> > ret_preds;
	ret_preds.resize(actions_add_p.size());
	for (int a = 0; a < actions_add_p.size(); ++a) {
		// check delete effect that contains the argument referred as lfa.instantiated_arg
		std::vector<unsigned int> dels = actions_add_p[a].lf.delsInst;
//		std::cout << prefix << "  delsInst: ";
//		for (int g = 0; g < dels.size(); ++g) {
//			std::cout << "(" << predargs[dels[g]].pred.symbol << "-" << predargs[dels[g]].instantiated_arg << ") ";
//		}
//		std::cout << std::endl;

		if (howManyContainedSortedVectors(dels, predicates) != 1) {
			std::vector<unsigned int> difs;
//			std::set_difference(dels.begin(), dels.end(), predicates.begin(),
//					predicates.end(), std::back_inserter(difs));

			difs = differenceSortedVectors(dels, predicates);

//			std::cout << prefix << "  difs: ";
//			for (int g = 0; g < difs.size(); ++g) {
//				std::cout << "(" << predargs[difs[g]].pred.symbol << "-" << predargs[difs[g]].instantiated_arg << ") ";
//			}
//			std::cout << std::endl;

			std::vector<unsigned int> children;
			children.resize(difs.size());
			// difs are delete effect - predicates
			for (int df = 0; df < difs.size(); ++df) {
				std::vector<unsigned int> args = predicates;
				// df
				args.push_back(difs[df]);
				std::sort(args.begin(), args.end());

//				std::vector<unsigned int> args = uniquelyMergeSortedVectors2(predicates, difs[df]);

//				std::cout << prefix << "  ";
//				for (int g = 0; g < args.size(); ++g) {
//					std::cout << "(" << predargs[args[g]].pred.symbol << "-"
//							<< predargs[args[g]].instantiated_arg << ") ";
//				}
//				std::cout << std::endl;

				std::vector<unsigned int> c = analyzeBalance(difs[df], args,
						laas);
				// TODO: should be smallest?
				if (c.size() > children.size()) {
					children = c;
				}
			}
			ret_preds[a] = children;
		} else {
			ret_preds[a] = predicates;
		}
	}

	std::vector<unsigned int> ret;
	for (int r = 0; r < ret_preds.size(); ++r) {
		if (ret_preds[r].size() == 0) {
			// failed to find balance in any of the action.
//			std::cout << prefix << "  failed" << std::endl;
//			return ret_preds[r];
		} else {
			uniquelyMergeSortedVectors(ret, ret_preds[r]);
		}
	}
// 1. find all actions that adds the predicates.
// 2.    for all actions found, list all delete effects.
// 3.       for all delete effects, do analyzeBalance().
	return ret;
}

// TODO: analyze predicates in goal conditions first.
void Strips::analyzeAllBalances(std::vector<Predicate> ps) {
//	for (int i = 0; i < lActions.size(); ++i) {
//		std::cout << lActions[i].symbol << std::endl;
//		for (int a = 0; a < lActions[i].adds.size(); ++a) {
//			std::cout << lActions[i].adds[a].first << " ";
//			std::cout << "(" << ps[lActions[i].adds[a].first].symbol;
//			for (int arg = 0; arg < lActions[i].adds[a].second.size(); ++arg) {
//				std::cout << " " << lActions[i].adds[a].second[arg];
//			}
//			std::cout << ")";
//			std::cout << std::endl;
//		}
//	}

//	return;
//	std::vector<PredicateArg> predargs;
	unsigned int key = 0;
	for (int i = 0; i < ps.size(); ++i) {
		if (!ps[i].isStatic) {
			for (int arg = 0; arg < ps[i].number_of_arguments; ++arg) {
				PredicateArg paa;
				paa.key = key++;
				paa.pred = ps[i];
				paa.instantiated_arg = arg;
				paa.group_key = -1;

				paa.isInGoal = false;
				for (int j = 0; j < goal_condition.size(); ++j) {
					if (paa.matchesLiftedKey(
							g_predicates->at(goal_condition[j]))) {
						paa.isInGoal = true;
						break;
					}
				}

				predargs.push_back(paa);
			}
		}
	}

//	std::cout << "PredicateArgs" << std::endl;
//	for (int i = 0; i < predargs.size(); ++i) {
//		if (predargs[i].group_key == -1) {
//			std::vector<PredicateArg> pss;
//			pss.push_back(predargs[i]);
//			std::cout << predargs[i].pred.symbol << "-"
//					<< predargs[i].instantiated_arg << std::endl
//		}
//	}

	std::vector<LiftedActionArg> laas;
	key = 0;
	for (int i = 0; i < lActions.size(); ++i) {
		for (int arg = 0; arg < lActions[i].n_arguments; ++arg) {
			LiftedActionArg laa;
			laa.lf = lActions[i];
			laa.instantiated_arg = arg;
			laa.key = key++;

			for (int as = 0; as < lActions[i].adds.size(); ++as) {
				std::vector<unsigned int> args = lActions[i].adds[as].second;
				std::vector<unsigned int>::iterator it = std::find(args.begin(),
						args.end(), arg);
				if (it == args.end()) {
					continue;
				} else {
					for (int p = 0; p < predargs.size(); ++p) {
						if (predargs[p].isEqual(lActions[i].adds[as].first,
								it - args.begin())) {
							laa.lf.addsInst.push_back(p);
							break;
						}
					}
					std::sort(laa.lf.addsInst.begin(), laa.lf.addsInst.end());
				}
			}
			for (int as = 0; as < lActions[i].dels.size(); ++as) {
				std::vector<unsigned int> args = lActions[i].dels[as].second;
				std::vector<unsigned int>::iterator it = std::find(args.begin(),
						args.end(), arg);
				if (it == args.end()) {
					continue;
				} else {
					for (int p = 0; p < predargs.size(); ++p) {
						if (predargs[p].isEqual(lActions[i].dels[as].first,
								it - args.begin())) {
							laa.lf.delsInst.push_back(p);
						}
					}
					std::sort(laa.lf.delsInst.begin(), laa.lf.delsInst.end());
				}
			}
			laas.push_back(laa);
		}
	}

//	std::cout << "LiftedActionArgs & adds" << std::endl;
//	for (int i = 0; i < laas.size(); ++i) {
//		std::cout << laas[i].lf.symbol << "-" << laas[i].instantiated_arg
//				<< std::endl;
//		std::cout << "adds: ";
//		for (int a = 0; a < laas[i].lf.addsInst.size(); ++a) {
//			std::cout << "(" << predargs[laas[i].lf.addsInst[a]].pred.symbol
//					<< "-" << predargs[laas[i].lf.addsInst[a]].instantiated_arg
//					<< ") ";
//		}
//
//		std::cout << std::endl << "dels: ";
//		for (int a = 0; a < laas[i].lf.delsInst.size(); ++a) {
//			std::cout << "(" << predargs[laas[i].lf.delsInst[a]].pred.symbol
//					<< "-" << predargs[laas[i].lf.delsInst[a]].instantiated_arg
//					<< ") ";
//		}
//		std::cout << std::endl;
//	}

	std::vector<std::vector<unsigned int>> groups;
//	std::cout << "PredicateArgs" << std::endl;

	// check predargs in goal conditions first.
	// then check other args later.

	for (int i = 0; i < predargs.size(); ++i) {
		if (predargs[i].isInGoal) {
			if (predargs[i].group_key == -1) {
				std::vector<unsigned int> pss;
				pss.push_back(predargs[i].key);
				std::cout << predargs[i].pred.symbol << "/"
						<< predargs[i].instantiated_arg << std::endl;
				groups.push_back(analyzeBalance(predargs[i].key, pss, laas));
			}
		}
	}

	for (int i = 0; i < predargs.size(); ++i) {
		if (predargs[i].group_key == -1) {
			std::vector<unsigned int> pss;
			pss.push_back(predargs[i].key);
			std::cout << predargs[i].pred.symbol << "/"
					<< predargs[i].instantiated_arg << std::endl;
			groups.push_back(analyzeBalance(predargs[i].key, pss, laas));
		}
	}

//	for (int i = 0; i < predargs.size(); ++i) {
//		std::cout << "(" << predargs[i].pred.symbol << "-"
//				<< predargs[i].instantiated_arg << "):" << predargs[i].group_key
//				<< " ";
//		std::cout << std::endl;
//	}

//	buildPDB();

}

void Strips::analyzeXORGroups() {
	for (int g = 0; g < n_groups; ++g) {
		std::vector<unsigned int> preds;
		// list predicates in group.
		for (int p = 0; p < predargs.size(); ++p) {
			if (predargs[p].group_key == g) {
				preds.push_back(p);
			}
		}
		// instantiate predicates for all objects.
		// HERE, in this iteration, we build one abstraction.
		for (int obj = 0; obj < objects.size(); ++obj) {
			std::vector<unsigned int> g_preds;
			for (int p = 0; p < preds.size(); ++p) {
				for (int g = 0; g < g_predicates->size(); ++g) {
					if (predargs[preds[p]].matches(g_predicates->at(g), obj)) {
						if (find(g_feasible_predicates.begin(),
								g_feasible_predicates.end(), g)
								!= g_feasible_predicates.end()) {
							g_preds.push_back(g);
						}
					}
				}
			}
			if (g_preds.size() > 1) {
				xor_groups.push_back(g_preds);
			}
		}
	}
	std::cout << "XORing groups for Strucuterd Zobrist" << std::endl;
	for (int g = 0; g < xor_groups.size(); ++g) {
		std::cout << g << " group: ";
		for (int p = 0; p < xor_groups[g].size(); ++p) {
			std::cout << "(" << g_predicates->at(xor_groups[g][p]).symbol
					<< ") ";
		}
		std::cout << std::endl;
	}

}

void Strips::analyzeTransitions() {

/////////////////////////////////////
/// Transition analysis
/////////////////////////////////////
	std::vector<std::vector<std::vector<unsigned int> > > xor_groups_transitions;
	xor_groups_transitions.resize(xor_groups.size());
	for (int g = 0; g < xor_groups.size(); ++g) {
		xor_groups_transitions[g].resize(xor_groups[g].size());
	}
// find all actions to delete the predicate.
// for all that actions, find the replacing group predicate.
// list up transitions.

	for (int g = 0; g < xor_groups.size(); ++g) {
		for (int p = 0; p < xor_groups[g].size(); ++p) {
			std::vector<Action> actions = actionTable.getActionsWhichDeletes(
					xor_groups[g][p]);
			std::vector<unsigned int> transitions;
			for (int a = 0; a < actions.size(); ++a) {
				std::vector<unsigned int> adds = actions[a].adds;
				// a bit of duplicate.
				std::vector<unsigned int> t = intersectingSortedVectors(
						xor_groups[g], adds);
				std::vector<unsigned int> buf = uniquelyMergeSortedVectors(
						transitions, t);
				transitions = buf;
			}
			xor_groups_transitions[g][p] = transitions;
		}
	}

	std::cout << "Transition groups for Strucuterd Zobrist" << std::endl;
	for (int g = 0; g < xor_groups_transitions.size(); ++g) {
		std::cout << g << " group: ";
		for (int p = 0; p < xor_groups_transitions[g].size(); ++p) {
			std::cout << xor_groups[g][p] << " -> ";
			for (int t = 0; t < xor_groups_transitions[g][p].size(); ++t) {
				std::cout << xor_groups_transitions[g][p][t] << " ";
			}
			std::cout << std::endl;
		}
	}

// TODO: this algorithm can be optimized in many ways.
//       for now this is just a prototype, going to be improved.

// TODO: How to build structure?
	/**
	 * 1. divide xor groups into two structures. (prototype, is not successful.)
	 * 2.
	 *
	 */
/////////////////////////////////////
/// Build Structures for SZ
/////////////////////////////////////
// 1. find the node with least edges and add that to group 0.
// 2. add a node which is mostly connected to group 0.
// 3. do it until the size of group 0 reaches n/2.
	std::cout << "building structure..." << std::endl;
	for (int gs = 0; gs < xor_groups_transitions.size(); ++gs) {
		// 1. find the most isolated node.
		std::vector<unsigned int> structure;
		std::vector<unsigned int> structure_index;
		std::vector<unsigned int> transitions;
		for (int p = 0; p < xor_groups_transitions[gs].size(); ++p) {
			transitions.insert(transitions.end(),
					xor_groups_transitions[gs][p].begin(),
					xor_groups_transitions[gs][p].end());
		}
//		std::cout << "transition" << std::endl;
		unsigned int least_connected = 100000;
		unsigned int least_connected_node = 0; // in index
		for (int p = 0; p < xor_groups[gs].size(); ++p) {
			int c = std::count(transitions.begin(), transitions.end(),
					xor_groups[gs][p]);
			c += xor_groups_transitions[gs][p].size();
//			std::cout << xor_groups[gs][p] << ": " << c << "edges" << std::endl;
			if (c < least_connected) {
				least_connected = c;
				least_connected_node = p;
			}
		}
		structure.push_back(xor_groups[gs][least_connected_node]);
		structure_index.push_back(least_connected_node);

		while (structure.size() < xor_groups[gs].size() / 2) {
			transitions.clear();
			// 2. add a node which is mostly connected to strucuture
			for (int p = 0; p < structure_index.size(); ++p) {
				transitions.insert(transitions.end(),
						xor_groups_transitions[gs][p].begin(),
						xor_groups_transitions[gs][p].end());
			}
			// count the most connected nodes
			unsigned int most_connected = 0;
			unsigned int most_connected_node = 0; // in index
			for (int p = 0; p < xor_groups[gs].size(); ++p) {
				// if p is already in the group, then skip that.
				if (find(structure_index.begin(), structure_index.end(), p)
						!= structure_index.end()) {
					continue;
				}
				int c = std::count(transitions.begin(), transitions.end(),
						xor_groups[gs][p]);
				c += xor_groups_transitions[gs][p].size();
				if (c > most_connected) {
					most_connected = c;
					most_connected_node = p;
				}
			}
			structure.push_back(xor_groups[gs][most_connected_node]);
			structure_index.push_back(most_connected_node);
		}
		std::sort(structure.begin(), structure.end());
		structures.push_back(structure);

		// the rest of the predicates will be in the second structure.
		// TODO: not sure this assumption is right or not.
		//       maybe not right.
		std::vector<unsigned int> structure2; // = xor_groups[gs] - structure;
		std::set_difference(xor_groups[gs].begin(), xor_groups[gs].end(),
				structure.begin(), structure.end(),
				std::inserter(structure2, structure2.begin()));
		std::sort(structure2.begin(), structure2.end());
		structures.push_back(structure2);
	}

	std::cout << "structures" << std::endl;
	for (int s = 0; s < structures.size(); ++s) {
		for (int ps = 0; ps < structures[s].size(); ++ps) {
			std::cout << structures[s][ps] << " ";
		}
		std::cout << std::endl;
	}

/////////////////////////////////////
// list of ungroupeds.
/////////////////////////////////////

	for (int p = 0; p < g_feasible_predicates.size(); ++p) {
		bool grouped = false;
		for (int gs = 0; gs < xor_groups.size(); ++gs) {
			for (int m = 0; m < xor_groups[gs].size(); ++m) {
				if (xor_groups[gs][m] == g_feasible_predicates[p]) {
					grouped = true;
					break;
				}
			}
			if (grouped) {
				break;
			}
		}
		if (!grouped) {
			xor_ungroupeds.push_back(g_feasible_predicates[p]);
		}
	}

	std::sort(ungroupeds.begin(), ungroupeds.end());

	std::cout << "xor_ungroupeds = ";
	for (int p = 0; p < xor_ungroupeds.size(); ++p) {
		std::cout << "(" << g_predicates->at(xor_ungroupeds[p]).symbol << ") ";
	}
	std::cout << std::endl;

}

/**
 * parse constants.
 *
 * WRITE: domain:predicates, instance:object, instance:init
 * TODO: implement polymorphic types with tree structure.
 */
void Strips::getConstants(std::istream& domain,
		std::vector<std::pair<std::string, std::string>>& type_object,
		std::string header, bool whole_text) {
	std::vector<std::pair<std::string, std::string>> constants;
	std::string text;
	if (whole_text) {
		text = gulp(domain);
	} else {
		getBracket2(domain, header, 0, text);
	}
//	cout << "getConstants: " << text << endl;
//	text = text.substr(1);
//	vector<string> undelimed = split(text, ' ');

	std::string deliminator = "-";
	std::vector<std::string> cur_objects;
	bool reading_object = true;

	std::istringstream iss(text);
	std::vector<std::string> tokens {
			std::istream_iterator<std::string> { iss }, std::istream_iterator<
					std::string> { } };

	bool hasTypingInfo = (text.find(deliminator) != text.npos);
	if (hasTypingInfo) {
		hasTypingInfo = false;
		for (int i = 0; i < tokens.size(); ++i) {
			if (tokens[i].compare(deliminator) == 0) {
				hasTypingInfo = true;
				break;
			}
		}
	}

// If no typing information there
	if (!hasTypingInfo) {
		for (int i = 1; i < tokens.size(); ++i) {
			std::vector<std::string> lits = split(tokens[i], ')');
			if (lits.size() == 0) {
				continue;
			}
			std::string lit = lits[0];
			std::pair<std::string, std::string> p(lit, "");
			constants.push_back(p);
		}
		type_object = constants;
		return;
	}

//////////////////////////////
/// Objects with typing
//////////////////////////////
	for (int i = 1; i < tokens.size(); ++i) {
		std::vector<std::string> lits = split(tokens[i], ')');
		if (lits.size() == 0) {
//			cout << "no literal" << endl;
			continue;
		}
		std::string lit = lits[0];
//		cout << "lit = " << lit;
		if (reading_object) {
			if (lit.compare(deliminator) != 0) {
//				cout << "obj" << endl;
				// reading objects
				cur_objects.push_back(lit);
			} else {
//				cout << "dlm" << endl;
				// deliminator
				reading_object = false;
			}
		} else {
//			cout << "type" << endl;

			// if reading types
			for (int obs = 0; obs < cur_objects.size(); ++obs) {
				std::pair<std::string, std::string> p(cur_objects[obs], lit);
				constants.push_back(p);
			}
			cur_objects.clear();
			reading_object = true;
		}
	}
	type_object = constants;
}

unsigned int Strips::groundedPredicateKey(
		const std::pair<unsigned int, std::vector<unsigned int> >& pred) {
	unsigned key = 0;
	for (int k = 0; k < pred.second.size(); ++k) {
		key = key * this->objects.size() + pred.second[k];
	}
	key += g_predicates_index[pred.first];
	return key;
}

int Strips::pow(int base, int p) {
	int ret = 1;
	for (int i = 0; i < p; ++i) {
		ret *= base;
	}
	return ret;
}

const std::vector<std::vector<unsigned int>> Strips::get_structures() {
	return structures;
}

//////////////////////////////////////////////////////
/// print
//////////////////////////////////////////////////////

void Strips::print_state(const std::vector<unsigned int>& propositions) const {
	std::cout << "state: ";
	for (int p = 0; p < propositions.size(); ++p) {
		std::cout << "(" << g_predicates->at(propositions[p]).symbol << ") ";
	}
	std::cout << std::endl;
}

void Strips::print_state(const State& s) const {
	print_state(s.propositions);
}

void Strips::print_plan(std::vector<State>& path) const {
	if (path.size() == 0) {
		std::cout << "failed to find a plan." << std::endl;
		return;
	}
//	print_state(path[path.size() - 1].propositions);
	for (int i = path.size() - 1; i >= 0; --i) {
		print_state(path[i].propositions);
	}
}

