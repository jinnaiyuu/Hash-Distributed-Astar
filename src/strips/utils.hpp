#ifndef UTILS_HPP_
#define UTILS_HPP_

#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
#include <functional>
#include <assert.h>
#include <utility>

static std::vector<std::string> &split(const std::string &s, char delim,
		std::vector<std::string> &elems) {
	std::stringstream ss(s);
	std::string item;
	while (std::getline(ss, item, delim)) {
		if (!item.empty()) {
			elems.push_back(item);
		}
	}
	return elems;
}

static std::vector<std::string> split(const std::string &s, char delim) {
	std::vector<std::string> elems;
	split(s, delim, elems);
	return elems;
}

static std::string& replace(std::string& s, const std::string& from,
		const std::string& to) {
	for (size_t pos = 0; (pos = s.find(from, pos)) != std::string::npos; pos +=
			to.size())
		s.replace(pos, from.size(), to);
	return s;
}

static std::string trim(std::string& str) {
	size_t first = str.find_first_not_of(' ');
	size_t last = str.find_last_not_of(' ');
	if (first == str.npos) {
		first = 0;
	}
	if (last == str.npos) {
		return str.substr(first);
	}
	return str.substr(first, (last - first + 1));
}

static std::string findRange(std::string& s, const std::string& from,
		const std::string& to) {
	size_t f = 0;
	size_t t = 0;
	f = s.find(from, f);
	t = s.find(to, f);
	if (t == std::string::npos || f == std::string::npos) {
		return s.substr(f);
	} else {
		return s.substr(f, t - f);

	}

//	if (f < 0 || t-f < 0 || t-f >= s.size()) {
//		std::cout << "out-of-range: " << f << " to " << t << ": " << s <<std::endl;
//	}

}

// if v2 has all numbers in v1 then return true.
// true if v2 >= v1.
static
bool isContainedSortedVectors(const std::vector<unsigned int>& v1,
		const std::vector<unsigned int>& v2) {
	unsigned int v1it = 0;
	unsigned int v2it = 0;

	while (v1it < v1.size() && v2it < v2.size()) {
		if (v1[v1it] == v2[v2it]) {
			++v1it;
			++v2it;
		} else if (v1[v1it] < v2[v2it]) {
			return false;
			++v1it;
		} else {
			++v2it;
		}
	}
	return true;
}

static
bool isContainedSortedVectors(unsigned int v1,
		const std::vector<unsigned int>& v2) {
	unsigned int v2it = 0;

	while (v2it < v2.size()) {
		if (v1 == v2[v2it]) {
			return true;
		} else if (v1 < v2[v2it]) {
			return false;
		} else {
			++v2it;
		}
	}
	return false;
}

static
bool isAnyContainedSortedVectors(const std::vector<unsigned int>& v1,
		const std::vector<unsigned int>& v2) {
	unsigned int v1it = 0;
	unsigned int v2it = 0;

	while (v1it < v1.size() && v2it < v2.size()) {
		if (v1[v1it] == v2[v2it]) {
			return true;
		} else if (v1[v1it] < v2[v2it]) {
			++v1it;
		} else {
			++v2it;
		}
	}
	return false;
}

// if any of v1 not contained in v2, return true. false otherwise.
static
bool isAnyNotContainedSortedVectors(const std::vector<unsigned int>& v1,
		const std::vector<unsigned int>& v2) {
	unsigned int v1it = 0;
	unsigned int v2it = 0;

	unsigned int contained = 0;

	while (v1it < v1.size() && v2it < v2.size()) {
		if (v1[v1it] == v2[v2it]) {
			++v1it;
			++v2it;
		} else if (v1[v1it] < v2[v2it]) {
			return true;
		} else {
			++v2it;
		}
	}
	if (v1it == v1.size()) {
		return false;
	} else {
		return true;
	}
}

static
unsigned int howManyContainedSortedVectors(const std::vector<unsigned int>& v1,
		const std::vector<unsigned int>& v2) {
	unsigned int v1it = 0;
	unsigned int v2it = 0;

	unsigned int contained = 0;

	while (v1it < v1.size() && v2it < v2.size()) {
		if (v1[v1it] == v2[v2it]) {
			++contained;
			++v1it;
			++v2it;
		} else if (v1[v1it] < v2[v2it]) {
			++v1it;
		} else {
			++v2it;
		}
	}
	return contained;
}

