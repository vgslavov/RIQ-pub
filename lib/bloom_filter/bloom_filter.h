/*
 * All Bloom filter code based on:
 * http://en.literateprograms.org/Bloom_filter_(C)
 * (available under the Creative Commons (CCO) license)
 * and modified and enhanced by Srivenu Paturi (sp895@umkc.edu)
 */

#ifndef __BF_H__
#define __BF_H__

#include <stdlib.h>
#include <vector>
#include "poly.h"

using namespace std;

typedef unsigned int (*hashfunc_t)(const char *);
typedef struct {
	size_t asize; /*size of bloom filter*/
	unsigned char *a; /*bloom filter array*/
	size_t nfuncs; /*number of hash functions*/
	int *para_a; /*first parameter*/
	int *para_b; /*second parameter*/
	/*hashfunc_t *funcs;*/
} BF;

BF *bf_create(size_t size, size_t nfuncs);
int bf_destroy(BF *bloom);
int bf_add(BF *bloom, const char *s);
int bf_add(BF *bloom, vector<POLY> &data);
int bf_check(BF *bloom, const char *s);
int bf_check(BF *bloom, vector<POLY> &query);
void bf_write(BF *bloom, const char *path);
void bf_details(BF *bloom);
BF *bf_read(const char *path);
#endif
