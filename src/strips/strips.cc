// Copyright 2012 Ethan Burns. All rights reserved.
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file.
#include "strips.hpp"

#include <string>
#include <iostream>

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

	std::cout << "init predicates..." << std::endl;
	readPredicates(domain, predicates);

	std::cout << "init objects..." << std::endl;
	readObjects(instance, objects);

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
		pos = line.find(search); // search
		if (pos != std::string::npos) {
			std::cout << "Found!: " << line << std::endl;
			std::size_t pos = line.find(":precondition") + 15;
			std::string prec = line.substr(pos);
			std::cout << prec << std::endl;
			int pnum = 0;
			std::string symbol;
			unsigned int argc;
//			Predicate* p = new Predicate();
			predicates.resize(1);
			while (phrase_parse((std::string::const_iterator) prec.begin(),
					(std::string::const_iterator) prec.end(), g, space,
					predicates[pnum])) {
				predicates[pnum].key = pnum;
				++pnum;
				predicates.resize(predicates.size() + 1);
				getline(domain, prec);
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

template<typename Iterator>
struct object_parser: qi::grammar<Iterator, std::vector<std::string>,
		ascii::space_type> {
	object_parser() :
			object_parser::base_type(start) {
		symbol %= lexeme[+(alpha)];
		start %=
				lit("(:objects") >>
				*(symbol) >>
				')';
	}
	qi::rule<Iterator, std::string(), ascii::space_type> symbol;
	qi::rule<Iterator, std::vector<std::string>, ascii::space_type> start;
};

//template <typename Iterator>
//bool object_parser(Iterator first, Iterator last, std::vector<std::string>& v)
//{
//    using phoenix::push_back;
//    using phoenix::ref;
//    bool r = phrase_parse(first, last,
//        (
//        	'(' >>
//            *(lexeme[+(alpha)] [push_back(ref(v), _1)]) >>
//            ')'
//        ),
//        space);
//    if (first != last) // fail if we did not get a full match
//        return false;
//    return r;
//}

void Strips::readObjects(std::istream &instance, std::vector<Object>& objects) {
	std::string line;
	std::string search = ":objects";
	size_t pos;
	std::vector<std::string> strings;

	object_parser<std::string::const_iterator> g;

	while (instance.good()) {
		getline(instance, line); // get line from file
		pos = line.find(search); // search
		if (pos != std::string::npos) {
			std::cout << "Found: " << line << std::endl;
			if (phrase_parse((std::string::const_iterator) line.begin(),
					(std::string::const_iterator) line.end(), g, space,
					strings)) {
				std::cout << "parse succeeded" << std::endl;
			}
			break;
		}
	}

	for (int i = 0; i < strings.size(); ++i) {
		std::cout << strings[i] << std::endl;
	}

}

void Strips::readInit(std::istream &instance) {

}

void Strips::readGoal(std::istream &instance) {

}

void Strips::readAction(std::istream &domain) {

}

