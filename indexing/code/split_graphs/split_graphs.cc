#include<iostream>
#include<algorithm>
#include<vector>
#include<cstring>
#include<cstdlib>
#include<fstream>
#include<string>

using namespace std;

int main(int argc, char** argv)
{
   if(!(argc==6)){
      cout<<"Usage: <NTriples_file> <NTriples.META_file> <Group_Ids_list> <Prefix_file> <suffix>"<<endl;
      exit(2);
   }
   ifstream ifs_NTriples(argv[1]);
   ifstream ifs_ids(argv[3]);

   string file_name=argv[3];
   string db_name=argv[3];
   string suffix=argv[5];
   db_name=db_name+".db";
   file_name=file_name+"."+suffix;
   string command="cp ";
   command=command+argv[4]+" "+file_name;
   int fail=system(&command[0]);
   if(fail){
      cout<<"Error! Unable to copy prefix file to output file"<<endl;
      exit(2);
   }
  
   ofstream ofs_ids(&file_name[0],ofstream::app);   
//   ofstream ofs_ids(&file_name[0]);
   if(!(ifs_NTriples.is_open() && ifs_ids.is_open() && ofs_ids.is_open())){
    cout<<"Unable to open an input file. Please check!"<<endl;
     exit(2);
    }
   

    ifstream ifm(argv[2]);
    if(!ifm.is_open()){
        cout<<"2 Unable to open an input file. Please check!"<<argv[2]<<endl;
      exit(2);
    }
    vector<unsigned long long> meta_array;
    // TODO: get # of lines in advance
    // TODO: don't load META file for each group, do it once
    meta_array.reserve(10000000);
    unsigned long long meta_data=0;
    string str="";
    while(getline(ifm,str)){
        
       meta_data=atol(&str[0]);
       meta_array.push_back(meta_data);
   }
   cout<<"Size of the META data loaded: "<<meta_array.size()<<endl;
   ifm.close(); 


    string line="";
    while(getline(ifs_ids,line)){
      //add logic here
      unsigned int graph_id = atoi(&line[0]);
      unsigned long long offset = meta_array.at(graph_id);
      cout<< "offset is ........>>:"<<offset<<endl;
      ifs_NTriples.seekg(offset,ios::beg);
      string ntriple="";
      while(getline(ifs_NTriples,ntriple)){
         if(ntriple=="")
            break;
         ofs_ids<<ntriple<<endl;
      }


//      ofs_ids<<"end is here"<<endl;
    }
 

  /*   string db_command="rdf3xload ";
    db_command=db_command+db_name+" "+file_name;
     int fail2=system(&db_command[0]);
      if(fail2){
      cout<<"Error! Unable to rdf3xload the file to output file"<<endl;
      exit(2);
   }
*/

/*
    ifs.seekg(atoi(argv[2]),ios::beg);
    int count=0;
    string dat="";
    getline(ifs,dat);
    cout<<dat<<endl;
   // while(getline(ifs,dat)){
   // }
*/
   ifs_NTriples.close();
   ifs_ids.close();
   ofs_ids.close();
   return 2;
}
