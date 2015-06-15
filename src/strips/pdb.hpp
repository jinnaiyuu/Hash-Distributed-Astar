#ifndef PDB_HPP_
#define PDB_HPP_

#include "utils.hpp"

#include <vector>
#include <utility>
#include <fstream>
#include <iterator>
#include <algorithm>

/**
 * pattern database to store abstract state and its
 *
 */
class PDB {
	const unsigned int TRUE_PREDICATE = 10000000;
public:
	PDB() {
	}

	void addPattern(unsigned int pattern, int h) {
//		std::pair<std::vector<unsigned int>, int> n(pattern, h);
		if (database.size() < pattern + 1) {
			database.resize(pattern + 1);
		}
		database[pattern] = h;
	}

//	void addPattern(std::vector<unsigned int> pattern, int h) {
//		std::pair<std::vector<unsigned int>, int> n(pattern, h);
//		database.push_back(n);
//	}

	void setGroups(std::vector<std::vector<unsigned int>> groups) {
		this->groups = groups;
	}

	int heuristic(const std::vector<unsigned int>& state) const {
		// 1. find abstract state representation
		// 2. look up database.
		// TODO: can be optimized to trie structure or some kind of perfect hash.
		// TODO: add implementation of true or else.
		//       this can be implement just by not putting anything after.

		// Here, we need a perfect hash for this pattern database.
		// Empirically, uint64 is way too enough to make it perfect hash.
		unsigned int arg_pat = 0;
//		std::cout << "ps = ";
//		for (int gs = 0; gs < groups.size(); ++gs) {
		for (int gs = groups.size() - 1; gs >= 0; --gs) {
			for (int ps = 0; ps < groups[gs].size(); ++ps) {
				if (isContainedSortedVectors(groups[gs][ps], state)) {
//					std::cout << groups[gs][ps] << " ";
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
		return database[arg_pat];

//		return database[arg_pat].second;

//		std::cout << ": " << arg_pat << " = " << database[arg_pat].second
//				<< std::endl;

//		std::cout << "bf = ";
//		for (int i = 0; i < database.size(); ++i) {
//			if (isContainedSortedVectors(database[i].first, state)) {
//				for (int j = 0; j < database[i].first.size(); ++j) {
//					std::cout << database[i].first[j] << " ";
//				}
//				std::cout << ": " << i << " = " << database[i].second
//						<< std::endl;
//				return database[i].second;
//			}
//		}
	}

	// TODO: for experiment, let's reuse pattern database for same problem.
	// TODO: read from file.
	void dump_all(std::string name) {
		std::ofstream file;
		std::string fname = "../pdb/" + name;
		file.open(fname.c_str());

		for (int gs = 0; gs < groups.size(); ++gs) {
			for (int ps = 0; ps < groups[gs].size(); ++ps) {
				file << groups[gs][ps] << " ";
			}
			file << std::endl;
		}
		file << "-1" << std::endl;

		for (int i = 0; i < database.size(); ++i) {
			file << database[i] << " ";

			//			for (int p = 0; p < database[i].first.size(); ++p) {
//				file << database[i].first[p] << " ";
//			}
//			file << "-1 " << database[i].second << std::endl;
		}
		file.close();
	}

	// structure of the input:
	//
	// p0 p1 p2 -1 h
	// p0 p1 p2 -1 h

	// TODO: way too slow.
	//       fastest reading method is fscan(). let's refactor to it.
	bool read_database(std::string name) {
		std::string fname = "../pdb/" + name;
		std::ifstream infile(fname);
//		std::ios_base::sync_with_stdio(false);
//		std::istream_iterator<int> start(infile), end;
//		std::istream_iterator<unsigned int> start(infile);

		if (!infile.good()) {
			std::cout << "no database available." << std::endl;
			return false;

		}

		std::cout << "reading database..." << std::endl;
//		std::cout << "is it working? " << std::endl;

		// 1. get groups
		std::string line = "";

		unsigned int size = 1;
		while (!infile.eof()) {
//			std::cout << "line: " << line;
			std::getline(infile, line);
//			std::cout << "line: " << line << " ";
			std::stringstream iss(line);
			std::vector<unsigned int> g;
			int input;
			iss >> input;
//			std::cout << "in: " << input;

			//			++start;
			if (input == -1) {
				break;
			}

			while (!iss.eof()) {
				g.push_back(input);
				iss >> input;
//				++start;
			}
			groups.push_back(g);
			size *= g.size();
//			std::cout << size << " patterns to read..." << std::endl;
		}
		std::cout << size << " patterns to read...";


		// g0 g1 g2 ... gn -1 heuristic

		// 2. get
		std::istream_iterator<unsigned int> start(infile);


		database = std::vector<unsigned int>((start), std::istream_iterator<unsigned int>());
//		std::vector<unsigned int> d = std::vector<unsigned int>((start), std::istream_iterator<unsigned int>());
//		std::copy(d.begin(), d.end(), database.begin());
//		database(d);

//		while(!infile.eof()) {
//			unsigned int h;
//			infile >> h;
//			database.push_back(h);
//		}
//		while (std::getline(infile, line)) {
//			std::stringstream iss(line);
//			std::vector<unsigned int> pat;
//			int h;
//			int input;
//			bool isPat = true;
//			while (!iss.eof()) {
//				iss >> input;
//				if (isPat) {
//					if (input == -1) {
//						isPat = false;
//					} else {
//						pat.push_back(input);
//					}
//				} else {
//					h = input;
//				}
//			}
//			std::pair<std::vector<unsigned int>, int> p(pat, h);
//			database.push_back(p);
//		}
		std::cout << "done!" << std::endl;
		return true;

	}

private:
	// TODO: trie structure?
	// for now let's implement this as a vector.
	// not sure what makes it slow/fast before implementing.
	// std::vector<unsigned int> -> int

	// Database:
	// pair: abstract state <-> heuristic
	std::vector<unsigned int> database;
//	std::vector<std::pair<std::vector<unsigned int>, int> > database;
	std::vector<std::vector<unsigned int>> groups;
};

#endif
