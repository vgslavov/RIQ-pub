// Author: Praveen Rao
#include "utils.h"
#include <iostream>
#include <sys/time.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <string>
#include <iostream>
#include <vector>
#include <map>
#include <algorithm>

bool isStrValue(std::string& input)
{
  for (int i = 0; i < (int) input.size(); i++) {
    if (input.at(i) > '9' || input.at(i) < '0') {
      return false;
    }
  }
  return true;
}

void readTags(FILE *fp, std::vector<std::string>& distinctTags)
{
  char buf[128];
  bool start = false, end = false;
	
  while (1) {
    if (fscanf(fp, "%s", buf) == EOF) break;

    if (strcmp(buf, ENDTAG) == 0) {
      end = true;
      break;
    }
    else if (strcmp(buf, BEGINTAG) == 0) {
      start = true;
      continue;
    }
    else {
      distinctTags.push_back(std::string(buf));
    }
  }

  assert(start && end);
  return;
}

// Cleanup a string by deleting special characters
std::string cleanString(std::string &s)
{
  std::string res = s;

  bool flag = false;
  for (int i = (int) s.length()-1; i >= 0; i--) {
    switch(res[i]) {
    case ' ':
    case '\t':
    case '\n':
    case '\r':
      res.erase(i, 1);
      break;
    default:
      flag = true;
			
    }

    if (flag) {
      break;
    }
  }

  flag = false;
  for (int i = 0; i < (int) res.length(); ) {
    switch(res[i]) {
    case ' ':
    case '\t':
    case '\n':
    case '\r':
      res.erase(i, 1);
      break;
    default:
      flag = true;
			
    }

    if (flag) {
      break;
    }
  }

  return res;
}


double getgtod()
{
  static struct timeval gtod_time;
  gettimeofday(&gtod_time, NULL);
  return (double)gtod_time.tv_sec
    + (double)gtod_time.tv_usec / CLOCKS_PER_SEC;
}


// Polynomial processing functions!
// Multiply the multiplicand with the multiplier
void multiplyPoly(std::vector<POLY>& multiplicand, POLY multiplier)
{
  unsigned long long product = 0;

  POLY x;
  unsigned long long y;

  for (int i = multiplicand.size() - 1; i >= 0; i--) {
    x = multiplier;
    y = multiplicand[i];
    while (x > 0) {
      if (x & 1 > (POLY) 0) {
	product = product ^ y;
      }
      x = x >> 1;
      y = y << 1;
    }
    // copy the lower bit set to the result
    multiplicand[i] = product & lowerBitsMask;
    product = product >> (sizeof(POLY) * 8);
  }
	
  if (product > 0) {
    multiplicand.insert(multiplicand.begin(), product & lowerBitsMask);
  }
}

// Polynomial processing functions!
// Multiply the multiplicand with the multiplier
void multiplyPoly(std::vector<POLY>& res, const std::vector<POLY>& multiplicand,
		  POLY multiplier)
{
  unsigned long long product = 0;

  assert(res.size() == multiplicand.size());
	
  POLY x;
  unsigned long long y;

  for (int i = multiplicand.size() - 1; i >= 0; i--) {
    x = multiplier;
    y = multiplicand[i];
    while (x > 0) {
      if (x & 1 > (POLY) 0) {
	product = product ^ y;
      }
      x = x >> 1;
      y = y << 1;
    }
    // copy the lower bit set to the result
    //res.insert(res.begin(), product & lowerBitsMask);
    res[i] = product & lowerBitsMask;
    //multiplicand[i] = product & lowerBitsMask;
    product = product >> (sizeof(POLY) * 8);
  }
	
  if (product > 0) {
    //multiplicand.insert(multiplicand.begin(), product & lowerBitsMask);
    res.insert(res.begin(), product & lowerBitsMask);
  }
}

// Compute x * y mod z
POLY modularMultPoly(POLY x, POLY y, IRRPOLY z)
{
#ifdef _DEBUG_
  cout << "Args: " << x << " " << y << " " << z << endl;
#endif
	
  POLY res = 0;
  POLY x1 = x;

  POLY y1 = y;
  POLY modMask = (z ^ testPoly);

  while (y1 > (POLY) 0) {
    if (y1 & 1 > (POLY) 0) {
      res = res ^ x1;
    }

    x1 = x1 << 1;

    // need casting
    if ((x1 & testPoly) > (POLY) 0) {
      x1 = (x1 ^ testPoly) ^ modMask;
    }
				
    y1 = y1 >> 1;
  }

  return res;
}

// Compute x mod z
POLY findMod(void *x, int len, IRRPOLY z)
{
  unsigned char *ptr = (unsigned char *) x;
 
  unsigned char bitMask = 1;
  int zDegree = getDegree(z);
#ifdef _DEBUG_
  cout << "POLY degree: " << zDegree << endl;
#endif
  POLY testMask = (1 << zDegree);
  POLY modMask = (z ^ (1 << zDegree));

  POLY res = 0;
  POLY resMask = modMask;
  int k = 0;
#ifdef _DEBUG_
  cout << "LENGTH: " << len << " Z: " << z << endl;
#endif

  POLY smallBitsMask = 1;
  for (int i = 0; i < len; i++) {
    for (int j = 0; j < 8; j++) {
      if (k >= zDegree) {
	if ((POLY) (*ptr & bitMask) > (POLY) 0) {
	  res = res ^ resMask;
	}
	resMask = resMask << 1;
	if ((POLY) (resMask & testMask) > (POLY) 0) {
	  resMask = (resMask ^ testMask) ^ modMask;
	}
      }
      else {
	if ((POLY) (*ptr & bitMask) > (POLY) 0) {
	  res = res | smallBitsMask;
	}
	smallBitsMask = smallBitsMask << 1;
      }
      k++;
      bitMask = bitMask << 1;
    }
			
    ptr += 1;
    bitMask = 1;
  }
#ifdef _DEBUG_
  cout << "TEXT mod: " << (char *) x << "=" << res << endl;
#endif
  return res;
}

