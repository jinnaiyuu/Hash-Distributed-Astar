/*
 * main_parser.cc
 *
 *  Created on: Jun 16, 2015
 *      Author: yuu
 */
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <iterator>
#include <algorithm>
#include "../utils.hpp"
using namespace std;

// argv[1]: domain file path
// argv[2]: instance file path

/**
 * parse defined types.
 * returns type names.
 * WRITE: these types will be defined in predicates.
 * TODO: implement polymorphic types with tree structure.
 */
vector<string> getTypes(istream& domain) {
	vector<string> types;
	string text;
	getBracket(domain, ":types", 0, text);
	vector<string> lits = split(text, ' ');
	for (int i = 1; i < lits.size(); ++i) {
		vector<string> delimeds = split(lits[i], ')');
		types.push_back(delimeds[0]);
	}
	return types;
}

/**
 * parse constants.
 *
 * WRITE: domain:predicates, instance:object, instance:init
 * TODO: implement polymorphic types with tree structure.
 */
void getConstants(istream& domain, vector<pair<string, string>>& type_object,
		string header) {
	vector<pair<string, string>> constants;
	string text;
	getBracket(domain, header, 0, text);
//	vector<string> undelimed = split(text, ' ');

	string deliminator = "-";
	vector<string> cur_objects;
	bool reading_object = true;

	istringstream iss(text);
	vector<string> tokens { istream_iterator<string> { iss }, istream_iterator<
			string> { } };
	//
	for (int i = 1; i < tokens.size(); ++i) {
		vector<string> lits = split(tokens[i], ')');
		if (lits.size() == 0) {
//			cout << "no literal" << endl;
			continue;
		}
		string lit = lits[0];
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
				pair<string, string> p(cur_objects[obs], lit);
				constants.push_back(p);
			}
			cur_objects.clear();
			reading_object = true;
		}
	}
	type_object = constants;
}

vector<string> getPredicate(istream& domain) {
	vector<string> types = getTypes(domain);

	string text;
	getBracket(domain, ":predicates", 0, text);
	istringstream tstream(text);

	string pred;
	bool isGood = true;
	unsigned int i = 0;
	vector<string> predicates;
	while (isGood) {
		string output;
		isGood = getBracket(tstream, "(", i, pred);
		istringstream pstream(pred);
		vector<string> tokens { istream_iterator<string> { pstream },
				istream_iterator<string> { } };
		for (int t = 0; t < tokens.size(); ++t) {
			std::size_t found = tokens[t].find("(");
			if (found != std::string::npos) {
				output.append(tokens[t]);
				output.append(" ");
			}
			found = tokens[t].find("?");
			if (found != std::string::npos) {
				output.append(tokens[t]);
				output.append(" ");
			}
			found = tokens[t].find(")");
			if (found != std::string::npos) {
				output.append(")\n");
			}
		}
		predicates.push_back(output);
	}

	for (int i = 0; i < types.size(); ++i) {
		string type = "(type_";
		type.append(types[i]);
		type.append(" ?t)\n");
		predicates.push_back(type);
	}
	return predicates;
}

// 1. for all actions, get all parameters.
// 2. add precondition the types of the parameter.
vector<string> getAction(istream& domain) {

	vector<string> actions;

	unsigned int action = 0;

	while (true) {
		string output;
		string text;
		if (!getBracket(domain, ":action", action++, text)) {
			break;
		}
		istringstream tstream(text);

		string param_text;
//		getText(tstream, ":parameters", ":precondition", 0, param_text);
//
		getText2(tstream, ":parameters", ":precondition", 0, param_text);

		istringstream param_stream(param_text);
		cout << "param_text: " << param_text;
		vector<pair<string, string>> types;
		getConstants(param_stream, types, "?");

		std::cout << "action parameters" << std::endl;
		for (int i = 0; i < types.size(); ++i) {
			cout << "(type_" << types[i].second << " " << types[i].first << ")"
					<< endl;
		}

		string precond_text;
		getText2(tstream, ":precondition", ":effect", 0, precond_text);
		istringstream precond_stream(precond_text);
		string prec_and;
		getBracket(precond_stream, "", 0, prec_and);
		cout << "prec_and: " << prec_and << endl;

	}
	return actions;
}

void writeDomain(ifstream& domain) {

}

int main(int argc, const char *argv[]) {

	ifstream domain(argv[1]);
	ifstream instance(argv[2]);

	//
//	vector<string> types = getTypes(domain);
//	for (int i = 0; i < types.size(); ++i) {
//		cout << types[i] << " ";
//	}
//	cout << endl;

	vector<pair<string, string>> constants;
	getConstants(domain, constants, ":constants");

	// Constants will be included in init states.
	std::cout << std::endl;
	std::cout << "################" << std::endl;
	std::cout << "Constants:" << std::endl;
	for (int i = 0; i < constants.size(); ++i) {
		cout << "(type_" << constants[i].second << " " << constants[i].first
				<< ")" << endl;
	}

	// Predicates
	std::cout << std::endl;
	std::cout << "################" << std::endl;
	std::cout << "Predicates:" << std::endl;
	vector<string> predicates = getPredicate(domain);
	for (int i = 0; i < predicates.size(); ++i) {
		cout << predicates[i];
	}
	cout << endl;

	// Actions
	std::cout << std::endl;
	std::cout << "################" << std::endl;
	std::cout << "Actions:" << std::endl;
	vector<string> actions = getAction(domain);

}

