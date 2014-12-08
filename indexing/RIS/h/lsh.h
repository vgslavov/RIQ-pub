#ifndef _LSH_H_
#define _LSH_H_

#include <string>
#include <vector>

#include "poly.h"

using namespace std;

class LSH
{
  public:
    // 1 - (1 - s^l)^k
    // h(x) = (ax + b) mod p
    LSH (int k, int l, int p, POLY irr, vector<int> a, vector<int> b, bool faster = false)
    {
      lshk = k;
      lshl = l;
      lshp = p;
      irrpoly = irr;
      randa = a;
      randb = b;
      only1stcol = faster;
    }

    ~LSH() {};
    vector<string> get_hash_str(vector<POLY> S);
    vector<POLY> get_hash_poly(vector<POLY> S);
    POLY find_min(vector<POLY> v);

  private:
    int lshk;
    int lshl;
    int lshp;
    POLY irrpoly;
    vector<int> randa;
    vector<int> randb;
    bool only1stcol;
};

#endif