// Compute x mod z
POLY findMod(const void *x, int len, POLY z, int zDegree)
{
	unsigned char *ptr = (unsigned char *) x;
 
	unsigned char bitMask = 1;
#ifdef _DEBUG_
	cout << "POLY degree: " << zDegree << endl;
#endif
	POLY testMask = (POLY) (1 << zDegree);
	POLY modMask = (POLY) (z ^ (1 << zDegree));

	POLY res = 0;
	POLY resMask = modMask;
	int k = 0;
	
#ifdef _DEBUG_
	cout << "LENGTH: " << len << " Z: " << z << endl;
#endif

	POLY smallBitsMask = 1;
	for (int i = 0; i < len; i++) {
    for (int j = 0; j < 8; j++) {
      if (k >= zDegree) {
        if ((POLY) (*ptr & bitMask) > (POLY) 0) {
          res = (POLY) (res ^ resMask);
 				}
				resMask = resMask << 1;
				if ((POLY) (resMask & testMask) > (POLY) 0) {
          resMask = (POLY) ((resMask ^ testMask) ^ modMask);
				}
			}
			else {
				if ((POLY) (*ptr & bitMask) > (POLY) 0) {
					res = (POLY) (res | smallBitsMask);
				}
				smallBitsMask = smallBitsMask << 1;
			}
			k++;
			bitMask = bitMask << 1;
		
		}

    ptr += 1;
		bitMask = 1;
	}
#ifdef _DEBUG_
	cout << "TEXT mod: " << (char *) x << "=" << res << endl;
#endif
	return res;
}

int hashIrrPoly(const char *input, POLY irrPoly)
{
  return findMod(input, strlen(input), irrPoly, getDegree(irrPoly));
}

// Return the remainder of x / y
POLY remainder(std::vector<POLY> &input, POLY z)
{
  POLY x; 
  POLY divisorMask = z;

  int degree = getDegree(z);
  while ((divisorMask & testPoly) == (POLY) 0) {
    divisorMask = divisorMask << 1;
  }

  int len = input.size();
  int i = 0;
  int bitNum = 0;
  int bitLimit = sizeof(POLY) * 8;
  int stopBitNum = bitLimit - degree;

  // Make sure that the MSW and the divisor are aligned with
  // their 1 bits together
  x = input[i];

  bool breakFlag = false;
  while (i < len - 1) {
    while (!(x & testPoly)) {
      x = x << 1;
      bitNum++;
      if (x == (POLY) 0 || bitNum == bitLimit) {
	bitNum = 0;
	i++;
	x = x ^ input[i];
	breakFlag = true;
	break;
      }
    }
			
    if (!breakFlag) {
      x = x ^ divisorMask;
    }
    else {
      breakFlag = false;
    }
  }
	
  while (i < len) {
    while (!(x & testPoly)) {
      x = x << 1;
      bitNum++;
      if (x == (POLY) 0 || bitNum == stopBitNum) {
	i++;
	break;
      }
    }

    if (i < len)
      x = x ^ divisorMask; 
  }
#ifdef _DEBUG_
  cout << "Remainder = " << x << endl;
#endif
  return x;
}


// Returns the degree of the polynomial
int getDegree(POLY z)
{
  POLY mask = (1 << (sizeof(POLY) * 8 - 1));

  int i = sizeof(POLY) * 8 - 1;
  while (mask > (POLY) 0) {
    if ((mask & z) != (POLY) 0) {
      return i;
    }
    i--;
    mask = mask >> 1;
  }
  return i;
}


// Degree of poly
int getDegree(const std::vector<POLY>& myPoly)
{
  if (myPoly.size() == 0) {
    assert(0);
  }
	
  int deg = getDegree(myPoly[0]);
  return ((myPoly.size() - 1) * sizeof(POLY) * 8) + deg;
}

// Degree of poly
int getDegreeNew(const std::vector<POLY>& myPoly)
{
  return MAXMODDEGREE * myPoly.size();
}

// Degree of poly
// sig v2
int getDegreeNew(const std::vector<std::vector<POLY> >& myPoly)
{
  return MAXMODDEGREE * myPoly[0].size();
}

// Shift left
void shiftLeft(std::vector<POLY>& myPoly, int numBits)
{
  if (numBits == 0) return;
  if (numBits == sizeof(POLY) * 8) {
    myPoly.push_back((POLY) 0);
    return;
  }
	
  assert(numBits > 0 && numBits <= (int) sizeof(POLY) * 8);
	
#ifdef _DEBUG_
  // Output before shifting
  cout << "Before left shift: ";
  for (int i = 0; i < myPoly.size(); i++) {
    printf("%x ", myPoly[i]);
  }
  cout << endl;
#endif
	
  POLY mask = (ONEFLAG << (sizeof(POLY) * 8 - numBits));
  POLY prevmsb = 0;
  for (int i = myPoly.size()-1; i >= 0; i--) {
    POLY msb = myPoly[i] & mask;
    msb = msb >> (sizeof(POLY) * 8 - numBits);
    myPoly[i] = myPoly[i] << numBits;
		
    if (i < (int) myPoly.size()-1) {
      myPoly[i] |= prevmsb;
    }
    prevmsb = msb;
  }

  // extra bits
  if (prevmsb > (POLY) 0) {
    myPoly.insert(myPoly.begin(), prevmsb);
  }
	
#ifdef _DEBUG_
  // Output before shifting
  cout << "After left shift: ";
  for (int i = 0; i < myPoly.size(); i++) {
    printf("%x ", myPoly[i]);
  }
  cout << endl;
#endif
	
}

