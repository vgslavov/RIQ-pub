#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <hashlibpp.h>

#include "lsh.h"
#include "utils.h"

using namespace std;

// determine min element value of a vector
POLY LSH::find_min(vector<POLY> v)
{
    POLY min = v[0];

    for (unsigned int i = 0; i < v.size(); i++) {
        if (v[i] < min) {
            min = v[i];
        }
    }
    return min;
}

vector<string> LSH::get_hash_str(vector<POLY> S)
{
  vector<POLY> h;
  vector<POLY> min_hash;
  vector<string> pre_hash;
  vector<string> hash;

  int f = 0;
  for (int z = 0; z < lshk; z++) {
    for (int j = 0; j < lshl; j++) {
      for (int i = 0; i < S.size(); i++) {
        POLY htmp = (randa[f] * S[i] + randb[f]) % lshp;
        h.push_back(htmp);
      }
      //POLY min = find_min(h);
      // returns an iterator
      POLY min = *min_element(h.begin(), h.end());
      min_hash.push_back(min);  
      h.clear();
      ++f;
    }
    string temp;
    stringstream ss;
    for (unsigned int t = 0; t < min_hash.size(); t++) {
      ss.str("");
      ss << min_hash[t];
      temp += ss.str();
    }
    
    ss.str("");
    pre_hash.push_back(temp);
    temp.clear();
    min_hash.clear();
    // only 1st column is needed (faster)
    if (only1stcol) break;
  }

  string ID;
  const char *p;
  //cout << "hash IDs:\n";
  for (unsigned int u = 0; u < pre_hash.size(); u++) {
    p = pre_hash[u].c_str();

    ID = to_string(hashIrrPoly(p, irrpoly));

    //SHA-1 library addition
    /*
    hashwrapper *myWrapper = new sha1wrapper();
    try {
       ID = myWrapper->getHashFromString(p);
    } catch(hlException &e) {
       cerr << "Error(" << e.error_number() << "): " << endl;
    }
    delete myWrapper;
    */

    //cout << ID << "\n";
    hash.push_back(ID);
  }
  return hash;
}

vector<POLY> LSH::get_hash_poly(vector<POLY> S)
{
  vector<POLY> h;
  vector<POLY> min_hash;
  vector<string> pre_hash;
  vector<POLY> hash;

  int f = 0;
  for (int z = 0; z < lshk; z++) {
    for (int j = 0; j < lshl; j++) {
      for (int i = 0; i < S.size(); i++) {
        POLY htmp = (randa[f] * S[i] + randb[f]) % lshp;
        h.push_back(htmp);
      }
      //POLY min = find_min(h);
      // returns an iterator
      POLY min = *min_element(h.begin(), h.end());
      min_hash.push_back(min);  
      h.clear();
      ++f;
    }
    string temp;
    stringstream ss;
    for (unsigned int t = 0; t < min_hash.size(); t++) {
      ss.str("");
      ss << min_hash[t];
      temp += ss.str();
    }
    
    ss.str("");
    pre_hash.push_back(temp);
    temp.clear();
    min_hash.clear();
    // only 1st column is needed (faster)
    if (only1stcol) break;
  }

  POLY ID;
  const char *p;
  //cout << "hash IDs:\n";
  for (unsigned int u = 0; u < pre_hash.size(); u++) {
    p = pre_hash[u].c_str();

    ID = hashIrrPoly(p, irrpoly);

    //SHA-1 library addition
    /*
    hashwrapper *myWrapper = new sha1wrapper();
    try {
       ID = myWrapper->getHashFromString(p);
    } catch(hlException &e) {
       cerr << "Error(" << e.error_number() << "): " << endl;
    }
    delete myWrapper;
    */

    //cout << ID << "\n";
    hash.push_back(ID);
  }
  return hash;
}
