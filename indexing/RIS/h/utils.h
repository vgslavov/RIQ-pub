// Author: Praveen Rao
#ifndef _UTILS_H_
#define _UTILS_H_

#include <string>
#include <vector>
#include "poly.h"
#include "nodeparams.h"

//using namespace std;
std::string cleanString(std::string &);
double getgtod();
const int MAXKEYLEN = 1028;
const int PAGESIZE = 8192;
const int MAXBUFS = 2000;

// filter index paths
const char *const BLOOMCOUNTERSDIR = "CBFS";
const char *const GRAPHIDSDIR = "GRAPHIDS";
const char *const UNIONSIGSDIR = "UNIONSIGS";

// CBFs
enum DabloomsType {
  DABF = 1,
  DACBF = 2,
  DASCLCBF = 3
};

// PV mapping
const int SPOLISTS = 7;
//enum SPOPVS {
enum {
  SPO_LIST = 0,
  SP_LIST = 1,
  SO_LIST = 2,
  PO_LIST = 3,
  S_LIST = 4,
  P_LIST = 5,
  O_LIST = 6
};

enum SpoOrderType {
  MIN_ORDER = 0,
  MAX_ORDER = 1,
  DEF_ORDER = 2
};

#define NODEID int
// Distinct tags
const char *const TAGFILE = ".tags";
const char *const PATHTAGFILE = ".ptags";
const char *const TAGDEPTH = ".tdepth";
const char *const BEGINTAG = "--BEGINTAG--";
const char *const ENDTAG = "--ENDTAG--";
const char *const PATHFILE = ".path";
const char *const HISTFILE = ".hist";
const char *const TEXTHASHFILE = ".texthash";
const char *const VALFILE = ".value";

// Polynomial related
const POLY testPoly = (1 << MAXMODDEGREE);
const POLY ONEFLAG = 0xffffffff;
const POLY lowerBitsMask = ONEFLAG;

POLY modularMultPoly(POLY, POLY, IRRPOLY);
POLY findMod(void *, int, IRRPOLY);
POLY findMod(const void *, int, POLY, int);
int hashIrrPoly(const char *, POLY);
POLY remainder(std::vector<POLY> &, POLY);
void remainder(std::vector<POLY>&, const std::vector<POLY>&, const std::vector<POLY>&);
void remainderNew(std::vector<POLY>&, const std::vector<POLY>&, const std::vector<POLY>&);

int getDegree(IRRPOLY);
int getDegree(const std::vector<POLY>&);
int getDegreeNew(const std::vector<POLY>&);
int getDegreeNew(const std::vector<std::vector<POLY> >&);
void multiplyPoly(std::vector<POLY>&, POLY);
void multiplyPoly(std::vector<POLY>&, const std::vector<POLY>&, POLY);
void multiplyPoly(std::vector<POLY>&, const std::vector<POLY>&, const std::vector<POLY>&);

void addPoly(std::vector<POLY>&, const std::vector<POLY>&);

void shiftLeft(std::vector<POLY>&, int);
void shiftRight(std::vector<POLY>&, int);


void gcd(std::vector<POLY>&, const std::vector<POLY>&, const std::vector<POLY>&);
void gcdNew(std::vector<POLY>&, const std::vector<POLY>&, const std::vector<POLY>&);
void gcdNew(std::vector<std::vector<POLY> >&, const std::vector<std::vector<POLY> >&, const std::vector<std::vector<POLY> >&);
void gcdSpecial(std::vector<POLY>&, const std::vector<POLY>&, const std::vector<POLY>&);
void lcm(std::vector<POLY>&, const std::vector<POLY>&);
void lcmNew(std::vector<POLY>&, const std::vector<POLY>&, const std::vector<POLY>&);
void lcmNew(std::vector<std::vector<POLY> >&, const std::vector<std::vector<POLY> >&, const std::vector<std::vector<POLY> >&);
void lcmNew(std::vector<POLY>&, const std::vector<POLY>&);
void lcmNew(std::vector<std::vector<POLY> >&, const std::vector<std::vector<POLY> >&);
void lcmGroup(std::vector<std::vector<POLY> >&, const std::vector<std::vector<std::vector<POLY> > >&);
void lcmGroup(std::vector<POLY>&, const std::vector<std::vector<POLY> >&);

// Interval for a node
struct Interval {
	int ln, ld, rn, rd;
	int level;
  int random;
	Interval() {
		ln = 0;
		ld = 1;
		rn = 1;
		rd = 1;
	}

    // == operator
    bool operator==(const Interval& testInt) {
        if (ln == testInt.ln && ld == testInt.ld &&
            rn == testInt.rn && rd == testInt.rd &&
            level == testInt.level) {
            return true;
        }
        else
            return false;
    }

    // != operator
    bool operator!=(const Interval& testInt) {
        if (ln == testInt.ln && ld == testInt.ld &&
            rn == testInt.rn && rd == testInt.rd &&
            level == testInt.level) {
            return false;
        }
        else
            return true;
    }

};

void makeKeyValue(char **ptr, int& len, std::vector<POLY>& textHash,
    std::vector<POLY>& sig, int docId, InsertType type);
void makeDocid(char **ptr, int *ptrlen, std::vector<POLY>& sig, char *docid);

void cleanup(char *);
void readTags(FILE *, std::vector<std::string>&);

void updateMBR(std::vector<POLY>&, std::vector<POLY>&);
bool isValue(std::string& input);
void lcmSpecial(std::vector<POLY>& t, const std::vector<POLY>& s, std::vector<POLY>& g);

void getKeyValue(const char *buf, int& docid, std::vector<POLY>& sig);
void getKeyValue(const char *buf, int& docid, std::vector<std::vector<POLY> >& sig);
void makeKeyValue(char **ptr, int&, std::vector<POLY>&, int);
void makeKeyValue(char **ptr, int&, std::vector<std::vector<POLY> >&, int);

void getKeyValue(const char *buf, char *docid, std::vector<POLY>& sig);
void getKeyValue(const char *buf, char *docid, std::vector<std::vector<POLY> >& sig);
void makeKeyValue(char **ptr, int&, std::vector<POLY>&, char *);
void makeKeyValue(char **ptr, int&, std::vector<std::vector<POLY> >&, char *);

// BEGIN FOR VASIL

bool compressSignatures(std::vector<std::vector<POLY> >&, 
			std::vector<POLY>&, std::vector<std::vector<unsigned char> >&);
bool uncompressSignatures(std::vector<std::vector<POLY> >&, 
			  std::vector<POLY>&, std::vector<std::vector<unsigned char> >&, int);

void makeKeyValue(char **, std::vector<POLY>&, 
		  std::vector<std::vector<unsigned char> >&, int);

void getKeyValue(const char *, std::vector<POLY>&, 
		 std::vector<std::vector<unsigned char> >&, int&);

#endif