// Shift right
void shiftRight(std::vector<POLY>& myPoly, int numBits)
{
  if (numBits == 0 || numBits == sizeof(POLY) * 8) return;
	
  //assert(numBits > 0 && numBits < (int) sizeof(POLY) * 8);

#ifdef _DEBUG_
  // Output before shifting
  cout << "Before right shift: " << numBits << "bits ";
  for (int i = 0; i < myPoly.size(); i++) {
    printf("%x ", myPoly[i]);
  }
  cout << endl;
#endif
	
  POLY mask = (ONEFLAG >> (sizeof(POLY) * 8 - numBits));
  POLY prevlsb = 0;
  for (int i = 0; i < (int) myPoly.size(); i++) {	
    POLY lsb = myPoly[i] & mask;
    lsb = lsb << (sizeof(POLY) * 8 - numBits);
    myPoly[i] = myPoly[i] >> numBits;

    if (i > 0) {
      myPoly[i] |= prevlsb;
    }
		
    prevlsb = lsb;
		
  }

  if (prevlsb > (POLY) 0) {
    myPoly.push_back(prevlsb);
  }

  // Remove empty leading entries
  if (myPoly.size() > 0 && myPoly[0] == (POLY) 0) {
    myPoly.erase(myPoly.begin());
  }
#ifdef _DEBUG_
  // Output before shifting
  cout << "After right shift: ";
  for (int i = 0; i < myPoly.size(); i++) {
    printf("%x ", myPoly[i]);
  }
  cout << endl;
#endif
}

// GCD for multisets
// sig v1
void gcdNew(std::vector<POLY>& resPoly, const std::vector<POLY>& f, const std::vector<POLY>& g)
{
  int i, j;
  i = 0;
  j = 0;

  while (i < (int) f.size() && j < (int) g.size()) {
    if (f[i] == g[j]) {
      resPoly.push_back(f[i]);
      i++;
      j++;
    }
    else if (f[i] < g[j]) {
      i++;
    }
    else {
      j++;
    }
  }
  return;
}

// GCD for multisets
// sig v2
void gcdNew(std::vector<std::vector<POLY> >& resPoly, const std::vector<std::vector<POLY> >& f, const std::vector<std::vector<POLY> >& g)
{
  int i, j, k;
  int lists = f.size();
  i = 0;
  j = 0;
  k = 0;

  //assert(f[0].size() == g[0].size());
  assert(lists == SPOLISTS);

  resPoly.resize(lists);
  while (k < lists) {
    while (i < (int) f[k].size() && j < (int) g[k].size()) {
      if (f[k][i] == g[k][j]) {
        resPoly[k].push_back(f[k][i]);
        i++;
        j++;
      }
      else if (f[k][i] < g[k][j]) {
        i++;
      }
      else {
        j++;
      }
    }
    k++;
    i = 0;
    j = 0;
  }
  return;
}

// Add a special check to have the larger degree poly on the left
// and smaller degree polynomial on the right.
void gcdSpecial(std::vector<POLY>& resPoly, const std::vector<POLY>& f, const std::vector<POLY>& g)
{
  int degf = getDegree(f);
  int degg = getDegree(g);
#ifdef _DEBUG_
  cout << "DEG f: " << degf
       << " DEG g: " << degg
       << endl;
#endif

  if (degf >= degg) {
    gcd(resPoly, f, g);
  }
  else {
    gcd(resPoly, g, f);
  }
}

// Compute GCD of two polynomials in GF(2)
void gcd(std::vector<POLY>& resPoly, const std::vector<POLY>& f, const std::vector<POLY>& g)
{
  if (g.size() == 1 && g[0] == (POLY) 0) {
    resPoly = f;
  }
  else {
    std::vector<POLY> r;
    remainder(r, f, g);
    gcd(resPoly, g, r);
  }
}

// Multiply two polynomials
void multiplyPoly(std::vector<POLY>& res, const std::vector<POLY>& f,
		  const std::vector<POLY>& g)
{
  std::vector<POLY> partialProd;

  //cout << "prod size: " << partialProd.size() << endl;
  for (int i = 0; i < (int) g.size(); i++) {
    for (int j = 0; j < (int) f.size(); j++) {
      partialProd.push_back((POLY) 0);
    }
																								 
    multiplyPoly(partialProd, f, g[i]);
    addPoly(res, partialProd);

    if (i < (int) g.size() - 1) {
      shiftLeft(res, sizeof(POLY) * 8);
    }

    // clear the list
    partialProd.clear();
  }

}

// Add two polynomials
void addPoly(std::vector<POLY>& x, const std::vector<POLY>& y)
{
  int sizeX, sizeY;
  sizeX = x.size();
  sizeY = y.size();

  int i, j;
	
  for (i = sizeX - 1, j = sizeY - 1; i >= 0 && j >= 0; i--, j--) {
    x[i] ^= y[j];
  }

  for ( ; i < 0 && j >= 0; j--) {
    x.insert(x.begin(), y[j]);
  }
}

// Remainder on multisets
void remainderNew(std::vector<POLY>& r, const std::vector<POLY>& f, const std::vector<POLY>& g)
{
  int i, j;
  i = 0;
  j = 0;

  while (i < (int) f.size() && j < (int) g.size()) {
    if (f[i] == g[j]) {
      r.push_back(f[i]);
      i++;
      j++;
    }
    else if (f[i] < g[j]) {
      i++;
    }
    else {
      j++;
    }
  }
  return;
}


