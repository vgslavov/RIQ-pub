#ifndef __BLOOM_H__
#define __BLOOM_H__

#include<stdlib.h>

typedef unsigned int (*hashfunc_t)(const char *);
typedef struct {
	size_t asize; /*size of bloom filter*/
	unsigned char *a; /*bloom filter array*/
	size_t nfuncs; /*number of hash functions*/
	int *para_a; /*first parameter*/
	int *para_b; /*second parameter*/
	/*hashfunc_t *funcs;*/
} BLOOM;

BLOOM *bloom_create(size_t size, size_t nfuncs);
int bloom_destroy(BLOOM *bloom);
int bloom_add(BLOOM *bloom, const char *s);
int bloom_check(BLOOM *bloom, const char *s);
void bloom_write(BLOOM *bloom, const char *path);
void bloom_details(BLOOM *bloom);
BLOOM *bloom_read(const char *path);
#endif
