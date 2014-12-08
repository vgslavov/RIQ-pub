#include<stdio.h>
//#include<conio.h>
#include<iostream>
#include<stdlib.h>
using namespace std;
int main(int argc, char* argv[]){
  
   FILE* fp;
   if(argc!=3){
      cout<<"Usage : <Input_File> <seek_pos>"<<endl;
      exit(0);
   }
   long seek_pos=atoi(argv[2]);
   seek_pos--;
   fp = fopen(argv[1],"r");
   if(fp==NULL){
      cout<<"File coudn't open !"<<endl;
      exit(0);
   }
   fseek(fp,seek_pos,SEEK_SET);
   char t[500];
   fread(t,sizeof(t),1,fp);
   cout<<"t :"<<t<<endl;
   
   fclose(fp);
   return 2;
}