// Remainder of two polynomials
void remainder(std::vector<POLY>& r, const std::vector<POLY>& f, const std::vector<POLY>& g)
{
  assert(f.size() > 0 && g.size() > 0);
	
  int fdeg = getDegree(f[0]);
  int gdeg = getDegree(g[0]);

  std::vector<POLY> gtemp(g);
	
  if (fdeg > gdeg) {
    shiftLeft(gtemp, fdeg - gdeg);	
  }
  else {
    shiftRight(gtemp, gdeg - fdeg);
  }

  POLY bitMask = (1 << fdeg);

  int i = 0;
  int fFullDegree = getDegree(f);
  int gFullDegree = getDegree(g);

  /*
    if (fFullDegree < gFullDegree) {
    cout << "F deg: " << fFullDegree << "-" << "G deg: " << gFullDegree << endl;
    assert(0);
    }
  */
  if (fFullDegree < gFullDegree) { // 2 % 5 = 2
    r = f;
    return;
  }
  //assert(fFullDegree >= gFullDegree);

  // r will contain the remainder eventually.
  r = f;

  for (int k = fFullDegree; k >= gFullDegree; k--) {
    if ((r[i] & bitMask) > (POLY) 0) {
      // Loop thro the polys and XOR each time...
      for (int j = 0; (i + j) < (int) r.size() && (j < (int) gtemp.size()); j++) {
	r[i + j] ^= gtemp[j];
      }

    }

    // Shift right
    // Can be optimized further...
    shiftRight(gtemp, 1);
    bitMask >>= 1;
    if (k % (sizeof(POLY) * 8) == 0) {
      i++;
      bitMask = (1 << (sizeof(POLY) * 8 - 1));
    }
  }

#ifdef _DEBUG_
  cout << "R[] : ";
  for (int i = 0; i < r.size(); i++) {
    cout << r[i] << " ";
  }
  cout << endl;
#endif
	
  // Remove leading empty entries
  int len = r.size() - 1;
  for (int j = 0; j < len; j++) {
    if (r[0] > (POLY) 0) {
      break;	
    }
    r.erase(r.begin());
  }
}

// Remainder of two polynomials
void quotient(std::vector<POLY>& q, const std::vector<POLY>& f, const std::vector<POLY>& g)
{
  assert(f.size() > 0 && g.size() > 0);

  std::vector<POLY> r;
  int fdeg = getDegree(f[0]);
  int gdeg = getDegree(g[0]);

  std::vector<POLY> gtemp(g);
	
  if (fdeg > gdeg) {
    shiftLeft(gtemp, fdeg - gdeg);	
  }
  else {
    shiftRight(gtemp, gdeg - fdeg);
  }

  POLY bitMask = (1 << fdeg);

  int i = 0;
  int fFullDegree = getDegree(f);
  int gFullDegree = getDegree(g);

  /*
    if (fFullDegree < gFullDegree) {
    cout << "F deg: " << fFullDegree << "-" << "G deg: " << gFullDegree << endl;
    assert(0);
    }
  */
  assert(fFullDegree >= gFullDegree);

  // r will contain the remainder eventually.
  r = f;

  // setup the quotient
  q.clear();
  q.push_back((POLY) 0);

  for (int k = fFullDegree; k >= gFullDegree; k--) {
    shiftLeft(q, 1);
    if ((r[i] & bitMask) > (POLY) 0) {
      // Store quotient
      q[q.size()-1] |= (POLY) 1;
      //cout << "Setting bit..." << endl;
      // Loop thro the polys and XOR each time...
      for (int j = 0; (i + j) < (int) r.size() && (j < (int) gtemp.size()); j++) {
	r[i + j] ^= gtemp[j];
      }
    }
		
    // Shift right
    // Can be optimized further...
    shiftRight(gtemp, 1);
    bitMask >>= 1;
    if (k % (sizeof(POLY) * 8) == 0) {
      i++;
      bitMask = (1 << (sizeof(POLY) * 8 - 1));
    }
  }

#ifdef _DEBUG_
  cout << "R[] : ";
  for (int i = 0; i < r.size(); i++) {
    cout << r[i] << " ";
  }
  cout << endl;
#endif

  /*
  // Remove leading empty entries
  int len = r.size() - 1;
  for (int j = 0; j < len; j++) {
  if (r[0] > (POLY) 0) {
  break;	
  }
  r.erase(r.begin());
  }
  */
}

// Compute the LCM assuming multisets
// sig v1
void lcmNew(std::vector<POLY>& f, const std::vector<POLY>& g)
{
  int i, j;
  i = 0;
  j = 0;

  std::vector<POLY> resPoly;
    
  while (i < (int) f.size() && j < (int) g.size()) {
    if (f[i] == g[j]) {
      i++;
      j++;
    }
    else if (f[i] < g[j]) {
      i++;
    }
    else {
      resPoly.push_back(g[j]);
      j++;
    }
  }

  while (i < (int) f.size()) {
    i++;
  }

  while (j < (int) g.size()) {
    resPoly.push_back(g[j]);
    j++;
  }

  // add missing elements
  for (i = 0; i < (int) resPoly.size(); i++) {
    f.push_back(resPoly[i]);
  }

  std::sort(f.begin(), f.end());
    
  return;
}

