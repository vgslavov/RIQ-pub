#include<stdio.h>
#include<string.h>
#include<iostream>
#include"bloom_filter.h"

using namespace std;

unsigned int sax_hash(const char *key) {
	unsigned int h=0;
	while(*key) h^=(h<<5)+(h>>2)+(unsigned char)*key++;
	return h;
}

unsigned int sdbm_hash(const char *key) {
	unsigned int h=0;
	while(*key) h=(unsigned char)*key++ + (h<<6) + (h<<16) - h;
	return h;
}

int main(int argc, char *argv[])
{
	FILE *fp,*tp;
	char line[1024];
	char *p, *t;
	BF *bf;
	int pos=0;
	int neg=0;
	
	if(!(argc==3||argc==5||argc==4)) {
		fprintf(stderr, "Usage: <file_loc> <r|w> [<filter_size>|<TestCases_File>] [<functions_#>]\n");
		return EXIT_FAILURE;
	}

	if(!strcmp(argv[2],"r")) {
		bf=bf_read(argv[1]);
		if(argc==4) {
			if(!(tp=fopen(argv[3], "r"))) {
				fprintf(stderr, "ERROR: Could not open test-case file %s\n", argv[3]);
				return EXIT_FAILURE;
			}
			
			while(fgets(line, 1024, tp)) {
				if((p=strchr(line, '\n'))) *p='\0';
				if(!bf_check(bf, line)) {
					neg++;
					printf("No match for the word \"%s\"\n\n", line);
				}
				else {
					pos++;
					printf("Got the word !\n\n");
				}
			}
			printf("Total Number of test cases: %d\n", pos+neg);
			printf("Total Number of false-positives: %d\n", pos);
			printf("Total Number of non false-positives: %d\n", neg);			
		}
		else {
			cout<<"Please enter a string to match -"<<endl;
			while(fgets(line,1024,stdin)) {
			
				if((p=strchr(line, '\n'))) *p='\0';
				if(!bf_check(bf, line)) 
					printf("No match for the word \"%s\"\n\n", line);
				else
					printf("Got the word !\n\n");	
				cout<<"Please enter a string to match -"<<endl;
			}
			
		}
		exit(2);
	}
	
	if(!(bf=bf_create(atoi(argv[3]),atoi(argv[4])))) {
		fprintf(stderr, "ERROR: Could not create bloom filter\n");
		return EXIT_FAILURE;
	}

	if(!(fp=fopen(argv[1], "r"))) {
		fprintf(stderr, "ERROR: Could not open file %s\n", argv[1]);
		return EXIT_FAILURE;
	}

	while(fgets(line, 1024, fp)) {
		if((p=strchr(line, '\n'))) *p='\0';
	//	cout<<"Extracted line :"<<line<<":"<<endl;
		bf_add(bf, line);
	}
	fclose(fp);
	bf_write(bf,"./first_write");
	bf_destroy(bf);
	return EXIT_SUCCESS;
}
