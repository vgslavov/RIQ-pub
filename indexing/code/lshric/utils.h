/*	$Id: utils.h,v 1.2 2012/08/24 16:16:34 vsfgd Exp $	*/

// Author: Praveen Rao
//#ifndef _UTILS_H_
//#define _UTILS_H_

#include <string>
#include <vector>
#include <map>
#include <fstream>

//#include "../tools/poly.h"
//#include "../tools/nodeparams.h"
//#include "../tools/retrievetypes.h"

typedef unsigned int POLY;
typedef unsigned int IRRPOLY;
const int MAXMODDEGREE = (8*sizeof(IRRPOLY))-1;

//const int MAXKEYLEN = 1028;
//const int PAGESIZE = 8192;
//const int MAXBUFS = 2000;

// Polynomial related
const POLY testPoly = (1 << MAXMODDEGREE);
const POLY ONEFLAG = 0xffffffff;
const POLY lowerBitsMask = ONEFLAG;

// dp244: all LSH code
class lsh {
	public:
	// constructor
	// k = number of elements in the signature
	// l = number of independent hash functions in each group (r rows)
	// m = number of groups (b bands)
	// n = random number seed
	lsh(int kc, int lc, int mc, int nc, std::vector<POLY> irrnumbers, std::vector<int> hasha, std::vector<int> hashb, int colc = 0) {
		k = kc;
		l = lc;
		m = mc;
		n = nc;
		col = colc;

		// loading poly's
		/*
		std::ifstream polystream;
		polystream.open(irrfile.c_str());
		POLY number;
		std::vector<POLY> numbers;
		while (polystream >> number) {
			numbers.push_back(number);
		}
		polystream.close();
		*/

		//randomly choose a poly
		srand(n);
		int range = irrnumbers.size() + 1;
		int rand_index = int((double)range*rand()/(RAND_MAX + 1.0));

		std::cout << "Rand_index: " << rand_index << std::endl;

		selected_poly = irrnumbers[rand_index];
		//irrpolyf = irrfile;
		//hashf = hashfile;

		// loading hash funcs
		highest = n;
	        /*	
		std::ifstream hashstream;
		hashstream.open(hashfile.c_str());
		int hashnum;
		randa.clear();
		randb.clear();
		int z = 0;
		while (hashstream >> hashnum) {
			++z;
			if (z <= l)
				randa.push_back(hashnum);
			else
				randb.push_back(hashnum);
		}
		assert(randa.size() == randb.size());
		assert((int)randa.size() == l);
		hashstream.close();
	        */	
		irrnums.clear();
		randa.clear();
		randb.clear();
		irrnums = irrnumbers;
		randa = hasha;
		randb = hashb;
	}

	// destructor
	~lsh() {};
	std::vector<POLY> getHashCodeFindMod(std::vector<POLY>& S,POLY polNumber);
	std::vector<std::string> getHashCode(std::vector<POLY>& S);
	POLY isMinimum(std::vector<POLY>& v);
	POLY getIRRPoly();
        //std::vector<POLY> getUniqueSet(std::vector<POLY> inputPols);
	// obsolete: instead give prime number when you create LSH object
	//int getPrimeNumberFromConfig(char* configFileName);

	// private variables
	private:
	int k;
	int l;
	int m;
	int n;
	int col;
        POLY selected_poly;
	//std::string irrpolyf;
	//std::string hashf;
	std::vector<POLY> irrnums;
	std::vector<int> randa;
	std::vector<int> randb;
	int highest;
};

// vsfgd
// copied from psi.C
struct CompareSig
{
	bool operator()(const std::vector<POLY>& s1, const std::vector<POLY>& s2) const
	{
		// Return TRUE if s1 < s2
		//warnx << "S1 size: " << s1.size() << " S2 size: " << s2.size() << "\n";
		if (s1.size() < s2.size()) return true;
		else if (s1.size() == s2.size()) {
			for (int i = 0; i < (int) s1.size(); i++) {
				if (s1[i] < s2[i]) return true;
				else if (s1[i] > s2[i]) return false;
			}
			return false;
		} else return false;
	}
};

void multiplyPoly(std::vector<POLY>&, POLY);
void multiplyPoly(std::vector<POLY>&, const std::vector<POLY>&, POLY);
void multiplyPoly(std::vector<POLY>&, const std::vector<POLY>&, const std::vector<POLY>&);
double getgtod();
int getDegree(IRRPOLY);
POLY findMod(void *, int, IRRPOLY);
POLY findMod(const void *, int, POLY, int);
int hashIrrPoly(const char *input, POLY irrPoly);

// bloom
std::string cleanString(std::string &);
POLY modularMultPoly(POLY, POLY, IRRPOLY);
POLY remainder(std::vector<POLY> &, POLY);
int getDegree(const std::vector<POLY>&);
void addPoly(std::vector<POLY>&, const std::vector<POLY>&);

void shiftLeft(std::vector<POLY>&, int);
void shiftRight(std::vector<POLY>&, int);

void remainder(std::vector<POLY>&, const std::vector<POLY>&, const std::vector<POLY>&);
void gcd(std::vector<POLY>&, const std::vector<POLY>&, const std::vector<POLY>&);
void gcdSpecial(std::vector<POLY>&, const std::vector<POLY>&, const std::vector<POLY>&);
void lcm(std::vector<POLY>&, const std::vector<POLY>&);
int randomNumGen(int);
int hashString(const char *);
void quotient(std::vector<POLY>&, const std::vector<POLY>&, const std::vector<POLY>&);