// Compute the LCM assuming multisets
// sig v2
void lcmNew(std::vector<std::vector<POLY> >& f, const std::vector<std::vector<POLY> >& g)
{
  int i, j, k;
  int lists = f.size();
  i = 0;
  j = 0;
  k = 0;

  //assert(f[0].size() == g[0].size());
  assert(lists == SPOLISTS);

  std::vector<std::vector<POLY> > resPoly(lists);
  while (k < lists) {
    while (i < (int) f[k].size() && j < (int) g[k].size()) {
      if (f[k][i] == g[k][j]) {
        i++;
        j++;
      }
      else if (f[k][i] < g[k][j]) {
        i++;
      }
      else {
        resPoly[k].push_back(g[k][j]);
        j++;
      }
    }

    while (i < (int) f[k].size()) {
      i++;
    }

    while (j < (int) g[k].size()) {
      resPoly[k].push_back(g[k][j]);
      j++;
    }
    k++;
    i = 0;
    j = 0;
  }

  // add missing elements
  for (k = 0; k < lists; k++) {
    for (i = 0; i < (int) resPoly[k].size(); i++) {
      f[k].push_back(resPoly[k][i]);
    }
  }

  for (k = 0; k < lists; k++) {
    std::sort(f[k].begin(), f[k].end());
  }
    
  return;
}

// Compute the LCM assuming multisets
// sig v1
void lcmNew(std::vector<POLY>& resPoly, const std::vector<POLY>& f, const std::vector<POLY>& g)
{
  int i, j;
  i = 0;
  j = 0;

  while (i < (int) f.size() && j < (int) g.size()) {
    if (f[i] == g[j]) {
      resPoly.push_back(f[i]);
      i++;
      j++;
    }
    else if (f[i] < g[j]) {
      resPoly.push_back(f[i]);
      i++;
    }
    else {
      resPoly.push_back(g[j]);
      j++;
    }
  }

  while (i < (int) f.size()) {
    resPoly.push_back(f[i]);
    i++;
  }

  while (j < (int) g.size()) {
    resPoly.push_back(g[j]);
    j++;
  }

  return;
}

// Compute the LCM assuming multisets
// sig v2
void lcmNew(std::vector<std::vector<POLY> >& resPoly, const std::vector<std::vector<POLY> >& f, const std::vector<std::vector<POLY> >& g)
{
  int i, j, k;
  int lists = f.size();
  i = 0;
  j = 0;
  k = 0;

  //assert(f[0].size() == g[0].size());
  assert(lists == SPOLISTS);

  resPoly.resize(lists);
  while (k < lists) {
    assert(f[k].size());
    assert(g[k].size());
    while (i < (int) f[k].size() && j < (int) g[k].size()) {
      if (f[k][i] == g[k][j]) {
        resPoly[k].push_back(f[k][i]);
        i++;
        j++;
      } else if (f[k][i] < g[k][j]) {
        resPoly[k].push_back(f[k][i]);
        i++;
      } else {
        resPoly[k].push_back(g[k][j]);
        j++;
      }
    }

    while (i < (int) f[k].size()) {
      resPoly[k].push_back(f[k][i]);
      i++;
    }

    while (j < (int) g[k].size()) {
      resPoly[k].push_back(g[k][j]);
      j++;
    }
    assert(resPoly[k].size());
    k++;
    i = 0;
    j = 0;
  }

  return;
}

// Compute the LCM of a group of signatures assuming multisets
// sig v1
void lcmGroup(std::vector<POLY>& resPoly, const std::vector<std::vector<POLY> >& listOfSigs)
{
  std::vector<int> idx(listOfSigs.size(), 0);
  std::vector<bool> endOfList(listOfSigs.size(), false);

  while (1) {

    // find the smallest item
    POLY minItem = ONEFLAG;  
    for (int i = 0; i < (int) listOfSigs.size(); i++) {
      if (!endOfList[i] && minItem > listOfSigs[i][idx[i]]) {
	minItem = listOfSigs[i][idx[i]];
      }
    }

    if (minItem == ONEFLAG) break; // EOF of all the lists 

    // update the indexes
    for (int i = 0; i < (int) listOfSigs.size(); i++) {
      if (!endOfList[i] && listOfSigs[i][idx[i]] == minItem) {
	idx[i]++;
	if (idx[i] == listOfSigs[i].size()) {
	  endOfList[i] = true;
	}
      }
    }
    // insert to union
    resPoly.push_back(minItem);  
  }

  return;
}

// Compute the LCM of a group of signatures assuming multisets
// sig v2
void lcmGroup(std::vector<std::vector<POLY> >& resPoly, const std::vector<std::vector<std::vector<POLY> > >& listOfSigs)
{
  int lists = listOfSigs[0].size();
  int mindone = 0;
  std::vector<std::vector<int> > idx;
  std::vector<std::vector<bool> > endOfList;

  assert(lists == SPOLISTS);

  resPoly.resize(lists);
  std::vector<int> tmpi(listOfSigs.size(), 0);
  std::vector<bool> tmpb(listOfSigs.size(), false);
  std::vector<POLY> minItem(lists, ONEFLAG);
  for (int k = 0; k < lists; k++) {
    idx.push_back(tmpi);
    endOfList.push_back(tmpb);
  }

  while (1) {
    mindone = 0;
    // k: signature list
    for (int k = 0; k < lists; k++) {
      // reset minItem
      minItem[k] = ONEFLAG;
      // i: signature
      for (int i = 0; i < (int) listOfSigs.size(); i++) {
	if (!endOfList[k][i] && minItem[k] > listOfSigs[i][k][idx[k][i]]) {
	  minItem[k] = listOfSigs[i][k][idx[k][i]];
	}
      }

      // check if a smaller one was found
      if (minItem[k] == ONEFLAG) {
        ++mindone;
        if (mindone == lists)
          return;
        else
          continue;
      }

      // update the indexes
      for (int i = 0; i < (int) listOfSigs.size(); i++) {
        if (!endOfList[k][i] && listOfSigs[i][k][idx[k][i]] == minItem[k]) {
          idx[k][i]++;
          if (idx[k][i] == listOfSigs[i][k].size()) {
            endOfList[k][i] = true;
          }
        }
      }
      // insert to union
      resPoly[k].push_back(minItem[k]);  
    }
  }

  return;
}

