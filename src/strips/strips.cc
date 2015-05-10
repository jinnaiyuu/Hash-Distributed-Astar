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

	std::vector<GroundedPredicate> g_predicates; // uninstantiated


	std::vector<GroundedPredicate> g_feasible_predicates;
//	std::vector < Action >

	// 1. learn predicates from domain.pddl
	std::cout << "init predicates..." << std::endl;
	readPredicates(domain, predicates);
	std::cout << "generated " << predicates.size() << " lifted predicates."
			<< std::endl;

	// 2. learn objects instance.pddl.
	std::cout << "init objects..." << std::endl;
	readObjects(instance, objects);
	std::cout << "generated" << objects.size() << " objects." << std::endl;

	// 3. instantiate all predicates.
	std::cout << "ground all predicates..." << std::endl;
	groundPredicates(predicates, objects, g_predicates);
	std::cout << "generated " << g_predicates.size() << " grounded predicates."
			<< std::endl;

	// 4. read initial state from instance.pddl.
	std::cout << "parse initial state..." << std::endl;
	readInit(instance, g_predicates);

	// 5. read goal condition from instance.pddl.
	std::cout << "parse goal condition..." << std::endl;
	readGoal(instance, g_predicates);

	// 6. read actions from domain.pddl.

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
}

void Strips::readAction(std::istream &domain) {

}

int Strips::pow(int base, int p) {
	int ret = 1;
	for (int i = 0; i < p; ++i) {
		ret *= base;
	}
	return ret;
}
