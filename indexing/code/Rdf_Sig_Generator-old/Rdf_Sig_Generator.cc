/*	AUTHOR     : Srivenu Paturi
*	DESCRIPTION: Produces Signature file and Offset meta file
*                    (corresponding to the original Ntriples file).
*
*
*	
*
*/

#include<iostream>
#include<fstream>
#include<stdlib.h>
#include<vector>
#include<cstring>
#include<algorithm>
#include<sstream>
#include"utils.h"
#include<ctime>

#define VECTOR_CAPACITY 10000
#define SPANNING_MAX 10000     //Without *7
using namespace std;
void Process_Pattern(string,bool[]);
void File_Input(char*,char*,bool[]);
void Triple_Processor(vector<POLY> &x,char*,char*,bool[]);
void Display(vector<vector<POLY> > &y);
void Write_File(vector<vector<POLY> > &records, string, int, int, long);
string Rem_Space(char*);

bool verbose = false;

int main(int argc, char* argv[]) {
  if((argc!=3&&argc!=4)){
    cout<<"Usage: <Input_Files_List> <Output_Filename> [Pattern]"<<endl
	<<"----------------------------"<<endl
        <<"	- PATERN LIST - "<<endl
	<<"----------------------------"<<endl
        <<"	0 	  ALL   (Default)"<<endl
        <<"	1       <S,P,0>"<<endl
        <<"	2 	<S,P,?>"<<endl
        <<"	3 	<S,?,O>"<<endl
        <<"	4 	<?,P,O>"<<endl
        <<"	5 	<S,?,?>"<<endl
        <<"	6 	<?,P,?>"<<endl
        <<"	7 	<?,?,O>"<<endl
	<<"----------------------------"<<endl;

    exit(2);
  }
  bool pattern[8] = { false };
  
  if(argc==4)
     Process_Pattern(argv[3], pattern);
  else
     pattern[0] = true;

   if(pattern[0])
      for(int i=1; i<=7;i++)
          pattern[i] = true;

   /* Printing the pattern control variable */
   //for(int i=0; i<=7;i++)
      // cout<<pattern[i]<<endl; 

  File_Input(argv[1], argv[2], pattern);
  return 0;
}

/*This method sets up the bool pattern[] according to the input pattern list*/
void Process_Pattern(string input_pattern, bool pattern[]){

   int count=0;
   while(count < input_pattern.length()){
      int sub_patt = input_pattern[count] - '0';
      pattern[sub_patt] = true;
      count++;
   }
//   for(int i=0; i<=7;i++)
//     cout<<pattern[i]<<endl; 
        
}


/* This method reads all the partitioned files from the disk, reads each line from every file, eliminates leading white spaces and new lines and gives the line to Triple_Processor*/

void File_Input(char *loc, char *dest, bool pattern[]){ 

//Statistics Variables
  unsigned int sig_wrt=0;
  unsigned int sig_ign=0;
  long nt_wrt=0;
  long avg_sig=0;
  int larg_sig=0;
  clock_t start_time=clock();


  ifstream list;    
  list.open(loc);
  vector<vector<POLY> > records;
  records.reserve(VECTOR_CAPACITY);
 
  int i=0;
  if(!list.is_open()){//If the list file doesn't open
    cout<<"Could not open the list of files, program terminating!"<<endl;
    exit(2);
  }
  string file;
  while (getline(list,file)){//Reading the path of each partition file
    ifstream data;
    data.open(&file[0]);
    if(!data.is_open()){ //If a partition file doesn't open
      cout<<"Could not open "<<file<<". Skipping it !!!"<<endl;
      continue;
    }
    string temp;
    int sig_size=0;
    int graph_id=0;
    long seek_loc=0;
    // bool new_sig=true;
    while(getline(data,temp)){		// while reading triples from the partition file
         
      if(strcmp(&temp[0],"")==0){   	//End of a graph signature

        if(records.empty())
	  continue;

        if(records.size()<=SPANNING_MAX){	//signature is good to write (WITHOUT *7)
	  //cout<<"The graph id is : "<<graph_id<<endl;
	  //cout<<"The location is : "<<seek_loc<<endl<<endl;
          //cout<<"sig_size :"<<sig_size<<endl;
          //cout<<"records.size():"<<records.size()<<endl;
 
	  //updating the stats
          sig_wrt++;
          nt_wrt=nt_wrt+records.size();
          if(records.size()>larg_sig)
             larg_sig=records.size();

	  Write_File(records,dest,sig_size,graph_id,seek_loc);
	  graph_id++;
        }
        else{
           cout<<"Skipped the graph with id : "<<graph_id<<" with size :"<<records.size()<<endl;
           sig_ign++;
        }
        sig_size=0;       
        records.clear();
        if(seek_loc>data.tellg()){
	  cout<< "Error: \"seek_loc: int\" datatype overflow!!!"<<endl;
          exit(2);
	}
        seek_loc=data.tellg();
        continue;

      }
      if(records.size()>SPANNING_MAX)
         continue;
      vector<POLY> row;
      Triple_Processor(row,&temp[0],&file[0],pattern);//Triple processing
      records.push_back(row);
      sig_size=sig_size+row.size();
      i++;
    }

    //Writing last signature
    if(records.size()<=SPANNING_MAX){
       sig_wrt++;
       nt_wrt=nt_wrt+records.size();
       if(records.size()>larg_sig)
         larg_sig=records.size();
       Write_File(records,dest,sig_size,graph_id,seek_loc);
    }
    else{
           cout<<"Skipped the graph with id : "<<graph_id<<" with size :"<<records.size()<<endl;
           sig_ign++;
        }
    records.clear();
    data.close();
    // Display(records);
  }
  list.close();
  double tot_time = (double)(clock()-start_time)/(double)(CLOCKS_PER_SEC);
  nt_wrt=nt_wrt*7;
  larg_sig=larg_sig*7;
  avg_sig=nt_wrt/sig_wrt;             // int/int division

  //printing the log details
  cout<<"*************************************"<<endl;
  cout<<"             LOG_INFO                "<<endl;
  cout<<"*************************************"<<endl;
  cout<<"Number of signatures ignored                 : "<<sig_ign<<endl;
  cout<<"Number of signatures written                 : "<<sig_wrt<<endl;
  cout<<"Total number of Ntriples wrt(including *7)   : "<<nt_wrt<<endl;
  cout<<"Largest sig size by NTriples #(including *7) : "<<larg_sig<<endl; 
  cout<<"Average size of written sig                  : "<<avg_sig<<endl;
  cout<<"Total time taken to write sig                : "<<tot_time<<endl;
  cout<<"*************************************"<<endl;


}