// Compute the LCM of two polynomials
void lcm(std::vector<POLY>& t, const std::vector<POLY>& s)
{
  if (t.size() == 1 && s.size() == 1 && t[0] == 0x1 && s[0] == 0x1) {
    return;
  }
	
  std::vector<POLY> g;
  gcdSpecial(g, t, s);

  std::vector<POLY> q;
  int degs = getDegree(s);
  int degg = getDegree(g);

  if (degs >= degg) {
    quotient(q, s, g);
  }
  else {
    quotient(q, g, s);
  }

#ifdef _DEBUG_
  int qDeg = getDegree(q);
  cout << "Quotient degree: " << qDeg << endl;

  cout << "Before LCM degree: " << getDegree(t) << endl;
  cout << "Before LCM: ";
  for (int i = 0; i < t.size(); i++) {
    cout << t[i] << " ";
  }
  cout << endl;
#endif
	
  std::vector<POLY> res;
  multiplyPoly(res, t, q);

  t = res;
	
#ifdef _DEBUG_
  cout << "LCM degree: " << getDegree(t) << endl;
  cout << "LCM: ";
  for (int i = 0; i < t.size(); i++) {
    cout << t[i] << " ";
  }
  cout << endl;

#endif
  return;
}

int randomNumGen(int r)
{
  static unsigned int seed = (unsigned int) getgtod();
  int j;
  j = 1 + (int) (1.0 * r * rand_r(&seed)/(RAND_MAX+1.0));
  return j;
}

void makeKeyValue(char **ptr, int& len, std::vector<POLY>& textHash,
		  std::vector<POLY>& sig, int docId, InsertType type)
{
  int textHashSize = textHash.size();
  int sigSize = sig.size();
    
  len = sizeof(int) + sizeof(textHashSize) + textHashSize * sizeof(POLY)
    +  sizeof(sigSize) + sigSize * sizeof(POLY) + sizeof(docId);
	
  *ptr = new char[len];
  assert(ptr);
  char *buf = *ptr;

  if (type != NONE) {
    // Copy type
    memcpy(buf, &type, sizeof(type));
    buf += sizeof(type);
  }

  // Copy textHash
  memcpy(buf, &textHashSize, sizeof(textHashSize));
  buf += sizeof(textHashSize);

  POLY *sigPtr = (POLY *) buf;
  for (int i = 0; i < (int) textHash.size(); i++) {
    sigPtr[i] = textHash[i];
  }

  buf += textHashSize * sizeof(POLY);

  // Copy signature
  memcpy(buf, &sigSize, sizeof(sigSize));
  buf += sizeof(sigSize);

  sigPtr = (POLY *) buf;
  for (int i = 0; i < (int) sig.size(); i++) {
    sigPtr[i] = sig[i];
  }

  buf += sigSize * sizeof(POLY);

  // Finally docid
  memcpy(buf, &docId, sizeof(docId));
        
  return;
}

// Free allocated memory
void cleanup(char *ptr)
{
  delete[] ptr;
}

// Concatenate sig and docid
void makeDocid(char **input, int *inputlen, std::vector<POLY>& sig, char *docid)
{
  int len = sizeof(int) + sizeof(POLY) * sig.size() + sizeof(int)
    + strlen(docid);
  *inputlen = len;
  *input = new char[len];
  assert(*input);
  char *ptr = *input;

  int size = sig.size();
  memcpy(ptr, &size, sizeof(size));
  ptr += sizeof(size);
    
  POLY *buf = (POLY *) ptr;

  for (int i = 0; i < (int) sig.size(); i++) {
    buf[i] = sig[i];
    ptr += sizeof(POLY);
  }

  size = strlen(docid);
  memcpy(ptr, &size, sizeof(size));
  ptr += sizeof(size);
  memcpy(ptr, docid, size);

  return;
}


// MBR
void updateMBR(std::vector<POLY>& src, std::vector<POLY>& e)
{
  if (e[0] < src[0])
    src[0] = e[0];

  if (e[1] > src[1])
    src[1] = e[1];
}

// Compute the LCM of two polynomials
void lcmSpecial(std::vector<POLY>& t, const std::vector<POLY>& s, std::vector<POLY>& g)
{
  if (t.size() == 1 && s.size() == 1 && t[0] == 0x1 && s[0] == 0x1) {
    return;
  }

  //std::vector<POLY> g;
  //gcdSpecial(g, t, s);

  std::vector<POLY> q;
  int degs = getDegree(s);
  int degg = getDegree(g);

  if (degs >= degg) {
    quotient(q, s, g);
  }
  else {
    quotient(q, g, s);
  }

#ifdef _DEBUG_
  int qDeg = getDegree(q);
  cout << "Quotient degree: " << qDeg << endl;

  cout << "Before LCM degree: " << getDegree(t) << endl;
  cout << "Before LCM: ";
  for (int i = 0; i < t.size(); i++) {
    cout << t[i] << " ";
  }
  cout << endl;
#endif

  std::vector<POLY> res;
  multiplyPoly(res, t, q);

  t = res;

#ifdef _DEBUG_
  cout << "LCM degree: " << getDegree(t) << endl;
  cout << "LCM: ";
  for (int i = 0; i < t.size(); i++) {
    cout << t[i] << " ";
  }
  cout << endl;

#endif
  return;
}
 
