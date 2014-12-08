/*
 *
 * AUTHOR 	: Srivenu Paturi
 * DESCRIPTION  : This program takes a n3 query file as the input and  
 *		  generates the corresponding binary file with poly
 *		  hashing on each triple. This generates an output
 *		  query file in the RIS format.
 */               

#include<cstdio>
#include<string>
#include<cstdlib>
#include<iostream>
#include<fstream>
#include<sstream>
#include "utils.h"
#include<vector>
#include<algorithm>
#include<assert.h>
#define SPO_BUFF 1024*4
using namespace std;

int SPOLISTS = 7;
bool verbose = true;
bool uricheck = false;

bool check_prefix(string str);
string check_var(string& var);
void write_file(vector<POLY>& full_query,string path,int query_num);
void write_file(vector<vector<POLY> >& query_lists,string path,int query_num);

int main(int argc, char **argv){

  if(argc!=3){
    cout<<"Usage : <input_query> <output_file>"<<endl;
    exit(2);
  }

  /*Query form 

    {
      ?a b c .
      ?b c d .
    }
  */

  ifstream ifs_query(argv[1]);
  if(!ifs_query.is_open()){
    cout<<"Error! Unable to open the input file."<<endl;
    exit(2);
  }

  //cout<<"file :"<<argv[1]<<endl;
  string query="why";
  bool is_query=false;
  int query_num=0;
  vector<POLY>full_query;
  vector<vector<POLY > > query_lists(SPOLISTS);
  
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
     if(verbose){
       	cout<<check_prefix(s); 
        cout<<check_prefix(p);
        cout<<check_prefix(o);
        cout<<endl;
      }

      //check prefix (fails if prefix exists in query)
      if(uricheck){
      	if(check_prefix(s) || check_prefix(p) || check_prefix(o)){
        	  cout<<"Error: Absence of \" or < in the start of literal/URI - Possible prefix."<<endl;
          	string trip_temp="( "+s+" , "+p+" , "+o+" )";
          	cout<<trip_temp<<endl;
          	exit(2);
      	}
     }
      
      s=check_var(s);
      p=check_var(p);
      o=check_var(o);

      string store="( "+s+" , "+p+" , "+o+" )";

      if(verbose)
      	cout<<store<<endl;
      /*
      cout<<"query:"<<query<<endl;
      cout<< "The s is :"<<s<<":";
      cout<<"   The p is :"<<p<<":";
      cout<<"   The o is :"<<o<<":"<<endl<<endl;
      */

      // TODO: make arg
      POLY irrPoly = 1675850375;
      POLY pol;
      pol=hashIrrPoly(&store[0],irrPoly);
      if(verbose)
  	    cout<<"Poly :"<<pol<<endl;

      // ( s, p, o )
      if (s != "?" && p != "?" && o != "?") {
        query_lists[0].push_back(pol); 
      // ( s, p, ? )
      } else if (s != "?" && p != "?" && o == "?") {
        query_lists[1].push_back(pol); 
      // ( s, ?, o )
      } else if (s != "?" && p == "?" && o != "?") {
        query_lists[2].push_back(pol); 
      // ( ?, p, o )
      } else if (s == "?" && p != "?" && o != "?") {
        query_lists[3].push_back(pol); 
      // ( s, ?, ? )
      } else if (s != "?" && p == "?" && o == "?") {
        query_lists[4].push_back(pol); 
      // ( ?, p, ? )
      } else if (s == "?" && p != "?" && o == "?") {
        query_lists[5].push_back(pol); 
      // ( ?, ?, o )
      } else if (s == "?" && p == "?" && o != "?") {
        query_lists[6].push_back(pol); 
      } else {
        cout << "invalid pattern: all vars\n";
      }

      //full_query.push_back(pol);     
    } 
   
  }
  write_file(query_lists,argv[2],query_num);
  //write_file(full_query,argv[2],query_num);
  return 2;
}

// format: <# of sigs: 1> <id: 0> <SPO list size> <POLY>...
void write_file(vector<vector<POLY> >& query_lists, string path,int query_num){

  //query_num is extra information (not used), use query_id statically instead
  int query_id=0;

  assert(query_lists.size() == SPOLISTS);

  int def_val=1;
  FILE *opf;
  opf = fopen(&path[0],"w");
  if(opf==NULL){
    cout<<"Error! Could not open the output file."<<endl;
    exit(2);
  } 

  fwrite((int*)&def_val,sizeof(int),1,opf);
  fwrite((int*)&query_id,sizeof(int),1,opf);

  for (int i = 0; i < SPOLISTS; i++) {
    int query_size=query_lists[i].size();
    if (query_size == 0) {
      cout << "warning: list " << i << " empty\n";
      fwrite((int*)&query_size,sizeof(int),1,opf);
      continue;
    } else {
      cout<<"list " << i << " size :"<<query_lists[i].size()<<endl;
    }

    sort(query_lists[i].begin(),query_lists[i].end());

    vector<POLY>::iterator it;
    it=query_lists[i].begin();
    fwrite((int*)&query_size,sizeof(int),1,opf);
    for(;it!=query_lists[i].end();it++){
      POLY temp=*it;
      //bool check_prefix(string str);
      fwrite((POLY*)&temp,sizeof(POLY),1,opf);
      //cout << temp << endl;
    }
  }
  
  fclose(opf);
}
 
// format: <# of sigs: 1> <list size> <POLY>... <id: 0>
void write_file(vector<POLY>& full_query, string path,int query_num){

  //query_num is extra information (not used), use query_id statically instead
  int query_id=0;

  int query_size=full_query.size();
  int def_val=1;
  
  //cout<<"vector size :"<<full_query.size()<<endl;
  sort(full_query.begin(),full_query.end());
  FILE *opf;
  opf = fopen(&path[0],"w");
  if(opf==NULL){
    cout<<"Error! Could not open the output file."<<endl;
    exit(2);
  }
  vector<POLY>::iterator it;
  it=full_query.begin();
  fwrite((int*)&def_val,sizeof(int),1,opf);
  fwrite((int*)&query_size,sizeof(int),1,opf);
  for(;it!=full_query.end();it++){
    POLY temp=*it;
    bool check_prefix(string str);
    fwrite((POLY*)&temp,sizeof(POLY),1,opf);
  }
  fwrite((int*)&query_id,sizeof(int),1,opf);
  
  fclose(opf);
}

string check_var(string& var){
  if(var.substr(0,1)=="?")
    return "?";
  else
    return var;
}

bool check_prefix(string str){
  if(str[0] == '<' || str[0] == '"' || str[0] == '?')
       return false;
  else
       return true;
}