static std::vector<unsigned int> intersectingSortedVectors(
		const std::vector<unsigned int>& v1,
		const std::vector<unsigned int>& v2) {
	std::vector<unsigned int> intersecting;
	unsigned int v1it = 0;
	unsigned int v2it = 0;

	while (v1it < v1.size() && v2it < v2.size()) {
		if (v1[v1it] == v2[v2it]) {
			intersecting.push_back(v1[v1it]);
			++v1it;
			++v2it;
		} else if (v1[v1it] < v2[v2it]) {
			++v1it;
		} else {
			++v2it;
		}
	}
	return intersecting;
}

static std::vector<unsigned int> uniquelyMergeSortedVectors2(
		const std::vector<unsigned int>& v1, const unsigned int& v2) {
	std::vector<unsigned int> uniqueMerge;
	unsigned int v1it = 0;
	while (v1it < v1.size()) {
		if (v1[v1it] == v2) {
			uniqueMerge.push_back(v1[v1it]);
			++v1it;
			break;
		} else if (v1[v1it] < v2) {
			uniqueMerge.push_back(v1[v1it]);
			++v1it;
		} else {
			uniqueMerge.push_back(v2);
			break;
		}
	}
	if (v1it < v1.size()) {
		uniqueMerge.insert(uniqueMerge.end(), v1.begin() + v1it, v1.end());
	}
	return uniqueMerge;
}

static std::vector<unsigned int> uniquelyMergeSortedVectors(
		const std::vector<unsigned int>& v1,
		const std::vector<unsigned int>& v2) {
	std::vector<unsigned int> uniqueMerge;
	unsigned int v1it = 0;
	unsigned int v2it = 0;
	while (v1it < v1.size() && v2it < v2.size()) {
		if (v1[v1it] == v2[v2it]) {
			uniqueMerge.push_back(v1[v1it]);
			++v1it;
			++v2it;
		} else if (v1[v1it] < v2[v2it]) {
			uniqueMerge.push_back(v1[v1it]);
			++v1it;
		} else {
			uniqueMerge.push_back(v2[v2it]);
			++v2it;
		}
	}

	if (v1it == v1.size()) {
		uniqueMerge.insert(uniqueMerge.end(), v2.begin() + v2it, v2.end());
	} else {
		uniqueMerge.insert(uniqueMerge.end(), v1.begin() + v1it, v1.end());
	}

	return uniqueMerge;
}

// Take difference of sets, v1 - v2.
static std::vector<unsigned int> differenceSortedVectors(
		const std::vector<unsigned int>& v1,
		const std::vector<unsigned int>& v2) {
	std::vector<unsigned int> difference;
	unsigned int v1it = 0;
	unsigned int v2it = 0;
	while (v1it < v1.size() && v2it < v2.size()) {
		if (v1[v1it] == v2[v2it]) {
			++v1it;
			++v2it;
		} else if (v1[v1it] < v2[v2it]) {
			difference.push_back(v1[v1it]);
			++v1it;
		} else {
			++v2it;
		}
	}
	if (v2it == v2.size()) {
		difference.insert(difference.end(), v1.begin() + v1it, v1.end());
	}
	return difference;
}

static std::vector<unsigned int> getArguements(
		const std::vector<unsigned int>& argument,
		const std::vector<unsigned int>& which) {
	std::vector<unsigned int> args;
	for (int i = 0; i < which.size(); ++i) {
		if (which[i] >= argument.size()) {
			args.push_back(which[i] - argument.size());
		} else {
			args.push_back(argument[which[i]]);
		}
	}
	return args;
}

inline unsigned int hash(const std::vector<unsigned int> v) {
	std::hash<unsigned int> hasher;
	unsigned int seed = 0;
	for (int i = 0; i < v.size(); i++) {
		seed ^= hasher(v[i]) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
	}
	return seed;
}

static
int matchStringIndex(const std::vector<std::pair<std::string, int>>& dic,
		const std::string& string) {
	for (int i = 0; i < dic.size(); ++i) {
		if (dic[i].first.compare(string) == 0) {
			return i;
		}
	}
	return -1;
}

static
int matchStringInt(const std::vector<std::pair<std::string, int>>& dic,
		const std::string& string) {
	for (int i = 0; i < dic.size(); ++i) {
		if (dic[i].first.compare(string) == 0) {
			return dic[i].second;
		}
	}
	return -1;
}

// Check if obj_type is type of subtype of req type.
static
bool isType(const int obj_type, const int req_type,
		const std::vector<std::pair<std::string, int>>& dic) {
	if (obj_type == req_type) {
		return true;
	}
	if (req_type == -1) {
		return true;
	}

	int super_obj_type = dic[obj_type].second;
	while (super_obj_type >= 0) {
		if (super_obj_type == req_type) {
			return true;
		}
		super_obj_type = dic[super_obj_type].second;
	}
	return false;
}