// sig v1: docid
void getKeyValue(const char *buf, int& docid, std::vector<POLY>& sig)
{
  int sigLen;
  memcpy(&sigLen, buf, sizeof(sigLen));
 
  const char *ptr = buf;
  ptr += sizeof(sigLen);
     
  POLY *p = (POLY *) ptr;
 
  for (int i = 0; i < sigLen; i++) {
    sig.push_back(p[i]);
    ptr += sizeof(POLY);
  }

  memcpy(&docid, ptr, sizeof(docid));
}

// sig v2: docid
void getKeyValue(const char *buf, int& docid, std::vector<std::vector<POLY> >& sig)
{
  int lists = sig.size();
  int sigLen;
  const char *ptr = buf;

  assert(lists == SPOLISTS);

  memcpy(&docid, ptr, sizeof(docid));
  ptr += sizeof(docid);

  memcpy(&sigLen, ptr, sizeof(sigLen));
  ptr += sizeof(sigLen);
     
  for (int j = 0; j < lists; j++) {
    POLY *p = (POLY *) ptr;
    for (int i = 0; i < sigLen; i++) {
      sig[j].push_back(p[i]);
      ptr += sizeof(POLY);
    }
  }
}
 
// sig v1: docid
void makeKeyValue(char **inptr, int& len, std::vector<POLY>& sig, int docid)
{
  len = sizeof(int) + sig.size() * sizeof(POLY) + sizeof(docid);
 
  char *ptr = new char[len];
  assert(ptr);
     
  *inptr = ptr;
 
  int sigLen = sig.size();
     
  memcpy(ptr, &sigLen, sizeof(sigLen));
 
  ptr += sizeof(sigLen);
 
  POLY *buf = (POLY *) ptr;
     
  for (int i = 0; i < (int) sig.size(); i++) {
    buf[i] = sig[i];
    ptr += sizeof(POLY);
  }
     
  memcpy(ptr, &docid, sizeof(docid));
     
}

// sig v2: docid
void makeKeyValue(char **inptr, int& len, std::vector<std::vector<POLY> >& siglists, int docid)
{
  int lists = siglists.size();
  len = sizeof(int) + siglists[0].size() * lists * sizeof(POLY) + sizeof(docid);

  assert(lists == SPOLISTS);
 
  char *ptr = new char[len];
  assert(ptr);
     
  *inptr = ptr;
 
  memcpy(ptr, &docid, sizeof(docid));
  ptr += sizeof(docid);

  int sigLen = siglists[0].size();
  memcpy(ptr, &sigLen, sizeof(sigLen));
  ptr += sizeof(sigLen);

  for (int j = 0; j < lists; j++) {
    POLY *buf = (POLY *) ptr;
       
    for (int i = 0; i < (int) siglists[j].size(); i++) {
      buf[i] = siglists[j][i];
      ptr += sizeof(POLY);
    }
  }
}

// sig v1: nodeid
void getKeyValue(const char *buf, char *docid, std::vector<POLY>& sig)
{
  int sigLen;
  memcpy(&sigLen, buf, sizeof(sigLen));
 
  const char *ptr = buf;
  ptr += sizeof(sigLen);
     
  POLY *p = (POLY *) ptr;
 
  for (int i = 0; i < sigLen; i++) {
    sig.push_back(p[i]);
    ptr += sizeof(POLY);
  }

  strcpy(docid, ptr);
}

// sig v2: nodeid
void getKeyValue(const char *buf, char *docid, std::vector<std::vector<POLY> >& sig)
{
  int lists = sig.size();
  int sigLen;
  const char *ptr = buf;

  assert(lists == SPOLISTS);

  strcpy(docid, ptr);
  ptr += strlen(docid);
     
  memcpy(&sigLen, ptr, sizeof(sigLen));
  ptr += sizeof(sigLen);

  for (int j = 0; j < lists; j++) {
    POLY *p = (POLY *) ptr;
    for (int i = 0; i < sigLen; i++) {
      sig[j].push_back(p[i]);
      ptr += sizeof(POLY);
    }
  }
}

// sig v1: nodeid
void makeKeyValue(char **inptr, int& len, std::vector<POLY>& sig, char *docid)
{
  len = sizeof(int) + sig.size() * sizeof(POLY) + strlen(docid) + 1;
 
  char *ptr = new char[len];
  assert(ptr);
     
  *inptr = ptr;
 
  int sigLen = sig.size();
     
  memcpy(ptr, &sigLen, sizeof(sigLen));
 
  ptr += sizeof(sigLen);
 
  POLY *buf = (POLY *) ptr;
     
  for (int i = 0; i < (int) sig.size(); i++) {
    buf[i] = sig[i];
    ptr += sizeof(POLY);
  }
     
  strcpy(ptr, docid);
}

// sig v2: nodeid
void makeKeyValue(char **inptr, int& len, std::vector<std::vector<POLY> >& siglists, char *docid)
{
  int lists = siglists.size();
  len = sizeof(int) + siglists[0].size() * lists * sizeof(POLY) + strlen(docid) + 1;

  assert(lists == SPOLISTS);
 
  char *ptr = new char[len];
  assert(ptr);
     
  *inptr = ptr;
 
  strcpy(ptr, docid);
  ptr += strlen(docid);

  int sigLen = siglists[0].size();
  memcpy(ptr, &sigLen, sizeof(sigLen));
  ptr += sizeof(sigLen);

  for (int j = 0; j < lists; j++) {
    POLY *buf = (POLY *) ptr;
       
    for (int i = 0; i < (int) siglists[j].size(); i++) {
      buf[i] = siglists[j][i];
      ptr += sizeof(POLY);
    }
  } 
}

