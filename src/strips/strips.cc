// Copyright 2012 Ethan Burns. All rights reserved.
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file.
#include "strips.hpp"
#include "action.hpp"
#include "utils.hpp"

#include <string>
#include <iostream>
#include <algorithm>

// spirit
#include <boost/config/warning_disable.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/io.hpp>

namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;
namespace phoenix = boost::phoenix;
using qi::int_;
using qi::lit;
using qi::_val;
using qi::_1;
using qi::eps;
using qi::alpha;
using qi::double_;
using qi::lexeme;
using qi::phrase_parse;
using ascii::char_;
using boost::spirit::ascii::space;

Strips::Strips(std::istream &domain, std::istream &instance) {

	std::vector<Predicate> predicates; // uninstantiated
	std::vector<Object> objects;

	std::vector<unsigned int> g_feasible_predicates;

//	std::vector < Action >

	// 1. learn predicates from domain.pddl
	std::cout << "parsing predicates..." << std::endl;
	readPredicates(domain, predicates);
	std::cout << "generated " << predicates.size() << " lifted predicates."
			<< std::endl;

	// 2. learn objects instance.pddl.
	std::cout << "parsing objects..." << std::endl;
	readObjects(instance, objects);
	std::cout << "generated" << objects.size() << " objects." << std::endl;

	// 3. instantiate all predicates.
	std::cout << "grounding all predicates..." << std::endl;
	groundPredicates(predicates, objects, g_predicates);
	std::cout << "generated " << g_predicates.size() << " grounded predicates."
			<< std::endl;

	// 4. read initial state from instance.pddl.
	std::cout << "parsing initial state..." << std::endl;
	readInit(instance, g_predicates);

	// 5. read goal condition from instance.pddl.
	std::cout << "parsing goal condition..." << std::endl;
	readGoal(instance, g_predicates);

	// 6. read actions from domain.pddl.
	std::cout << "parsing actions..." << std::endl;
	readAction(domain, objects, g_predicates);

	std::cout << "generated " << actionTable.getSize() << " grounded actions."
			<< std::endl;
//	actionTable.printAllActions();

	// 7. list all feasible predicates to be true.
	// init state, actions,
	std::cout << "calculating feasible predicates..." << std::endl;
	listFeasiblePredicates(g_feasible_predicates);
	std::cout << g_feasible_predicates.size() << " predicates feasible."
			<< std::endl;

	// 8. list all feasible actions.
	std::cout << "calculating feasible actions..." << std::endl;
	std::vector<unsigned int> feasible_actions;
	listFeasibleActions(g_feasible_predicates, feasible_actions);
	std::cout << feasible_actions.size() << " actions feasible."
			<< std::endl;

	std::cout << "building action trie..." << std::endl;
	buildActionTrie(feasible_actions);
	std::cout << "done!" << std::endl;

	actionTrie.printTree();

	std::vector<unsigned int> init_actions = actionTrie.searchPossibleActions(init_state);

	std::cout << "init state: ";
	for (int i = 0; i < init_state.size(); ++i) {
		std::cout << init_state[i] << " ";
	}
	std::cout << std::endl;
	std::cout << init_actions.size() << " initial actions." << std::endl;
	for (int i = 0; i < init_actions.size(); ++i) {
		actionTable.getAction(init_actions[i]).print();
	}

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

BOOST_FUSION_ADAPT_STRUCT( Strips::Predicate,
		(std::string, symbol) (unsigned int, number_of_arguments))

template<typename Iterator>
struct predicate_parser: qi::grammar<Iterator, Strips::Predicate(),
		ascii::space_type> {
	predicate_parser() :
			predicate_parser::base_type(start) {
		symbol %= lexeme[+(alpha)];
		args = eps[_val = 0] >> *(lexeme['?' >> +(alpha)])[_val += 1];
		start %= '(' >> symbol >> args >> ')';
	}
	qi::rule<Iterator, std::string(), ascii::space_type> symbol;
	qi::rule<Iterator, unsigned(), ascii::space_type> args;
	qi::rule<Iterator, Strips::Predicate(), ascii::space_type> start;
};

void Strips::readPredicates(std::istream &domain,
		std::vector<Predicate>& predicates) {
	std::string line;
	std::string search = ":predicates";
	size_t pos;
	predicate_parser<std::string::const_iterator> g;

	while (domain.good()) {
		getline(domain, line); // get line from file
		std::transform(line.begin(), line.end(), line.begin(), ::tolower);
		pos = line.find(search); // search
		if (pos != std::string::npos) {
			std::cout << "Found!: " << line << std::endl;
			std::size_t pos = line.find(":precondition") + 15;
			std::string prec = line.substr(pos);
			std::cout << prec << std::endl;
			int pnum = 0;
			std::string symbol;
//			Predicate* p = new Predicate();
			predicates.resize(1);
			while (phrase_parse((std::string::const_iterator) prec.begin(),
					(std::string::const_iterator) prec.end(), g, space,
					predicates[pnum])) {
				predicates[pnum].key = pnum;
				++pnum;
				predicates.resize(predicates.size() + 1);
				getline(domain, prec);
				std::transform(prec.begin(), prec.end(), prec.begin(),
						::tolower);
			}
			predicates.pop_back();
			break;
//				p = new Predicate();
		}
//			delete p;
	}

	for (int i = 0; i < predicates.size(); ++i) {
		std::cout << predicates[i].key << " " << predicates[i].symbol << "/"
				<< predicates[i].number_of_arguments << std::endl;
	}
}

void Strips::readObjects(std::istream &instance, std::vector<Object>& objects) {
	std::string line;
	std::string search = ":objects";
	size_t pos;
	std::vector<std::string> strings;

	while (instance.good()) {
		getline(instance, line); // get line from file
		std::transform(line.begin(), line.end(), line.begin(), ::tolower);
		pos = line.find(search); // search
		if (pos != std::string::npos) {
			std::cout << "Found: " << line << std::endl;

			std::vector<std::string> tokens;
			std::istringstream iss(line);
			std::copy(std::istream_iterator<std::string>(iss),
					std::istream_iterator<std::string>(),
					back_inserter(tokens));

			for (int i = 1; i < tokens.size() - 1; ++i) {
				strings.push_back(tokens[i]);
			}

			break;
		}
	}
	std::cout << "strings" << std::endl;
	objects.resize(strings.size());
	for (int i = 0; i < strings.size(); ++i) {
		std::cout << strings[i] << std::endl;
		objects[i].key = i;
		objects[i].symbol = strings[i];
	}

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

	for (int gnum = 0; gnum < gs.size(); ++gnum) {
		std::cout << gs[gnum].symbol << std::endl;
	}
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
			std::cout << "Found: " << line << std::endl;
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
	std::cout << "inital state: " << total_text << std::endl;

	std::vector<std::string> symbols; // parse into here.

	// now we need to parse total text into vector of strings(symbols).
	std::vector<std::string> forward_delimeds = split(total_text, '(');
	for (int i = 2; i < forward_delimeds.size(); ++i) {
		std::vector<std::string> token = split(forward_delimeds[i], ')');

		symbols.push_back(token[0]);
	}

	for (int i = 0; i < symbols.size(); ++i) {
		std::cout << i << ": " << symbols[i];

		for (int j = 0; j < gs.size(); ++j) {
//			std::cout << j << ": " << gs[j].symbol << std::endl;
			if (symbols[i].compare(gs[j].symbol) == 0) {
				// if two symbols are equal, then same predicates.
				std::cout << " matches " << j << std::endl;
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

	std::string total_text;
	std::string line;
	std::string init_ = ":goal";
	size_t pos;
	std::vector<std::string> strings;

	// put total_text the text for inital state.
	while (instance.good()) {
		getline(instance, line); // get line from file
		std::transform(line.begin(), line.end(), line.begin(), ::tolower);
		pos = line.find(init_); // search
		if (pos != std::string::npos) {
			std::cout << "Found: " << line << std::endl;
			total_text = line;
			while (instance.good()) {
				getline(instance, line); // get line from file
				std::transform(line.begin(), line.end(), line.begin(),
						::tolower);
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
	std::cout << "goal state: " << total_text << std::endl;

	std::vector<std::string> symbols; // parse into here.

	// now we need to parse total text into vector of strings(symbols).
	std::vector<std::string> forward_delimeds = split(total_text, '(');
	for (int i = 3; i < forward_delimeds.size(); ++i) {
		std::vector<std::string> token = split(forward_delimeds[i], ')');

		symbols.push_back(token[0]);
	}

	for (int i = 0; i < symbols.size(); ++i) {
		std::cout << i << ": " << symbols[i];

		for (int j = 0; j < gs.size(); ++j) {
//			std::cout << j << ": " << gs[j].symbol << std::endl;
			if (symbols[i].compare(gs[j].symbol) == 0) {
				// if two symbols are equal, then same predicates.
				std::cout << " matches " << j << std::endl;
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
		std::vector<GroundedPredicate> gs) {
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

		// instantiate grounded actions.
		// TODO: enable typing
		int newactnum = pow(obs.size(), parameters.size());
		for (unsigned int newnum = 0; newnum < newactnum; ++newnum) {
			std::vector<std::string> args;
			args.resize(parameters.size());
			// build arguments list.
			for (unsigned int arg = 0; arg < parameters.size(); ++arg) {
				args[arg] = obs[(newnum
						/ pow(obs.size(), parameters.size() - 1 - arg))
						% obs.size()].symbol;
			}

//			std::cout << "instantiate with ";
//			for (unsigned int arg = 0; arg < parameters.size(); ++arg) {
//				std::cout << args[arg];
//			}
//			std::cout << std::endl;

			// replace arguments with objects.
			std::string groundedText = text;
			for (unsigned int arg = 0; arg < parameters.size(); ++arg) {
				std::string buf = replace(groundedText, parameters[arg],
						args[arg]);
				groundedText = buf;
//				std::cout << groundedText << std::endl;

			}
//			std::cout << "grounded text = " << groundedText << std::endl;

			// read preconditions and translate into GroundedPredicate keys.
			std::string preconditionText = findRange(groundedText,
					":precondition", ":effect");
//			std::cout << "prec = " << preconditionText << std::endl;

			std::vector<std::string> preconditions;
			std::vector<unsigned int> precnums;

			forward_delimeds = split(preconditionText, '(');
			for (int i = 1; i < forward_delimeds.size(); ++i) {
				//			std::cout << "fd = " << forward_delimeds[i] << std::endl;
				std::vector<std::string> token = split(forward_delimeds[i],
						')');
				if (token[0].compare("and") != 0) {
					preconditions.push_back(token[0]);
				}
			}

//			std::cout << "preconditions = ";
//			for (int p = 0; p < preconditions.size(); ++p) {
//				std::cout << "(" << preconditions[p] << ") ";
//			}
//			std::cout << std::endl;
			for (int p = 0; p < preconditions.size(); ++p) {
				for (int g = 0; g < gs.size(); ++g) {
					if (preconditions[p].compare(gs[g].symbol) == 0) {
						precnums.push_back(g);
					}
				}
			}
			std::sort(precnums.begin(), precnums.end());
//			for (int p = 0; p < precnums.size(); ++p) {
//				std::cout << "num = " << precnums[p] << std::endl;
//			}

			std::string effectText = findRange(groundedText, ":effect", ")))");

			std::vector<std::string> effects;
			std::vector<unsigned int> addnums;
			std::vector<unsigned int> delnums;

			forward_delimeds = split(effectText, '(');
			for (int i = 2; i < forward_delimeds.size(); ++i) {
				//			std::cout << "fd = " << forward_delimeds[i] << std::endl;
				std::vector<std::string> token = split(forward_delimeds[i],
						')');
				effects.push_back(token[0]);
			}

//			std::cout << "effects = ";
//			for (int p = 0; p < effects.size(); ++p) {
//				std::cout << "(" << effects[p] << ") ";
//			}
//			std::cout << std::endl;
			bool isDel = false;
			for (int p = 0; p < effects.size(); ++p) {
				if (effects[p].compare("not ") == 0) {
					isDel = true;
					continue;
				}
				for (int g = 0; g < gs.size(); ++g) {
					if (effects[p].compare(gs[g].symbol) == 0) {
						if (isDel) {
							delnums.push_back(g);
						} else {
							addnums.push_back(g);
						}
						isDel = false;
						break;
					}
				}
			}
			std::sort(addnums.begin(), addnums.end());
			std::sort(delnums.begin(), delnums.end());

//			for (int p = 0; p < addnums.size(); ++p) {
//				std::cout << "add num = " << addnums[p] << std::endl;
//			}
//			for (int p = 0; p < delnums.size(); ++p) {
//				std::cout << "del num = " << delnums[p] << std::endl;
//			}

			std::string groundedName = actionname;
			for (int i = 0; i < args.size(); ++i) {
				groundedName.append(" " + args[i]);
			}
//			std::cout << "gname = " << groundedName << std::endl;

			Action a(groundedName, gActionNumber, precnums, addnums, delnums);
			actionTable.addAction(a);
			++gActionNumber;
		}

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

void Strips::listFeasibleActions(std::vector<unsigned int> gs, std::vector<unsigned int>& actions) {
	for (int a = 0; a < actionTable.getSize(); ++a) {
		Action action = actionTable.getAction(a);
		bool isFeasible = true;
		for (int prec = 0; prec < action.preconditions.size(); ++prec) {
			if (std::find(gs.begin(), gs.end(), action.preconditions[prec]) == gs.end()) {
				isFeasible = false;
				break;
			}
		}
		if (isFeasible) {
			actions.push_back(a);
		}
	}
}

void Strips::buildActionTrie(std::vector<unsigned> keys) {
	for (int a = 0; a < keys.size(); ++a) {
		std::cout << a << std::endl;
		Action action = actionTable.getAction(keys[a]);
		action.print();
		actionTrie.addAction(action);
	}
}

int Strips::pow(int base, int p) {
	int ret = 1;
	for (int i = 0; i < p; ++i) {
		ret *= base;
	}
	return ret;
}

//////////////////////////////////////////////////////
/// utilities
//////////////////////////////////////////////////////

void Strips::print_state(std::vector<unsigned int>& propositions) const {
	std::cout << "state: ";
	for (int p = 0; p < propositions.size(); ++p) {
		std::cout << "(" << g_predicates[propositions[p]].symbol << ") ";
	}
	std::cout << std::endl;
}

void Strips::print_plan(std::vector<unsigned int>& path) const {
	State s;
	s.propositions = init_state;
	print_state(s.propositions);
	for (int i = 0; i < path.size(); ++i) {
		std::cout << "action: " << actionTable.getAction(path[i]).name << std::endl;
		apply_action(s, path[i]);
		print_state(s.propositions);
	}
}


