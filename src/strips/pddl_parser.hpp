#ifndef PDDL_PARSER_HPP_
#define PDDL_PARSER_HPP_

#include <vector>
#include <string>
#include <fstream>

class PDDLParser {
	struct Node {
		std::string symbol;
		std::vector<Node*> children;
	};

	PDDLParser(std::istream& file);

};

#endif
