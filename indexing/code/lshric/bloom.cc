#include<limits.h>
#include<stdarg.h>
#include"bloom.h"
#include<iostream>
#include<fstream>
#include<assert.h>
#include<string.h>
#define SETBIT(a, n) (a[n/CHAR_BIT] |= (1<<(n%CHAR_BIT)))
#define GETBIT(a, n) (a[n/CHAR_BIT] & (1<<(n%CHAR_BIT)))

#include "utils.h" // contains all the apis required to manipulate polys
using namespace std;

//BLOOM *bloom_create(size_t size, size_t nfuncs, ...)
BLOOM *bloom_create(size_t size, size_t nfuncs) {
	BLOOM *bloom;
	
	if(!(bloom=(BLOOM*)malloc(sizeof(BLOOM)))) return NULL;
	if(!(bloom->a=(unsigned char*)calloc((size+CHAR_BIT-1)/CHAR_BIT, sizeof(char)))) {		free(bloom);
		return NULL;
	}
	if(!(bloom->para_a=(int*)malloc(nfuncs*sizeof(int)))) {
		free(bloom->a);
		free(bloom);
		return NULL;	
	}
	if(!(bloom->para_b=(int*)malloc(nfuncs*sizeof(int)))){
		free(bloom->a);
		free(bloom);
		return NULL;	
	}
	srand(time(NULL));
	for (int i = 0; i < nfuncs; i++) {
		// TODO: verify randomness
		bloom->para_a[i]  = int((double)size*rand()/(RAND_MAX + 1.0));
		bloom->para_b[i]  = int((double)size*rand()/(RAND_MAX + 1.0));
	}
	bloom->nfuncs=nfuncs;
	bloom->asize=size;
	return bloom;
}

int bloom_destroy(BLOOM *bloom) {
	free(bloom->a);
	free(bloom->para_a);
	free(bloom->para_b);
	free(bloom);
	return 0;
}

int bloom_add(BLOOM *bloom, const char *s) {
	size_t n;	
	cout<<"Inserted :"<<s<<":"<<endl;
	POLY irrPoly = 1675850375;
	// converts string to int
	int hashVal = hashIrrPoly(s, irrPoly);	
	for(n=0; n<bloom->nfuncs; ++n) {		
		SETBIT(bloom->a, (((bloom->para_a[n])*hashVal) + (bloom->para_b[n]))%bloom->asize);
	}
	return 0;
}

int bloom_check(BLOOM *bloom, const char *s) {
	size_t n;
	cout<<"Checking :"<<s<<endl;

	POLY irrPoly = 1675850375;
	// converts string to int
	int hashVal = hashIrrPoly(s, irrPoly);
	for(n=0; n<bloom->nfuncs; ++n) {
		if(!(GETBIT(bloom->a, (((bloom->para_a[n])*hashVal) + (bloom->para_b[n]))%bloom->asize))) return 0;
	}
	return 1;
}

void bloom_write(BLOOM *bloom, const char *path) {
	
	FILE *fp = fopen(path, "w");
	assert(fp);
	fprintf(fp, "%lu\n",bloom->asize);
	fprintf(fp, "%lu\n",bloom->nfuncs);
	for(int k=0;k<bloom->nfuncs;k++) {
		fprintf(fp,"%d ",bloom->para_a[k]);
	}
	fprintf(fp,"\n");
	for(int k=0;k<bloom->nfuncs;k++) {
		fprintf(fp,"%d ",bloom->para_b[k]);
	}
	fprintf(fp,"\n");

	fwrite(bloom->a, sizeof(unsigned char), bloom->asize, fp);  
	fclose(fp);
}

BLOOM* bloom_read(const char *path) {

	BLOOM *bloom;
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

	if(!(bloom=(BLOOM*)malloc(sizeof(BLOOM)))) return NULL;
	if(!(bloom->a=(unsigned char*)calloc((size+CHAR_BIT-1)/CHAR_BIT, sizeof(char)))) {		
		free(bloom);
		return NULL;
	}
	if(!(bloom->para_a=(int*)malloc(nfuncs*sizeof(int)))) {
		free(bloom->a);
		free(bloom);
		return NULL;	
	}
	if(!(bloom->para_b=(int*)malloc(nfuncs*sizeof(int)))){
		free(bloom->a);
		free(bloom);
		return NULL;	
	}
	getline(ifs,buff);
	char *tok1=strtok(&buff[0]," ");
	int i=0;
	while(tok1!=NULL){
		bloom->para_a[i]=atoi(tok1);
		tok1=strtok(NULL," ");
		i++;  	
	}
	getline(ifs,buff);	
	char *tok2=strtok(&buff[0]," ");
	i=0;
	while(tok2!=NULL){
		bloom->para_b[i]=atoi(tok2);
		tok2=strtok(NULL," ");
		i++;  	
	}
	int temp=((size+CHAR_BIT-1)/CHAR_BIT)*sizeof(char);
	if(!ifs.read((char*)bloom->a,temp)){
		cout<<"Error:Unable to read filter from file "<<path<<". Terminating the program!"<<endl;
		exit(2);
	}
	bloom->asize=size;
	bloom->nfuncs=nfuncs;	
	ifs.close();
	return bloom;
}
void bloom_details(BLOOM *bloom) {
	//cout<<" Bloom size :"<<strlen(bloom->a);
	cout<<" Bloom is :"<< ":"<<endl;
}

