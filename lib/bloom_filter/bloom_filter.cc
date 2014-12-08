/*
 * All Bloom filter code based on:
 * http://en.literateprograms.org/Bloom_filter_(C)
 * (available under the Creative Commons (CCO) license)
 * and modified and enhanced by Srivenu Paturi (sp895@umkc.edu)
 */

#include<limits.h>
#include<stdarg.h>
#include"bloom_filter.h"
#include<iostream>
#include<fstream>
#include<assert.h>
#include<string.h>
#define SETBIT(a, n) (a[n/CHAR_BIT] |= (1<<(n%CHAR_BIT)))
#define GETBIT(a, n) (a[n/CHAR_BIT] & (1<<(n%CHAR_BIT)))

#include "utils.h" // contains all the apis required to manipulate polys
using namespace std;

//BF *bf_create(size_t size, size_t nfuncs, ...)
BF *bf_create(size_t size, size_t nfuncs) {
	BF *bf;
	
	if(!(bf=(BF*)malloc(sizeof(BF)))) return NULL;
	if(!(bf->a=(unsigned char*)calloc((size+CHAR_BIT-1)/CHAR_BIT, sizeof(char)))) {		free(bf);
		return NULL;
	}
	if(!(bf->para_a=(int*)malloc(nfuncs*sizeof(int)))) {
		free(bf->a);
		free(bf);
		return NULL;	
	}
	if(!(bf->para_b=(int*)malloc(nfuncs*sizeof(int)))){
		free(bf->a);
		free(bf);
		return NULL;	
	}
	srand(time(NULL));
	for (int i = 0; i < nfuncs; i++) {
		// TODO: verify randomness
		bf->para_a[i]  = int((double)size*rand()/(RAND_MAX + 1.0));
		bf->para_b[i]  = int((double)size*rand()/(RAND_MAX + 1.0));
	}
	bf->nfuncs=nfuncs;
	bf->asize=size;
	return bf;
}

int bf_destroy(BF *bf) {
	free(bf->a);
	free(bf->para_a);
	free(bf->para_b);
	free(bf);
	return 0;
}

int bf_add(BF *bf, const char *s) {
	size_t n;	
	cout<<"Inserted :"<<s<<":"<<endl;
	POLY irrPoly = 1675850375;
	// converts string to int
	int hashVal = hashIrrPoly(s, irrPoly);	
	for(n=0; n<bf->nfuncs; ++n) {		
		SETBIT(bf->a, (((bf->para_a[n])*hashVal) + (bf->para_b[n]))%bf->asize);
	}
	return 0;
}

int bf_add(BF *bf, vector<POLY> &data) {
	size_t n;	
	vector<POLY>::iterator item;

	//cout<<"Inserted :"<<s<<":"<<endl;
	//POLY irrPoly = 1675850375;
	// converts string to int
	//int hashVal = hashIrrPoly(s, irrPoly);	

	for (item = data.begin(); item != data.end(); item++) {
		POLY hashVal = *item;
		for(n=0; n<bf->nfuncs; ++n) {		
			SETBIT(bf->a, (((bf->para_a[n])*hashVal) + (bf->para_b[n]))%bf->asize);
		}
	}
	return 0;
}

int bf_check(BF *bf, const char *s) {
	size_t n;
	cout<<"Checking :"<<s<<endl;

	POLY irrPoly = 1675850375;
	// converts string to int
	int hashVal = hashIrrPoly(s, irrPoly);
	for(n=0; n<bf->nfuncs; ++n) {
		if(!(GETBIT(bf->a, (((bf->para_a[n])*hashVal) + (bf->para_b[n]))%bf->asize))) return 0;
	}
	return 1;
}

int bf_check(BF *bf, vector<POLY> &query) {
	size_t n;
	vector<POLY>::iterator itr;

	//POLY irrPoly = 1675850375;
	// converts string to int
	//int hashVal = hashIrrPoly(s, irrPoly);
	for (itr = query.begin(); itr != query.end(); itr++) {
	  POLY hashVal = *itr;
	  for(n=0; n<bf->nfuncs; ++n) {
		  if(!(GETBIT(bf->a, (((bf->para_a[n])*hashVal) + (bf->para_b[n]))%bf->asize))) return 0;
	  }
	}
	return 1;
}

void bf_write(BF *bf, const char *path) {
	
	FILE *fp = fopen(path, "w");
	assert(fp);
	fprintf(fp, "%lu\n",bf->asize);
	fprintf(fp, "%lu\n",bf->nfuncs);
	for(int k=0;k<bf->nfuncs;k++) {
		fprintf(fp,"%d ",bf->para_a[k]);
	}
	fprintf(fp,"\n");
	for(int k=0;k<bf->nfuncs;k++) {
		fprintf(fp,"%d ",bf->para_b[k]);
	}
	fprintf(fp,"\n");

	fwrite(bf->a, sizeof(unsigned char), bf->asize, fp);  
	fclose(fp);
}

BF* bf_read(const char *path) {

	BF *bf;
	string buff;
	ifstream ifs(path,ios::binary);
	if(!ifs.is_open()) {
		cout<<"Error:Unable to open file "<<path<<". Terminating the program!"<<endl;
		exit(2);
	}
	getline(ifs,buff);
	unsigned long int size = atoi(&buff[0]);
	getline(ifs,buff);
	int nfuncs = atoi(&buff[0]); 

	if(!(bf=(BF*)malloc(sizeof(BF)))) return NULL;
	if(!(bf->a=(unsigned char*)calloc((size+CHAR_BIT-1)/CHAR_BIT, sizeof(char)))) {		
		free(bf);
		return NULL;
	}
	if(!(bf->para_a=(int*)malloc(nfuncs*sizeof(int)))) {
		free(bf->a);
		free(bf);
		return NULL;	
	}
	if(!(bf->para_b=(int*)malloc(nfuncs*sizeof(int)))){
		free(bf->a);
		free(bf);
		return NULL;	
	}
	getline(ifs,buff);
	char *tok1=strtok(&buff[0]," ");
	int i=0;
	while(tok1!=NULL){
		bf->para_a[i]=atoi(tok1);
		tok1=strtok(NULL," ");
		i++;  	
	}
	getline(ifs,buff);	
	char *tok2=strtok(&buff[0]," ");
	i=0;
	while(tok2!=NULL){
		bf->para_b[i]=atoi(tok2);
		tok2=strtok(NULL," ");
		i++;  	
	}
	int temp=((size+CHAR_BIT-1)/CHAR_BIT)*sizeof(char);
	if(!ifs.read((char*)bf->a,temp)){
		cout<<"Error:Unable to read filter from file "<<path<<". Terminating the program!"<<endl;
		exit(2);
	}
	bf->asize=size;
	bf->nfuncs=nfuncs;	
	ifs.close();
	return bf;
}
void bf_details(BF *bf) {
	//cout<<" Bloom size :"<<strlen(bf->a);
	cout<<" Bloom is :"<< ":"<<endl;
}

