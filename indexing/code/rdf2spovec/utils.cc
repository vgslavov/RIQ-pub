// Author: Praveen Rao
// UMKC/UArizona, 2008
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include "utils.h"
#include <sys/time.h>
#include <assert.h>
using namespace std;
// Cleanup a string by deleting special characters
string cleanString(string &s)
{
	string res = s;

    
	bool flag = false;
	for (int i = s.length()-1; i >= 0; i--) {
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
    for (int i = 0; i < res.length(); ) {
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
void multiplyPoly(vector<POLY>& multiplicand, POLY multiplier)
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
void multiplyPoly(vector<POLY>& res, const vector<POLY>& multiplicand,
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

// Return the remainder of x / y
POLY remainder(vector<POLY> &input, POLY z)
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
int getDegree(const vector<POLY>& myPoly)
{
	if (myPoly.size() == 0) {
		assert(0);
	}
	
	int deg = getDegree(myPoly[0]);
	return ((myPoly.size() - 1) * sizeof(POLY) * 8) + deg;
}

// Shift left
void shiftLeft(vector<POLY>& myPoly, int numBits)
{
	if (numBits == 0) return;
	if (numBits == sizeof(POLY) * 8) {
		myPoly.push_back((POLY) 0);
		return;
	}
	
	assert(numBits > 0 && numBits <= sizeof(POLY) * 8);
	
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
		
		if (i < myPoly.size()-1) {
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
void shiftRight(vector<POLY>& myPoly, int numBits)
{
	if (numBits == 0 || numBits == sizeof(POLY) * 8) return;
	
	assert(numBits > 0 && numBits < sizeof(POLY) * 8);

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
	for (int i = 0; i < myPoly.size(); i++) {	
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

// Add a special check to have the larger degree poly on the left
// and smaller degree polynomial on the right.
void gcdSpecial(vector<POLY>& resPoly, const vector<POLY>& f, const vector<POLY>& g)
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
void gcd(vector<POLY>& resPoly, const vector<POLY>& f, const vector<POLY>& g)
{
	if (g.size() == 1 && g[0] == (POLY) 0) {
		resPoly = f;
	}
	else {
		vector<POLY> r;
		remainder(r, f, g);
		gcd(resPoly, g, r);
	}
}

// Multiply two polynomials
void multiplyPoly(vector<POLY>& res, const vector<POLY>& f,
									const vector<POLY>& g)
{
	vector<POLY> partialProd;

	//cout << "prod size: " << partialProd.size() << endl;
	for (int i = 0; i < g.size(); i++) {
		for (int j = 0; j < f.size(); j++) {
			partialProd.push_back((POLY) 0);
		}
																								 
		multiplyPoly(partialProd, f, g[i]);
		addPoly(res, partialProd);

		if (i < g.size() - 1) {
			shiftLeft(res, sizeof(POLY) * 8);
		}

		// clear the list
		partialProd.clear();
	}

}

// Add two polynomials
void addPoly(vector<POLY>& x, const vector<POLY>& y)
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

// Remainder of two polynomials
void remainder(vector<POLY>& r, const vector<POLY>& f, const vector<POLY>& g)
{
	assert(f.size() > 0 && g.size() > 0);
	
	int fdeg = getDegree(f[0]);
	int gdeg = getDegree(g[0]);

	vector<POLY> gtemp(g);
	
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

	for (int k = fFullDegree; k >= gFullDegree; k--) {
		if ((r[i] & bitMask) > (POLY) 0) {
			// Loop thro the polys and XOR each time...
			for (int j = 0; (i + j) < r.size() && (j < gtemp.size()); j++) {
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
void quotient(vector<POLY>& q, const vector<POLY>& f, const vector<POLY>& g)
{
	assert(f.size() > 0 && g.size() > 0);

	vector<POLY> r;
	int fdeg = getDegree(f[0]);
	int gdeg = getDegree(g[0]);

	vector<POLY> gtemp(g);
	
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
			for (int j = 0; (i + j) < r.size() && (j < gtemp.size()); j++) {
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

// Compute the LCM of two polynomials
void lcm(vector<POLY>& t, const vector<POLY>& s)
{
	if (t.size() == 1 && s.size() == 1 && t[0] == 0x1 && s[0] == 0x1) {
		return;
	}
	
	vector<POLY> g;
	gcdSpecial(g, t, s);

	vector<POLY> q;
	int degs = getDegree(s);
	int degg = getDegree(g);

	if (degs >= degg) {
		quotient(q, s, g);
	}
	else {
		quotient(q, g, s);
	}

	int qDeg = getDegree(q);

#ifdef _DEBUG_
	cout << "Quotient degree: " << qDeg << endl;

	cout << "Before LCM degree: " << getDegree(t) << endl;
	cout << "Before LCM: ";
	for (int i = 0; i < t.size(); i++) {
		cout << t[i] << " ";
	}
	cout << endl;
#endif
	
	vector<POLY> res;
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

// Random number generator
int randomNumGen(int r)
{
	static unsigned int seed = (unsigned int) getgtod();
	int j;
	j = 1 + (int) (1.0 * r * rand_r(&seed)/(RAND_MAX+1.0));
	return j;
}

int hashString(const char *input)
{
  int h = 0;
  for (int i = 0; i < strlen(input); i++) {
    h = 15*h + input[i];
  }
  return h;
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
