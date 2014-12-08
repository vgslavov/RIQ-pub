#include<stdio.h>
#include<string.h>
#include<iostream>
#include"bloom_counter.h"

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
	FILE *fp;
	char line[1024];
	char *p, *t;
	BLOOM *bloom;
	
	if(!(argc==3||argc==6)) {
		fprintf(stderr, "Usage: <file_loc> <r|w> [<filter_size>] [<functions_#>] [2/4/8] \n");
		return EXIT_FAILURE;
	}

	if(!strcmp(argv[2],"r")) {
		bloom=bloom_read(argv[1]);
		cout<<"Please enter a string to match -"<<endl;
		while(fgets(line,1024,stdin)) {
			
			if((p=strchr(line, '\n'))) *p='\0';
			if(!bloom_check(bloom, line)) 
				printf("No match for the word \"%s\"\n\n", line);
			else
				printf("Got the word !\n\n");	
			cout<<"Please enter a string to match -"<<endl;
		}
		exit(2);
	}
	
	if(!(bloom=bloom_create(atoi(argv[3]),atoi(argv[4]), atoi(argv[5])))) {
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
		if(!bloom_add(bloom, line)){
   			cout<<"Couldnot add a line into bloom, terminating..."<<endl;
			exit(2);
		}
	}
        bloom_write(bloom,"./first_write");
	
/*	while(fgets(line, 1024, stdin)) {
		if((p=strchr(line, '\n'))) *p='\0';
	//	cout<<"Extracted line :"<<line<<":"<<endl;
		if(!bloom_check(bloom, line)){
   			cout<<"Not Present"<<endl;
		}
  		else
 			cout<<"Yes Present"<<endl;
	}*/
	//while(1);
	fclose(fp);
	//bloom_write(bloom,"./first_write");
	bloom_destroy(bloom);
	return EXIT_SUCCESS;
}