/**
 * @param from: first literal inside bracket.
 * @param number: if there are multiple brackets starting from "from", it selects which one to read.
 * @param ret: returning value.
 * @return: if it has the
 *
 */
static
bool getBracket(std::istream &file, const std::string &from,
		unsigned int number, std::string& ret) {
	if (!file.good()) {
//		std::cout << "error on file" << std::endl;
		file.clear();
	}
	file.seekg(0, std::ios_base::beg);

	std::string total_text;
	std::string line;
	size_t pos;
	std::vector<std::string> strings;

	unsigned int counter = 0;
	// put total_text the text for inital state.

	// find "from" from the input stream.
	while (file.good()) {
		getline(file, line); // get line from file
		std::transform(line.begin(), line.end(), line.begin(), ::tolower);
		pos = line.find(from); // search
		if (pos != std::string::npos) {
			++counter;
//			std::cout << "match = " << line << std::endl;
			if (counter > number) {
				total_text = line;
//				std::cout << "take" << std::endl;
				break;
			} else {
//				std::cout << "throw" << std::endl;
			}
		}
	}

	// find the ending of the bracket.
	bool hasEnded = false;
	while (file.good()) {
		getline(file, line); // get line from file
		std::transform(line.begin(), line.end(), line.begin(), ::tolower);

		size_t open = std::count(total_text.begin(), total_text.end(), '(');
		size_t close = std::count(total_text.begin(), total_text.end(), ')');

		if (line.length() > 0) {
			total_text.append(" ");
		}
		total_text.append(line);
		if (open == close) {
			hasEnded = true;
			break;
		} else if (open < close) {
			// error: more closure than open
//			assert(false);
		}
	}

	if (!hasEnded) {
		ret = total_text;
		return false;
	}
	ret = total_text;
	return true;
}

/**
 * @param from: first literal inside bracket.
 * @param number: if there are multiple brackets starting from "from", it selects which one to read.
 * @param ret: returning value.
 * @return: if it has the
 *
 */
static
bool getBracket2(std::istream &file, const std::string &from,
		unsigned int number, std::string& ret) {
//	std::cout << "getBracket2" << std::endl;
	if (!file.good()) {
//		std::cout << "error on file" << std::endl;
		file.clear();
	}
	file.seekg(0, std::ios_base::beg);

	std::string r = "( ";
	std::string token;
	int parenthesis = 0;
	int token_parenthesis;

//	std::cout << "start" << std::endl;
	int n = 0;

	while (file.good()) {
		file >> token;
//		std::cout << "token = " << token;
		if (token.compare(from) == 0) {
			++n;
			if (n > number) {
				token_parenthesis = parenthesis;
				break;
			}
		} else if (token.compare("(") == 0) {
			++parenthesis;
		} else if (token.compare(")") == 0) {
			--parenthesis;
		} else if (file.eof()) {
			return false;
		}
	}

	if (!file.good()) {
		return false;
	}

//	std::cout << token;

	r.append(from);
	r.append(" ");
	while (file.good()) {
		file >> token;
		r.append(token);
		r.append(" ");
		if (token.compare("(") == 0) {
			++parenthesis;
		} else if (token.compare(")") == 0) {
			--parenthesis;
			if (parenthesis == token_parenthesis - 1) {
				break;
			}
		} else if (file.eof()) {
			return false;
		}

	}
	ret = r;

	return true;
}

static
bool getBracketAfter(std::istream &file, const std::string &from,
		unsigned int number, std::string& ret) {
//	std::cout << "getBracket2" << std::endl;
	if (!file.good()) {
//		std::cout << "error on file" << std::endl;
		file.clear();
	}
	file.seekg(0, std::ios_base::beg);

	std::string r = "";
	std::string token;
	int parenthesis = 0;
	int token_parenthesis;

//	std::cout << "start" << std::endl;

	while (file.good()) {
		file >> token;
//		std::cout << "token = " << token;
		if (token.compare(from) == 0) {
			token_parenthesis = parenthesis;
			break;
		} else if (token.compare("(") == 0) {
			++parenthesis;
		} else if (token.compare(")") == 0) {
			--parenthesis;
		} else if (file.eof()) {
			return false;
		}
	}

//	std::cout << token;

//	r.append(from);
//	r.append(" ");
//	file >> token;

	while (file.good()) {
		file >> token;
		r.append(token);
		r.append(" ");
		if (token.compare("(") == 0) {
			++parenthesis;
		} else if (token.compare(")") == 0) {
			--parenthesis;
			if (parenthesis == token_parenthesis) {
				break;
			}
		} else if (file.eof()) {
			return false;
		}
	}
	ret = r;

	return true;
}

