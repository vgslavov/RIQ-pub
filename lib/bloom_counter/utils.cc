// Author: Praveen Rao
// UMKC/UArizona, 2008
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include "utils.h"
#include <sys/time.h>
#include <assert.h>
using namespace std;

// Returns the degree of the polynomial
int getDEGREE(POLY z)
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
  return findMod(input, strlen(input), irrPoly, getDEGREE(irrPoly));
}
