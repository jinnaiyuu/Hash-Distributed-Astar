// Copyright 2012 Ethan Burns. All rights reserved.
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file.
#include "strips.hpp"
#include "action.hpp"
#include "utils.hpp"
#include "dijkstra.hpp"
#include "../astar.hpp"
#include "../astar_heap.hpp"
#include "../search.hpp"

#include <string>
#include <iostream>
#include <algorithm>
#include <iterator>

// spirit
//#include <boost/config/warning_disable.hpp>
//#include <boost/spirit/include/qi.hpp>
//#include <boost/spirit/include/phoenix_core.hpp>
//#include <boost/spirit/include/phoenix_operator.hpp>
//#include <boost/spirit/include/phoenix_object.hpp>
//#include <boost/spirit/include/phoenix_stl.hpp>
//#include <boost/fusion/include/adapt_struct.hpp>
//#include <boost/fusion/include/io.hpp>
//
//namespace qi = boost::spirit::qi;
//namespace ascii = boost::spirit::ascii;
//namespace phoenix = boost::phoenix;
//using qi::int_;
//using qi::lit;
//using qi::_val;
//using qi::_1;
//using qi::eps;
//using qi::alpha;
//using qi::alnum;
//using qi::double_;
//using qi::lexeme;
//using qi::phrase_parse;
//using ascii::char_;
//using boost::spirit::ascii::space;

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
	const unsigned int TRUE_PREDICATE = 10000000;
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

		unsigned int rest = 1
				<< (goal_condition.size() - pdb_groups.size() - 1);

		if (size * xor_groups[g].size() * rest > 100000) {
			full = true;
			continue;
		}
		pdb_groups.push_back(xor_groups[g]);
		size *= xor_groups[g].size();
		std::cout << "size = " << size << std::endl;
	}

// TODO: for all goal_conditions which is out of groups,
//       add TRUE OR FALSE group to the patterns.
	for (int c = 0; c < goal_condition.size(); ++c) {
//		if (size * 2 > 500000) {
//			break;
//		}

		bool is_grouped = false;
		for (int gs = 0; gs < pdb_groups.size(); ++gs) {
			if (find(pdb_groups[gs].begin(), pdb_groups[gs].end(),
					goal_condition[c]) != pdb_groups[gs].end()) {
				is_grouped = true;
				break;
			}
		}

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

//		std::cout << "pat = ";
		for (int a = 0; a < args_in_groups.size(); ++a) {
//			std::cout << "(" << g_predicates->at(groups[a][args_in_groups[a]]).symbol << ") ";
			args_in_preds.push_back(pdb_groups[a][args_in_groups[a]]);
		}
		std::sort(args_in_preds.begin(), args_in_preds.end());

		all_patterns_preds.push_back(args_in_preds);
	}

// TODO: here, we got init state as args_in_preds.
//       we need to run search from this init state to the goal state, or the other way around.
//       efficient way is to trace back from the goal state. how do i do that?
// pattern: abstract

	dijkstra(all_patterns_preds, pdb_groups);
// input: patterns, ungroupeds, simplified actions,

//	pd.addPattern()

	pd->setGroups(pdb_groups);

	pd->dump_all(name);

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

	search->search(init, 60.0 * 1);