/*Triple_Processor takes triples and parses it to arrange S P O in the seven possible cases of variables and known elements*/

void Triple_Processor(vector<POLY> &vect, char *rec,char* file, bool pattern[])
{
    string triple = rec;
    char* sep;
    char *sep2;
    string s,p,o;
    int setter = 0;
    bool flag = false;
    char str_end;
    sep = strtok(&triple[0]," ");
    string temp = "";
    while (sep != NULL) {
        string temp_sep = sep;
        if (temp_sep.at(0) == '<' || temp_sep.at(0) == '"' || flag) {
            if (flag == 0) {
                if (temp_sep.at(0) == '<')
                    str_end = '>';
                else if (temp_sep.at(0) == '"')
                    str_end = '"';
                else {
                    cout<<"Error in handling triples with spaces in partition "<<file<<". Program terminating!";
                    exit(2);
                }
            }
            if (temp_sep.at(temp_sep.length()-1)!=str_end) { //base case-1
                flag=true;
                temp.append(sep);
                temp.append(" ");
                sep=strtok(NULL," ");
                continue;
            }
            temp.append(sep);
            temp_sep=temp;
            //     cout<<"temp is:"<<temp_sep<<endl;
            flag=false;
        }

        if (strcmp(sep,".")==0) {//"." case
            break;
        }

        if (setter==0) {
            s=temp_sep;
            setter++;
            sep=strtok(NULL," ");
            temp="";
            // cout<<"S :"<<s<<endl;
        } else if (setter==1) {
            p=temp_sep;
            setter++;
            sep=strtok(NULL," ");
            temp="";
            // cout<<"P :"<<p<<endl;
        } else if (setter==2) {
            o=temp_sep;
            setter++;
            sep=strtok(NULL," ");
            temp="";
            // cout<<"O :"<<o<<endl;
        } else {
            cout<<"Error Processing a triple in the partition "<<file<<". Terminating the program!"<<endl;
            cout<<"triple: " << triple << endl;
            exit(2);
        }
    }

    POLY irrPoly = 1675850375;
    POLY pol = 0;
    stringstream ss("");
    string store ="";

    if (pattern[1]) {
        ss.str("");
        ss<<"( "<<s<<" , "<<p<<" , "<<o<<" )";   //( S , P , O )
        store=ss.str();
        if (verbose) cout<<store<<endl;
        pol = hashIrrPoly(&store[0],irrPoly);
        if (verbose) cout<<pol<<endl<<endl;
        vect.push_back(pol);                   
    }

    if (pattern[2]) {
       ss.str("");
       ss<<"( "<<s<<" , "<<p<<" , "<<"?"<<" )"; //( S , P , ? )
       store = ss.str();
       if (verbose) cout<<store<<endl;
       pol = hashIrrPoly(&store[0],irrPoly);
       if (verbose) cout<<pol<<endl<<endl;
       vect.push_back(pol);
    }

    if (pattern[3]) {
        ss.str("");
        ss<<"( "<<s<<" , "<<"?"<<" , "<<o<<" )"; //( S , ? , O ) 
        store = ss.str();
        if (verbose) cout<<store<<endl;
        pol = hashIrrPoly(&store[0],irrPoly);
        if (verbose) cout<<pol<<endl<<endl;
        vect.push_back(pol);
    }

    if (pattern[4]) {
        ss.str("");
        ss<<"( "<<"?"<<" , "<<p<<" , "<<o<<" )"; //( ? , P , O ) 
        store=ss.str();
        if (verbose) cout<<store<<endl;
        pol=hashIrrPoly(&store[0],irrPoly);
        if (verbose) cout<<pol<<endl<<endl;
        vect.push_back(pol);
    }

    if (pattern[5]) {
        ss.str("");
        ss<<"( "<<s<<" , "<<"?"<<" , "<<"?"<<" )"; //( S , ? , ? )
        store=ss.str();
        if (verbose) cout<<store<<endl;
        pol=hashIrrPoly(&store[0],irrPoly);
        if (verbose) cout<<pol<<endl<<endl;
        vect.push_back(pol);
    }

    if (pattern[6]) {
        ss.str("");
        ss<<"( "<<"?"<<" , "<<p<<" , "<<"?"<<" )"; //( ? , P , ? )
        store=ss.str();
        if (verbose) cout<<store<<endl;
        pol=hashIrrPoly(&store[0],irrPoly);
        if (verbose) cout<<pol<<": "<<endl<<endl;
        vect.push_back(pol);
    }

    if (pattern[7]) {
        ss.str("");
        ss<<"( "<<"?"<<" , "<<"?"<<" , "<<o<<" )"; //( ? , ? , O )
        store=ss.str();
        if (verbose) cout<<store<<endl;
        pol=hashIrrPoly(&store[0],irrPoly);
        if (verbose) cout<<pol<<endl<<endl;
        vect.push_back(pol);
    }
}

