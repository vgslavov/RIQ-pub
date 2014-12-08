/*
 * All Bloom filter code based on:
 * http://en.literateprograms.org/Bloom_filter_(C)
 * (available under the Creative Commons (CCO) license)
 * and modified and enhanced by Srivenu Paturi (sp895@umkc.edu)
 */

#ifndef __BLOOM_H__
#define __BLOOM_H__

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
	int range; /*range of counter in bits*/
	/*hashfunc_t *funcs;*/
} BLOOM;

BLOOM *bloom_create(size_t size, size_t nfuncs, size_t range);
int bloom_destroy(BLOOM *bloom);
int bloom_add(BLOOM *bloom, vector<int> &data);
int bloom_add(BLOOM *bloom, vector<POLY> &data);
int bloom_check(BLOOM *bloom, const char *s);
void bloom_write(BLOOM *bloom, const char *path);
void bloom_meta_write(BLOOM *bloom, const char *path);
int bloom_multi(BLOOM *bloom, vector<int> &query); 
int bloom_multi(BLOOM *bloom, vector<POLY> &query); 
void bloom_details(BLOOM *bloom);
BLOOM *bloom_read(const char *path);
BLOOM *bloom_read_mmap(const char *path);
BLOOM *bloom_meta_read(const char *path);
BLOOM* bloom_reset (BLOOM *bloom);
#endif
