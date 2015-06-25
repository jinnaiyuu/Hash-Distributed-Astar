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

// TODO: ignore lines starting with double semicolons. ;;

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

	// If no typing information there
	if (text.find(deliminator) == std::string::npos) {
		for (int i = 1; i < tokens.size(); ++i) {
			vector<string> lits = split(tokens[i], ')');
			if (lits.size() == 0) {
				continue;
			}
			string lit = lits[0];
			pair<string, string> p(lit, "");
			constants.push_back(p);
		}
		type_object = constants;
		return;
	}

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

/**
 * parse defined types.
 * returns type names.
 * WRITE: these types will be defined in predicates.
 * TODO: implement polymorphic types with tree structure.
 */
vector<pair<string, int>> getTypes(istream& domain) {
	vector<pair<string, int>> ret;
	vector<string> types_buf;

	vector<pair<string, string>> types;
	getConstants(domain, types, ":types");
	//	string text;
	for (int i = 0; i < types.size(); ++i) {
		vector<string>::iterator it;
		it = find(types_buf.begin(), types_buf.end(), types[i].second);
		if (it == types_buf.end()) {
			unsigned int w = - 1;
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

vector<string> getPredicate(istream& domain) {
	vector<pair<string, int>> types = getTypes(domain);

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
			if (found != std::string::npos && t + 1 != tokens.size()) {
				output.append(")\n\t");
			}
		}
		predicates.push_back(output);
	}

	for (int i = 0; i < types.size(); ++i) {
		string type = "\t(type_";
		type.append(types[i].first);
		type.append(" ?t)");
		predicates.push_back(type);
	}
	return predicates;
}

// 1. for all actions, get all parameters.
// 2. add precondition the types of the parameter.
vector<string> getAction(istream& domain) {

	vector<string> actions;

	unsigned int action = 0;

	// read each actions
	bool done = false;
	while (!done) {
		string output = "";
		string text;
//		if (!getBracket(domain, ":action", action++, text)) {
//			break;
//		}

		if (!getText2(domain, "(:action", "(:action", action++, text)) {
			done = true;
		}
//		cout << "action braket: " << text << endl;
		istringstream tstream(text);
//		cout << "action t stream = [" << tstream.str() << "]" << endl;
//		tstream >> std::noskipws;

		string param_text;
//		getText(tstream, ":parameters", ":precondition", 0, param_text);
//
		getText2(tstream, ":parameters", ":precondition", 0, param_text);
		istringstream param_stream(param_text);
//		cout << "param_text: " << param_text << endl;
		vector<pair<string, string>> types;
		getConstants(param_stream, types, "?");
		for (int i = 0; i < types.size(); ++i) {
			types[i].first.erase(
					remove(types[i].first.begin(), types[i].first.end(), '('),
					types[i].first.end());
		}
//		std::cout << "action parameters" << std::endl;
//		for (int i = 0; i < types.size(); ++i) {
//			cout << "(type_" << types[i].second << " " << types[i].first << ")"
//					<< endl;
//		}

		string precond_text;
		getText2(tstream, ":precondition", ":effect", 0, precond_text);
		istringstream precond_stream(precond_text);
//		string prec_and;
////		precond_stream >> prec_and;
//		getBracket(precond_stream, "", 0, prec_and);
//		cout << "prec_and: " << prec_and << endl;
//
//		istringstream prec_
		vector<string> tokens { istream_iterator<string> { precond_stream },
				istream_iterator<string> { } };

		// first: :precondition
		// last:  )
		vector<string> type_preds;
		for (int i = 0; i < types.size(); ++i) {
			string t = "(type_" + types[i].second + " " + types[i].first + ")";
			type_preds.push_back(t);
		}
		unsigned int s = tokens.size();
		if (tokens[s - 1].compare(")") == 0) {
			tokens.insert(tokens.end() - 1, type_preds.begin(),
					type_preds.end());
		} else {
			tokens.insert(tokens.end(), type_preds.begin(), type_preds.end());
		}
		string prec_string = "";
		for (int i = 0; i < tokens.size(); ++i) {
			prec_string.append(tokens[i]);
			prec_string.append(" ");
		}
//		cout << "prec_string: " << prec_string << endl;

		string act_name;
		getText2(tstream, "", ":parameters", 0, act_name);
		string param_name = ":parameters (";

		for (int i = 0; i < types.size(); ++i) {
			param_name.append(types[i].first);
			param_name.append(" ");
		}
		param_name.append(")");
//		getText2(tstream, ":parameters", ":precondition", 0, param_name);
		string tail;
		// read till the end
		getText2(tstream, ":effect", "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa", 0,
				tail);

		output.append(act_name);
		output.append("\r\n\t");
		output.append(param_name);
		output.append("\r\n\t");
		output.append(prec_string);
		output.append("\r\n\t");
		output.append(tail);
		actions.push_back(output);
	}
	return actions;
}

void getObjects(ifstream& instance, vector<pair<string, string>>& constants,
		string& ret) {
	string objects;
	getBracket(instance, ":objects", 0, objects);

	istringstream obj_stream(objects);

	// First read objects. objects have types.
	// Add those into constants.
	vector<pair<string, string>> types;
	getConstants(obj_stream, types, "");

	std::cout << "################################## " << std::endl << "types:"
			<< std::endl;
	for (int i = 0; i < types.size(); ++i) {
		cout << "(type_" << types[i].second << " " << types[i].first << ")"
				<< endl;
	}
//	for (int i = 0; i < constants.size(); ++i) {
//		cout << "(type_" << constants[i].second << " " << constants[i].first << ")"
//				<< endl;
//	}
	constants.insert(constants.end(), types.begin(), types.end());

	// Add constants from domain file.

//	vector<string> tokens { istream_iterator<string> { obj_stream },
//			istream_iterator<string> { } };
//	for (int i = 0; i < constants.size(); ++i) {
//		string t = constants[i].first;
//		tokens.insert(tokens.end() - 1, t);
//	}

	// Add new constants as a side effect.

	string out = "(:objects \n\t";

	for (int i = 0; i < constants.size(); ++i) {
		out.append(constants[i].first);
		out.append("\n\t");
	}
	out.append(")");
//	for (int i = 0; i < tokens.size(); ++i) {
//		out.append(tokens[i]);
//		out.append("\n\t");
//	}
	ret = out;
}

// not the optimal implementation. but polymorphic type is not so often used.
vector<int> matchTypes(int t, const vector<pair<string, int>>& types) {
	if (t == -1) {
		vector<int> a;
		return a;
	}
	vector<int> r = matchTypes(types[t].second, types);
	r.push_back(t);
	return r;
}

void getInit(ifstream& instance, const vector<pair<string, string>>& constants,
		const vector<pair<string, int>>& types, string& ret) {
	string inits;
	getBracket(instance, ":init", 0, inits);

	istringstream init_stream(inits);

	vector<string> tokens { istream_iterator<string> { init_stream },
			istream_iterator<string> { } };

//	for (int i = 0; i < constants.size(); ++i) {
//		string t = "(type_" + constants[i].second + " " + constants[i].first;
//		if (t.find(")") == std::string::npos) {
//			t.append(")");
//		}
//		tokens.insert(tokens.end(), t);
//	}

	for (int ts = 0; ts < types.size(); ++ts) {
		cout << types[ts].first << endl;
	}


	for (int i = 0; i < constants.size(); ++i) {
		string t = constants[i].second;
//		cout << "consts: " << constants[i].second << ", " << constants[i].first;
		for (int j = 0; j < types.size(); ++j) {
			if (t.compare(types[j].first) == 0) {
				vector<int> t;
				if (types[j].second >= 0) {
					t = matchTypes(j, types);
				}
				for (int ts = 0; ts < t.size(); ++ts) {
//					cout << ", t: " << types[t[ts]].first;
					string type = "(type_" + types[t[ts]].first + " "
							+ constants[i].first + ")";
					tokens.insert(tokens.end(), type);
				}
//				cout << endl;
				break;
			}
		}
	}

	string out = "";
	for (int i = 0; i < tokens.size(); ++i) {
		out.append(tokens[i]);
		out.append(" ");
		if (tokens[i].find(")") != std::string::npos) {
			out.append("\n\t");
		}
	}
	ret = out;
}

int main(int argc, const char *argv[]) {

	ifstream domain(argv[1]);
	ifstream instance(argv[2]);

	//
	vector<pair<string, int>> types = getTypes(domain);
	std::cout << std::endl;
	std::cout << "################" << std::endl;
	std::cout << "Types:" << std::endl;
	for (int i = 0; i < types.size(); ++i) {
		if (types[i].second >= 0) {
			cout << types[i].first << " - " << types[types[i].second].first << endl;
		} else {
			cout << types[i].first << endl;
		}
	}
	cout << endl;

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
	for (int i = 0; i < actions.size(); ++i) {
		cout << actions[i] << endl << endl;
	}

	////////////////////////////////
	/// Output domain file
	////////////////////////////////
	string head_out;
	getText2(domain, "(define", "(:types", 0, head_out);
	string predicate_out = "";
	for (int i = 0; i < predicates.size(); ++i) {
		predicate_out.append(predicates[i]);
		predicate_out.append("\n");
	}
	predicate_out.append("\t)\n");
	string action_out = "";
	for (int i = 0; i < actions.size(); ++i) {
		action_out.append(actions[i]);
		action_out.append("\n\n");
	}

	string out = "";
	out.append(head_out);
	out.append("\n\n");
	out.append(predicate_out);
	out.append("\n\n");
	out.append(action_out);
//	cout << out;

	string outdomain = string(argv[1]) + "-strips";
	ofstream output;
	output.open(outdomain.c_str());
	output << out;
	output.close();

	////////////////////////////////
	/// Parse instance file
	////////////////////////////////

	// 1. Add objects defined in domain::constant
	// 2. Add predicates defined in domain::constant
	string object_out = "";
	string init_out = "";
	string goal_out = "";

	getObjects(instance, constants, object_out);
	getInit(instance, constants, types, init_out);

	////////////////////////////////
	/// Output instance file
	////////////////////////////////
	getText2(instance, "(define", "(:objects", 0, head_out);

	getText2(instance, "(:goal", "aaaaaaaaaaaaaaa", 0, goal_out);

	out = "";
	out.append(head_out);
	out.append("\n\n");
	out.append(object_out);
	out.append("\n\n");
	out.append(init_out);
	out.append("\n\n");
	out.append(goal_out);
//	cout << out;

	outdomain = string(argv[2]) + "-strips";
	output.open(outdomain.c_str());
	output << out;
	output.close();

	return 0;
}