//	print_plan(path);

	std::vector<unsigned int> costs = search->get_costs();

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
Strips::Strips(std::istream & domain, std::istream & instance) {
	domain.seekg(0, std::ios_base::beg);
	instance.seekg(0, std::ios_base::beg);

	std::vector<Predicate> predicates; // uninstantiated, ungrounded
//	std::vector<Object> objects;

//	std::vector<unsigned int> g_feasible_predicates;

//	std::vector < Action >

	readDomainName(domain);
	readInstanceName(instance);

	readRequirements(domain);

// TODO: if (:requirements :typing) then
// TODO: learning types needed.
	readTypes(domain, predicates);

// 1. learn predicates from domain.pddl
	std::cout << "parsing predicates..." << std::endl;
	readPredicates(domain, predicates);
	std::cout << "generated " << predicates.size() << " lifted predicates."
			<< std::endl;

// 2. learn objects instance.pddl.
	std::cout << "parsing objects..." << std::endl;
	readObjects(instance);
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

	std::cout << "generated " << actionTable.getSize() << " grounded actions."
			<< std::endl;
//	actionTable.printAllActions();

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

//	actionTrie.printTree();

	std::vector<unsigned int> init_actions = actionTrie.searchPossibleActions(
			init_state);

	std::cout << "init state: ";
	for (int i = 0; i < init_state.size(); ++i) {
		std::cout << "(" << g_predicates->at(init_state[i]).symbol << ") ";
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

//BOOST_FUSION_ADAPT_STRUCT( Strips::Predicate,
//		(std::string, symbol) (unsigned int, number_of_arguments))
//
//template<typename Iterator>
//struct predicate_parser: qi::grammar<Iterator, Strips::Predicate(),
//		ascii::space_type> {
//	predicate_parser() :
//			predicate_parser::base_type(start) {
//		symbol %= lexeme[+(alnum)];
//		args = eps[_val = 0] >> *(lexeme['?' >> +(alnum)])[_val += 1];
//		start %= '(' >> symbol >> args >> ')';
//	}
//	qi::rule<Iterator, std::string(), ascii::space_type> symbol;
//	qi::rule<Iterator, unsigned(), ascii::space_type> args;
//	qi::rule<Iterator, Strips::Predicate(), ascii::space_type> start;
//};

void Strips::readDomainName(std::istream &domain) {
	domain.seekg(0, std::ios_base::beg);
	std::string text;
	getBracket(domain, "domain", 0, text);
	std::vector<std::string> forward_delimeds = split(text, ' ');
	std::vector<std::string> backward_delimeds = split(forward_delimeds[2],
			')');
	domain_ = backward_delimeds[0];
	std::cout << "domain name = " << domain_ << std::endl;
}

void Strips::readInstanceName(std::istream &instance) {
	instance.seekg(0, std::ios_base::beg);
	std::string text;
	getBracket(instance, "problem", 0, text);
	std::vector<std::string> forward_delimeds = split(text, ' ');
	if (forward_delimeds.size() < 3) {
		std::cout << "instance name broken" << std::endl;
		std::cout << "text = " << text;

		instance_ = "";
		return;
	}
	std::vector<std::string> backward_delimeds = split(forward_delimeds[2],
			')');
	instance_ = backward_delimeds[0];
	std::cout << "instance name = " << instance_ << std::endl;
}

void Strips::readRequirements(std::istream &domain) {
	domain.seekg(0, std::ios_base::beg);
	std::string text;
	getBracket(domain, ":requirements", 0, text);
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
	getText(domain, ":predicates", ":action", 0, text);
	replace(text, "(:predicates", "");
//	std::cout << "predicates text = " << text << std::endl;

//	predicate_parser<std::string::const_iterator> g;

	std::vector<std::string> symbols;
	std::vector<unsigned int> argcs;
	std::vector<std::string> forward_delimeds = split(text, '(');
	for (int i = 1; i < forward_delimeds.size(); ++i) {
		std::vector<std::string> predicate = split(forward_delimeds[i], ')');
		std::vector<std::string> token = split(predicate[0], ' ');

		symbols.push_back(token[0]);
		argcs.push_back(token.size() - 1);
	}

	predicates.resize(symbols.size());
	for (int i = 0; i < predicates.size(); ++i) {
		predicates[i].key = i;
		predicates[i].symbol = symbols[i];
		predicates[i].number_of_arguments = argcs[i];
	}

	for (int i = 0; i < predicates.size(); ++i) {
		std::cout << predicates[i].key << " " << predicates[i].symbol << "/"
				<< predicates[i].number_of_arguments << std::endl;
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
void Strips::readTypes(std::istream &domain,
		std::vector<Predicate>& predicates) {
	if (!typing) {
		return;
	}
	domain.seekg(0, std::ios_base::beg);

	std::string text;
	getBracket(domain, ":types", 0, text);
	std::cout << "types = " << text << std::endl;
// structure
// type type type - type

}

void Strips::readObjects(std::istream &instance) {
	std::vector<std::string> strings;
	std::string text;
	getText(instance, ":objects", ":init", 0, text);

	std::vector<std::string> tokens;
	std::istringstream iss(text);
	std::copy(std::istream_iterator<std::string>(iss),
			std::istream_iterator<std::string>(), back_inserter(tokens));

	for (int i = 1; i < tokens.size() - 1; ++i) {
		strings.push_back(tokens[i]);
	}
//	std::cout << "strings" << std::endl;
//	objects.resize(strings.size());
	for (int i = 0; i < strings.size(); ++i) {
		std::cout << strings[i] << " ";
		Object obj;
		obj.key = i;
		obj.symbol = strings[i];
		objects.push_back(obj);
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
	std::string line;
	std::string init_ = ":init";
	std::string goal_ = ":goal";
	size_t pos;
	std::vector<std::string> strings;

// put total_text the text for inital state.
	while (instance.good()) {
		getline(instance, line); // get line from file
		std::transform(line.begin(), line.end(), line.begin(), ::tolower);
		pos = line.find(init_); // search
		if (pos != std::string::npos) {
//			std::cout << "Found: " << line << std::endl;
			total_text = line;
			while (instance.good()) {
				getline(instance, line); // get line from file
				std::transform(line.begin(), line.end(), line.begin(),
						::tolower);
				pos = line.find(goal_); // search
				if (pos != std::string::npos) {
					break;
				} else {
					total_text.append(line);
				}
			}
			break;
		}
	}

// parse the total_text.
//	std::cout << "inital state: " << total_text << std::endl;

	std::vector<std::string> symbols; // parse into here.

// now we need to parse total text into vector of strings(symbols).
	std::vector<std::string> forward_delimeds = split(total_text, '(');
	for (int i = 2; i < forward_delimeds.size(); ++i) {
		std::vector<std::string> token = split(forward_delimeds[i], ')');

		symbols.push_back(token[0]);
	}

	for (int i = 0; i < symbols.size(); ++i) {
//		std::cout << i << ": " << symbols[i];

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
	getText(instance, ":goal", ")))", 0, text);
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
// 1. read parameters.
// 2. read precondition (lifted).
// 3. read add&delete effect (lifted).
// 4. assign objects for parameters and ground preconditions & effects.
// 5. build Action.
	std::vector<std::string> parameters;

	std::string text;

	unsigned int actionNumber = 0;
	unsigned int gActionNumber = 0;
	while (getText(domain, ":action", ":action", actionNumber, text)) {
//		std::cout << "action-> " << text << std::endl;
		std::stringstream textstream(text);

		// action name
		std::vector<std::string> tokens;
		std::copy(std::istream_iterator<std::string>(textstream),
				std::istream_iterator<std::string>(), back_inserter(tokens));
		std::string actionname = tokens[1];
//		std::cout << "action name: " << actionname << std::endl;

		// parameters
		std::string parametersText = "";
		getText(domain, ":parameters", ":precondition", actionNumber,
				parametersText);
//		std::cout << parametersText << std::endl;
		std::vector<std::string> parameters;
		std::vector<std::string> forward_delimeds = split(parametersText, '?');
		for (int i = 1; i < forward_delimeds.size(); ++i) {
//			std::cout << "fd = " << forward_delimeds[i] << std::endl;
			std::vector<std::string> token = split(forward_delimeds[i], ')');
			parameters.push_back("?" + trim(token[0]));
		}
//		std::cout << "parameters: ";
//		for (int i = 0; i < parameters.size(); ++i) {
//			std::cout << parameters[i];
//		}
//		std::cout << std::endl;

		// Read in lifted expression
		LiftedAction lf;
		lf.key = actionNumber;
		lf.n_arguments = parameters.size();
		lf.symbol = actionname;

		std::string effectText = findRange(text, ":effect", ")))");

		std::vector<std::string> effects;
		std::vector<std::pair<unsigned int, std::vector<unsigned int>>>addnums;
		std::vector<std::pair<unsigned int, std::vector<unsigned int>>>delnums;

		forward_delimeds = split(effectText, '(');
		for (int i = 1; i < forward_delimeds.size(); ++i) {
			//			std::cout << "fd = " << forward_delimeds[i] << std::endl;
			std::vector<std::string> token = split(forward_delimeds[i], ')');
			if (token[0].compare("and ") != 0) {
				effects.push_back(token[0]);
			}
		}

		bool isDel = false;
		for (int p = 0; p < effects.size(); ++p) {
			if (effects[p].compare("not ") == 0) {
				isDel = true;
				continue;
			}
			for (int o = 0; o < ps.size(); ++o) {
				std::vector<std::string> lits = split(effects[p], ' ');
				if (lits[0].compare(ps[o].symbol) == 0) {
					std::pair<unsigned int, std::vector<unsigned int>> predicate;
					predicate.first = o;
					for (int i = 1; i < lits.size(); ++i) {
						for (int parms = 0; parms < parameters.size();
								++parms) {
							if (lits[i].compare(parameters[parms]) == 0) {
								predicate.second.push_back(parms);
								break;
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

		lf.adds = addnums;
		lf.dels = delnums;

		// TODO: need to parse preconditions.
		// Preconditions
		effects.clear();
		std::stringstream t(text);
//		std::string precText;
//		getBracket(t, ":precondition", 0, precText);
//		getText(t, ":precondition", ":effect", 0, precText);
		std::string precText = findRange(text, ":precondition", ":effect");
		std::vector<std::pair<unsigned int, std::vector<unsigned int>>>precnums;
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
			for (int o = 0; o < ps.size(); ++o) {
				if (lits[0].compare(ps[o].symbol) == 0) {
					std::pair<unsigned int, std::vector<unsigned int>> predicate;
					predicate.first = o;
					for (int i = 1; i < lits.size(); ++i) {
						for (int parms = 0; parms < parameters.size();
								++parms) {
							if (lits[i].compare(parameters[parms]) == 0) {
								predicate.second.push_back(parms);
								break;
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
		std::cout << "instantiating " << lf.symbol << "..." << std::endl;

		int param_max = pow(obs.size(), parameters.size());
		for (unsigned int pnum = 0; pnum < param_max; ++pnum) {
			std::vector<unsigned int> args;
			args.resize(parameters.size());
			// build arguments list.
			for (unsigned int arg = 0; arg < parameters.size(); ++arg) {
				args[arg] = obs[(pnum
						/ pow(obs.size(), parameters.size() - 1 - arg))
						% obs.size()].key;
			}

			std::vector<unsigned int> precnums;
			std::vector<unsigned int> addnums;
			std::vector<unsigned int> delnums;

			// TODO: These implementations can be optimized more.
			//       seems its closed to be able to parse freecell, but still slow.
			// TODO: build predicate table which return predicate key with lifted key and arguments.
			for (int i = 0; i < lf.precs.size(); ++i) {
				std::pair<unsigned int, std::vector<unsigned int>> prec =
						lf.precs[i];
				std::vector<unsigned int> prec_arg = getArguements(args,
						prec.second);
				unsigned key = 0;
				for(int k = 0; k < prec_arg.size(); ++k) {
					key = key * obs.size() + prec_arg[k];
				}
				key += g_predicates_index[prec.first];
				precnums.push_back(key);

//				for (int j = 0; j < g_predicates->size(); ++j) {
//					if (g_predicates->at(j).isEqual(prec.first, prec_arg)) {
//						precnums.push_back(g_predicates->at(j).key);
//						std::cout << "key = " << key << ":" << g_predicates->at(j).key << std::endl;
//						break;
//					}
//				}
			}
			for (int i = 0; i < lf.adds.size(); ++i) {
				std::pair<unsigned int, std::vector<unsigned int>> add =
						lf.adds[i];
				std::vector<unsigned int> add_arg = getArguements(args,
						add.second);
				unsigned key = 0;
				for(int k = 0; k < add_arg.size(); ++k) {
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
				for(int k = 0; k < del_arg.size(); ++k) {
					key = key * obs.size() + del_arg[k];
				}
				key += g_predicates_index[del.first];
				delnums.push_back(key);
			}
			std::string groundedName = actionname;

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

		// TODO: efficient way is to instantiate lifted actions.
		//       how can we do that?


		++actionNumber;
	}

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
			std::cout << "contradictory action: " << "(" << action.name << ")"
					<< std::endl;
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
	for (int a = 0; a < keys.size(); ++a) {
//		std::cout << a << std::endl;
		Action action = actionTable.getAction(keys[a]);
//		action.print();
		actionTrie.addAction(action);
	}
}

/**
 * Greedy algorithm to find balances.
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

// list all actions which adds p.
// TODO: would this be predicates not p? let's try it.
	std::vector<LiftedActionArg> actions_add_p;
	for (int a = 0; a < laas.size(); ++a) {
		std::vector<unsigned int> adds = laas[a].lf.addsInst;
		for (int ps = 0; ps < predicates.size(); ++ps) {
			if (find(adds.begin(), adds.end(), predicates[ps]) != adds.end()) {
				// found
				actions_add_p.push_back(laas[a]);
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
			std::set_difference(dels.begin(), dels.end(), predicates.begin(),
					predicates.end(), std::back_inserter(difs));
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
		for (int arg = 0; arg < ps[i].number_of_arguments; ++arg) {
			PredicateArg paa;
			paa.key = key++;
			paa.pred = ps[i];
			paa.instantiated_arg = arg;
			paa.group_key = -1;
			predargs.push_back(paa);
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
	for (int i = 0; i < predargs.size(); ++i) {
		if (predargs[i].group_key == -1) {
			std::vector<unsigned int> pss;
			pss.push_back(predargs[i].key);
//			std::cout << predargs[i].pred.symbol << "/"
//					<< predargs[i].instantiated_arg << std::endl;
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
	// TODO: make multiple way of making
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

		// the rest of the predicates will be the second structure.
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

int Strips::pow(int base, int p) {
	int ret = 1;
	for (int i = 0; i < p; ++i) {
		ret *= base;
	}
	return ret;
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

