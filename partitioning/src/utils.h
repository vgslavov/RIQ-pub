// Author: Praveen Rao
// UMKC/UArizona, 2008
#ifndef _UTILS_H_
#define _UTILS_H_

#include <string>
#include <vector>
#include "poly.h"

//using namespace std;
std::string cleanString(std::string &);
double getgtod();
const int MAXKEYLEN = 1028;

// Polynomial related
const POLY testPoly = (1 << MAXMODDEGREE);
const POLY ONEFLAG = 0xffffffff;
const POLY lowerBitsMask = ONEFLAG;

POLY modularMultPoly(POLY, POLY, IRRPOLY);
POLY findMod(void *, int, IRRPOLY);
POLY remainder(std::vector<POLY> &, POLY);
	
int getDegree(IRRPOLY);
int getDegree(const std::vector<POLY>&);
void multiplyPoly(std::vector<POLY>&, POLY);
void multiplyPoly(std::vector<POLY>&, const std::vector<POLY>&, POLY);
void multiplyPoly(std::vector<POLY>&, const std::vector<POLY>&, const std::vector<POLY>&);

void addPoly(std::vector<POLY>&, const std::vector<POLY>&);

void shiftLeft(std::vector<POLY>&, int);
void shiftRight(std::vector<POLY>&, int);

void remainder(std::vector<POLY>&, const std::vector<POLY>&, const std::vector<POLY>&);
void gcd(std::vector<POLY>&, const std::vector<POLY>&, const std::vector<POLY>&);
void gcdSpecial(std::vector<POLY>&, const std::vector<POLY>&, const std::vector<POLY>&);
void lcm(std::vector<POLY>&, const std::vector<POLY>&);
int randomNumGen(int);
int hashString(const char *);
int hashIrrPoly(const char *input, POLY irrPoly);
#endif
