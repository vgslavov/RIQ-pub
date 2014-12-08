#include<stdio.h>
#include<string>
#include<fstream>
#include<sstream>
#include<cstring>
#include<stdlib.h>
#include<iostream>
#include"bloom_counter.h"
#include "utils.h"
#include <vector>
#include<algorithm>
#include<assert.h>
#define SPO_BUFF 1024*4
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


string check_var(string& var);
int main(int argc, char *argv[])
{
  BLOOM *bloom;
  
  if(!(argc==4||argc==6)) {
    fprintf(stderr, "Usage: <data_file_loc> <r|w> [query_file] [<filter_size>] [<functions_#>] [2/4/8] \n");
    return EXIT_FAILURE;
  }

  if(!strcmp(argv[2],"r")) {
    FILE *fp;
    char element[1024];
    char *t;
    bloom=bloom_read(argv[1]);

//    cout<<"done..."<<endl;

    if(!(fp=fopen(argv[3], "r"))) {
      fprintf(stderr, "ERROR: Could not open file %s\n", argv[1]);
      return EXIT_FAILURE;
    }

/*    
    POLY irrPoly = 1675850375;  // converts string to int
    vector<int> query;

    while(fgets(element,1024,fp)) {
      if((t=strchr(element, '\n'))) 
        *t='\0';
	int con = hashIrrPoly(element, irrPoly);
	cout<<"con : "<<con<<endl;  
	query.push_back(con);// converts string to int
*/

  ifstream ifs_query(argv[3]);
  if(!ifs_query.is_open()){
    cout<<"Error! Unable to open the input file."<<endl;
    exit(2);
  }

  //cout<<"file :"<<argv[1]<<endl;
  string query="why";
  bool is_query=false;
  int query_num=0;
  vector<POLY>full_query;

  while(getline(ifs_query,query)){

    stringstream start;
    string st(query);
    start<<st;
    start>>st;
    // cout<<"st is :"<<st<<endl;
    // cout<<"substr is :"<<st.substr(0,1)<<endl;
    if(st.substr(0,1)=="#"||query=="")//commented or blank lines
      continue;
    if(query.substr(0,1)=="{"){   //Start of query
      is_query=true;
      query_num++;
      continue;
    }
    if(query.substr(0,1)=="}")   //End of query
      is_query=false;
    // is_query=true;
    if(is_query){
      

      char su[SPO_BUFF],pr[SPO_BUFF],ob[SPO_BUFF];
      sscanf(&query[0],"%s %s %[^\n]s",su,pr,ob);
      string o(ob);
      string p(pr);
      string s(su);
      o=o.substr(0,o.length()-2);
      
      s=check_var(s);
      p=check_var(p);
      o=check_var(o);

      string store="( "+s+" , "+p+" , "+o+" )";
      //cout<<store<<endl;
      
      //cout<<"query:"<<query<<endl;
      //cout<< "The s is :"<<s<<":";
      //cout<<"   The p is :"<<p<<":";
      //cout<<"   The o is :"<<o<<":"<<endl<<endl;
      

      POLY irrPoly = 1675850375;
      int pol;
      pol=hashIrrPoly(&store[0],irrPoly);
    //  cout<<"Poly :"<<pol<<endl;
      full_query.push_back(pol);     
    } 
  }

  if(!bloom_multi(bloom, full_query))
  	;//    printf("No match for the query\n\n");
    else
      //printf("Query has a hit !!!\n\n");
        printf("%s\n",argv[1]);

 //   cout<<"Calling Bloom Details :"<<endl;
 //   bloom_details(bloom);
    
    bloom_destroy(bloom);
    exit(2);

  }



/*
    int graph_id=101;
    fclose(fp);
    size_t size_query=query.size();
    cout<<"Size is :"<<size_query<<endl;
    sort(query.begin(), query.end());
    FILE *fp_ind = fopen("./index_input","w");
    assert(fp_ind);
    fwrite(&graph_id,sizeof(int),1,fp_ind);
    fwrite(&size_query,sizeof(size_t),1,fp_ind);
    
    vector<int>::iterator it_index;    
    for(it_index=query.begin();it_index!=query.end();++it_index)   {    int a;
      a=*it_index;
      fwrite(&a,sizeof(int),1,fp_ind);
    }
    fclose(fp_ind);  
    fp_ind=fopen("./index_input","r"); 
    assert(fp_ind);

    int *ab = (int*) malloc(2*sizeof(int));
    int abb[2];
    fread(ab,sizeof(int),2, fp_ind);
    cout<<"Extract :"<<ab<<endl;
*/

/*

  if(!bloom_multi(bloom, query)) 
      printf("No match for the query\n\n");
    else
      printf("Query has a hit !!!\n\n");	
    exit(2);
  
  }
*/



  else if(!strcmp(argv[2],"w")) {	
    if(!(bloom=bloom_create(atoi(argv[3]),atoi(argv[4]), atoi(argv[5])))) {
      fprintf(stderr, "ERROR: Could not create bloom filter\n");
      return EXIT_FAILURE;
    }

    FILE *fp;
    char line[1024];
    char *p ;

    if(!(fp=fopen(argv[1], "r"))) {
      fprintf(stderr, "ERROR: Could not open file %s\n", argv[1]);
      return EXIT_FAILURE;
    }

    POLY irrPoly = 1675850375;  // converts string to int
    vector<int> data;
    while(fgets(line, 1024, fp)) {
      if((p=strchr(line, '\n'))) 
        *p='\0';
  	data.push_back(hashIrrPoly(line, irrPoly));// converts string to int
    }
    fclose(fp);

    if(!bloom_add(bloom, data))
    {
      cout<<"Couldnot add a line into bloom, terminating..."<<endl;
      exit(2);
    }
    bloom_meta_write(bloom,"./first_write");	
    /*	while(fgets(line, 1024, stdin)) {
		if((p=strchr(line, '\n'))) *p='\0';
	//	cout<<"Extracted line :"<<line<<":"<<endl;
		if(!bloom_check(bloom, line)){
   			cout<<"Not Present"<<endl;
		}
  		else
 			cout<<"Yes Present"<<endl;
	}*/
  	//bloom_write(bloom,"./first_write");
    bloom_destroy(bloom);
  }

  else
    cout<<"Wrong set of parameters. Please specify r/w only!"<<endl;
  return EXIT_SUCCESS;
}

string check_var(string& var){
  if(var.substr(0,1)=="?")
    return "?";
  else
    return var;
}
