// Author: Praveen Rao
// UMKC/UArizona, 2008
#ifndef _UTILS_H_
#define _UTILS_H_

#include <string>
#include <vector>
#include "poly.h"

//using namespace std;
const int MAXKEYLEN = 1028;

// Polynomial related
const POLY testPoly = (1 << MAXMODDEGREE);
const POLY ONEFLAG = 0xffffffff;
const POLY lowerBitsMask = ONEFLAG;

int getDEGREE(IRRPOLY);
int hashIrrPoly(const char *input, POLY irrPoly);
#endif