static std::string gulp(std::istream &in) {
	std::string ret;
	char buffer[4096];
	while (in.read(buffer, sizeof(buffer)))
		ret.append(buffer, sizeof(buffer));
	ret.append(buffer, in.gcount());
	return ret;
}

static
bool getText3(std::istream &file, const std::string &from, std::string to,
		unsigned int number, std::string& ret) {
	if (!file.good()) {
//		std::cout << "error on file" << std::endl;
		file.clear();
	}
	file.seekg(0, std::ios_base::beg);

	std::string text = gulp(file);
	file.seekg(0, std::ios_base::beg);

	size_t s_from = 0;
	unsigned int n = 0;

	while (n <= number) {
		s_from = text.find(from, s_from);
		if (s_from != text.npos) {
			++n;
		} else {
			return false;
		}
	}

	size_t s_to = text.find(to, s_from);
	if (s_to == text.npos) {
		return false;
	}

	std::string f = text.substr(s_from, s_to - s_from);
	ret = f;
	return true;
}

static
bool getText2(std::istream &file, std::string from, std::string to,
		unsigned int number, std::string& ret) {
	if (!file.good()) {
//		std::cout << "error on file" << std::endl;
		file.clear();
	}
	file.seekg(0, std::ios_base::beg);

	std::string str;
	size_t pos;
	std::vector<std::string> strings;
	bool good = false;

	unsigned int counter = 0;
	// put total_text the text for inital state.

	// find init
	while (file.good()) {
		file >> str;
		pos = str.find(from);
		if (pos != std::string::npos) {
			if (number == counter) {
				strings.push_back(str);
				break;
			} else {
				++counter;
			}
		}
	}

	while (file.good()) {
		file >> str;
		pos = str.find(to);
		if (pos != std::string::npos) {
			good = true;
			break;
		}
		if (file.good()) {
			strings.push_back(str);
		}
	}

	ret = "";
	for (int i = 0; i < strings.size(); ++i) {
		ret.append(strings[i]);
		ret.append(" ");
	}
	std::transform(ret.begin(), ret.end(), ret.begin(), ::tolower);

//	std::cout << "getText: [" << ret << "]" << std::endl;
	return good && file.good();
}

static
bool getText(std::istream &file, std::string from, std::string to,
		unsigned int number, std::string& ret) {
	if (!file.good()) {
//		std::cout << "error on file" << std::endl;
		file.clear();
	}
	file.seekg(0, std::ios_base::beg);

	std::string total_text;
	std::string line;
	size_t pos;
	std::vector<std::string> strings;

	unsigned int counter = 0;
	// put total_text the text for inital state.

	// find init
	while (file.good()) {
		getline(file, line); // get line from file
		std::transform(line.begin(), line.end(), line.begin(), ::tolower);
		pos = line.find(from); // search
		if (pos != std::string::npos) {
			++counter;
//			std::cout << "match = " << line << std::endl;
			if (counter > number) {
				total_text = line;
//				std::cout << "take" << std::endl;
				break;
			} else {
//				std::cout << "throw" << std::endl;
			}
		}
	}

//	std::cout << "init text = " << total_text << std::endl;

	// append until goal.
	while (file.good()) {
		getline(file, line); // get line from file
		std::transform(line.begin(), line.end(), line.begin(), ::tolower);
		pos = line.find(to); // search
		if (pos != std::string::npos) {
			break;
		} else {
			total_text.append(line);
		}
	}

	std::transform(total_text.begin(), total_text.end(), total_text.begin(),
			::tolower);

	if (total_text.empty()) {
		ret = total_text;
		return false;
	}
	ret = total_text;
	return true;
}

// read all and then
// 1. replace ";" -> comment out
// 2. replace "("  -> " ( "
// 3. replace ")"  -> " ) "

static std::string readAll(std::istream& f) {
	std::string allin;
	std::string input;
	while (std::getline(f, input)) {
		// comment out
		size_t c = input.find(";");
		if (c != input.npos) {
			input = input.substr(0, c);
		}
		allin.append(input);
		allin.append("\n");
	}
	replace(allin, "(", " ( ");
	replace(allin, ")", " ) ");
	transform(allin.begin(), allin.end(), allin.begin(), ::tolower);

	return allin;
}

#endif