/*This method is useful to print all the processed data of the currently processing partition file*/

void Display(vector<vector<string > > &records)
{
  cout<<records.size()<<endl;int i,j;
  for(i=0;i<records.size();i++)
    for(j=0;j<records[i].size();j++)
      cout<<records[i][j]<<endl;
 
}

//Writes a signature vector and its offset (refered to origial .n3 file) to the output files *.DAT and *.META
void Write_File(vector<vector<POLY> > &records, string temp, int sig_size, int graph_id, long seek_loc)
{

  if(records.empty())  
    return ;
  
  string out_file=temp+".DAT";
  string meta_file = temp+".META";
  FILE *fp;
  fp = fopen(&out_file[0],"a+");
  if(fp==NULL){//If the output file doesn't open
    cout<<"Could not open the output file, program terminating!"<<endl;
    exit(2);
  }
    
  //Writing the sorted signature(DAT) file.
  vector<POLY> comb;
  int def_val=1;
  int i=0,j=0;
  for(i=0;i<records.size();i++)
    for(j=0;j<records[i].size();j++)
      comb.push_back(records[i][j]);
  sort(comb.begin(),comb.end());

  // cout<<"Sig_size :"<<sig_size<<"      Vector_size :"<<comb.size()<<endl;

      
  fwrite(&def_val,sizeof(int),1,fp);
  fwrite(&sig_size,sizeof(int),1,fp);
  for(i=0;i<comb.size();i++){
     fwrite(&comb[i],sizeof(POLY),1,fp);
   //  cout<<comb[i]<<endl;
  }
  fwrite(&graph_id,sizeof(int),1,fp);
  fclose(fp);
 

/*
  FILE *fp2;
  fp2 = fopen(&meta_file[0],"a+");
  cout<<graph_id<<": "<< seek_loc<<endl;
  if(fp2==NULL){
    cout<<"Unable to open output META file. Please check!"<<endl;
    exit(2);
  }
  fwrite(&seek_loc,sizeof(long),1,fp)

*/


  //Writing the offset(META) file.
  ofstream ofs2(&meta_file[0],ios::app);
//  cout<<graph_id<<": "<< seek_loc<<endl;
  if(!ofs2.is_open()){
    cout<<"Unable to open output META file. Please check!"<<endl;
    exit(2);
  }
  ofs2<<seek_loc<<endl;
  ofs2.close();



}


/*This method trims a string of the leading white spaces*/

string Rem_Space(char *spr)
{
  string s=spr;
  s.erase(s.begin(),find_if(s.begin(), s.end(),not1(ptr_fun<int, int>(isspace))));
  return s;
}