// BEGIN FOR VASIL
bool compressSignatures(std::vector<std::vector<POLY> >& inputSig, 
			std::vector<POLY>& outputSig, std::vector<std::vector<unsigned char> >& outputBitmap)
{
  
  // find # of sigs to compress
  int numSigs = inputSig.size();
  if (numSigs == 0) {
    std::cout << "Nothing to compress..." << std::endl;
    return false;
  }

  POLY minPoly;
  std::vector<int> index;
  
  // init the indexes
  for (int i = 0; i < numSigs; i++) {
    index.push_back(0x0);
    if (inputSig[i].size() == 0) {
      std::cout << "At least one signature is empty." << std::endl;
      return false;
    }
  }

  // Create an empty bitmap
  std::vector<unsigned char> ZEROVECTOR;
  for (int j = 0; j < ceil(numSigs/8.0); j++) {
    ZEROVECTOR.push_back((unsigned char) 0x0);
  }

  // Create the bitmaps
  while (1) {
    // Check the loop break out condition
    bool eov = true;
    for (int i = 0; i < numSigs; i++) {
      if (index[i] < (int) inputSig[i].size()) {
	eov = false;
	break;
      }
    }

    // We are done if we have scanned all the sigs
    if (eov == true) break;

    minPoly = (POLY) 0xffffffff;
    // First find the min from all the sigs
    for (int i = 0; i < numSigs; i++) {
      if (index[i] < (int) inputSig[i].size()) {
	if (inputSig[i][index[i]] < minPoly) {
	  minPoly = inputSig[i][index[i]];
	}
      }
      //else {
      //std::cout << "Reached EOF for " << i << std::endl;
      //}
    }
  
    std::cout << "MIN poly: " << minPoly << std::endl;

    // Push the minPoly to the output signature
    outputSig.push_back(minPoly);
    // Push an zero vector
    outputBitmap.push_back(ZEROVECTOR);

    // Now loop through all the signatures and increment the indexes and
    // update the bitmaps

    for (int i = 0; i < numSigs; i++) {
      // If it matches minPoly
      printf("Input sig: %u %d\n", inputSig[i][index[i]], index[i]);
      if (index[i] < (int) inputSig[i].size() && inputSig[i][index[i]] == minPoly) {
	// Adjust the bitmap
	int bitmapid = i / 8;
	
	unsigned char mask = (unsigned char) 0x80 >> (i % 8);

	printf("Bitmapid: %d %d mask: %x\n", i, bitmapid, mask);
	// do a bitwise OR using the mask
	outputBitmap[outputBitmap.size() - 1][bitmapid] = 
	  outputBitmap[outputBitmap.size() - 1][bitmapid] | mask;
	
	// move to the next element
	index[i]++;
      }
    }
  }

  return true;
}


bool uncompressSignatures(std::vector<std::vector<POLY> >& outputSig, 
			  std::vector<POLY>& inputSig, std::vector<std::vector<unsigned char> >& inputBitmap, int numSigs)
{
  
  // Initialize 
  for (int i = 0; i < numSigs; i++) {
    std::vector<POLY> e;
    outputSig.push_back(e);
  }

  // Now extract the items
  for (int i = 0; i < (int) inputSig.size(); i++) {
    for (int j = 0; j < numSigs; j++) {
      unsigned char mask = (unsigned char) 0x80 >> (j % 8);
      int bitmapid = j / 8;
      if (inputBitmap[i][bitmapid] & mask) {
	outputSig[j].push_back(inputSig[i]);
      }
    }
  }
  return true;
}


void makeKeyValue(char **inptr, std::vector<POLY>& inputSig, 
		  std::vector<std::vector<unsigned char> >& inputBitmap, int numSigs)
{
  int len = sizeof(numSigs) + sizeof(int) + (int) inputSig.size() * (sizeof(POLY) + (int) ceil(numSigs/8.0));
  unsigned char *ptr = new unsigned char[len];
  *inptr = (char *) ptr;

  // store the num of signatures
  memcpy(ptr, &numSigs, sizeof(numSigs));
  ptr += sizeof(numSigs);

  // store the len of the aligned multiset
  int alen = (int) inputSig.size();
  memcpy(ptr, &alen, sizeof(alen));
  ptr += sizeof(alen);

  // store the aligned multiset
  for (int i = 0; i < alen; i++) {
    memcpy(ptr, &inputSig[i], sizeof(POLY));
    ptr += sizeof(POLY);

    // bitmap
    for (int j = 0; j < (int) inputBitmap[i].size(); j++) {
      memcpy(ptr, &inputBitmap[i][j], sizeof(unsigned char));
      ptr += sizeof(unsigned char);
    }
  }

  return;
}

void getKeyValue(const char *buf, std::vector<POLY>& outputSig, 
		 std::vector<std::vector<unsigned char> >& outputBitmap, int& numSigs)
{
  const char *ptr = buf;
  memcpy(&numSigs, ptr, sizeof(numSigs));
  ptr += sizeof(numSigs);

  int alen;
  memcpy(&alen, ptr, sizeof(alen));
  ptr += sizeof(alen);

  printf("Num sigs: %d Alen %d\n", numSigs, alen);

  for (int i = 0; i < alen; i++) {
    outputSig.push_back(*((POLY *) ptr));
    ptr += sizeof(POLY);
    
    std::vector<unsigned char> e;
    e.clear();

    for (int j = 0; j < (int) ceil(numSigs/8.0); j++) {
      e.push_back(*((unsigned char *) ptr));
      ptr += sizeof(unsigned char);
    }
    
    outputBitmap.push_back(e);
  }
  return;
}
