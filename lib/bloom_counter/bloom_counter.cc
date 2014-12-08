/*
 * All Bloom filter code based on:
 * http://en.literateprograms.org/Bloom_filter_(C)
 * (available under the Creative Commons (CCO) license)
 * and modified and enhanced by Srivenu Paturi (sp895@umkc.edu)
 */

#include<limits.h>
#include<stdarg.h>
#include"bloom_counter.h"
#include<iostream>
#include<fstream>
#include<assert.h>
#include<string.h>
#include<math.h>
#include<vector>
#include<map>

#include<sys/mman.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<fcntl.h>

#include "utils.h" // contains all the apis required to manipulate polys
#define SETBIT(a, n) (a[n/CHAR_BIT] |= (1<<(n%CHAR_BIT)))
#define GETBIT(a, n) (a[n/CHAR_BIT] & (1<<(n%CHAR_BIT)))
#define COUNT_RANGE 4;//range of counter in bits

using namespace std;
extern long int warning_num=0;
//BLOOM *bloom_create(size_t size, size_t nfuncs, ...)
BLOOM *bloom_create(size_t size, size_t nfuncs, size_t range) {
  BLOOM *bloom;
  
  if(!(bloom=(BLOOM*)malloc(sizeof(BLOOM)))) 
    return NULL;
  bloom->range=range;
  //bloom->range=COUNT_RANGE;
  //cout<<"size :"<<size<<endl;
  //cout<<"CHAR_BIT :"<<CHAR_BIT<<endl;
  //cout<<"bloom->range :"<<bloom->range<<endl;
  
  //Overflowing hash range
  if(size>RAND_MAX)
	assert(0);

  cout<<"Array size is :"<<(((size*bloom->range)+CHAR_BIT-1)/CHAR_BIT)<<endl;
  //below equation rounds off the filter size to the next char
  if(!(bloom->a=(unsigned char*)calloc(((size*bloom->range)+CHAR_BIT-1)/CHAR_BIT, sizeof(char)))) {		
    free(bloom);
    cout<<"Unable to create the bloom, program terminating !!"<<endl;
    exit(2);
    return NULL;
  }
  //  cout<<"Bloom is :"<<bloom->a<<endl;
  if(!(bloom->para_a=(int*)malloc(nfuncs*sizeof(int)))) {
    free(bloom->a);
    free(bloom);
    cout<<"Unable to create the bloom, program terminating !!"<<endl;
    exit(2);
    return NULL;	
  }
  if(!(bloom->para_b=(int*)malloc(nfuncs*sizeof(int)))){
    free(bloom->a);
    free(bloom);
    cout<<"Unable to create the bloom, program terminating !!"<<endl;
    exit(2);
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

BLOOM* bloom_reset (BLOOM *bloom){

  free(bloom->a);
  size_t size = bloom->asize;
  if(!(bloom->a=(unsigned char*)calloc(((size*bloom->range)+CHAR_BIT-1)/CHAR_BIT, sizeof(char)))) {
     cout<<"Unable to reset the bloom, program terminating !!"<<endl;
     exit(2);
     return NULL;
  }

}

int bloom_destroy(BLOOM *bloom) {
  free(bloom->a);
  free(bloom->para_a);
  free(bloom->para_b);
  free(bloom);
  return 0;
}

int bloom_update(BLOOM *bloom, size_t n, int calling) {
  unsigned char num;
  num=0;
  //cout<<"n in the array is :"<<n<<endl;

  //Extracting the bloom byte
  size_t source=(((n)*bloom->range)/CHAR_BIT);
  memcpy(&num,&bloom->a[source],1); 
  //cout<<"Raw Byte value (before1) :"<<(short)num<<endl;
  

  //Extracting the bloom counter
  unsigned char dup_num;
  dup_num=num;
  int position=((n*bloom->range)%CHAR_BIT);
  
  int left_shift=(CHAR_BIT-position-bloom->range);
  int right_shift=(CHAR_BIT-bloom->range);
  dup_num=dup_num<<left_shift;
  dup_num=dup_num>>right_shift;

  //returning the count to bloom_check
  if(calling==1)
    return dup_num;  

  //Counter exceeding the range
  if(dup_num==(pow(2,bloom->range)-1)){
    //cout<<"Error: Counter overflow, program terminating!"<<endl;
    //cout << "Warning: counter will overflow, skipping update." << endl;
    warning_num++;
    return 1;
    //exit(2);
  }

  //Increamenting the bloom counter
  dup_num++;
  
  //cout<<"Updated counter-"<<n<<"  :  "<<(short)dup_num<<endl;
  //cout<<"Raw Byte value (before2) :"<<(short)num<<endl;
  //Making operand-1 ready
  dup_num=dup_num<<position;


  //Making counter in bloom byte to 0s for operand-2
  unsigned char temp=0;
  int count=bloom->range;
  while(count>0) {
    temp=temp<<1;
    temp=temp|1;
    count--;
  }
  temp=~(temp<<position);
  num=num&temp;


  //cout<<"Position is :"<<position<<endl;
  //cout<<"Source Byte index is :"<<source<<endl;

  //OR operation on the operands.
  //cout<<"Updated Operands :"<<(short)num<<" and :"<<(short)dup_num<<endl;
  num=num|dup_num;
 
  //Writing Back to the memory  
  memcpy(&bloom->a[source],&num,1);

  //cout<<"Updated Byte :"<<(short)num<<endl<<endl;
  return 1;
}


int bloom_add(BLOOM *bloom, vector<int> &data) {
  size_t n;	
  vector<int>::iterator item;
  for(item=data.begin();item!=data.end();++item){
    int hashVal=*item;
    for(n=0; n<bloom->nfuncs; ++n) {		
      size_t hashOutput = (((bloom->para_a[n])*hashVal + bloom->para_b[n]) % RAND_MAX) % (bloom->asize);
      bloom_update(bloom, hashOutput, 0);
    }
  }
  //cout<<endl;
  return 1;
}

int bloom_add(BLOOM *bloom, vector<POLY> &data) {
  size_t n;	
  vector<POLY>::iterator item;
  for(item=data.begin();item!=data.end();++item){
    POLY hashVal=*item;
    for(n=0; n<bloom->nfuncs; ++n) {		
      size_t hashOutput = (((bloom->para_a[n])*hashVal + bloom->para_b[n]) % RAND_MAX) % (bloom->asize);
      bloom_update(bloom, hashOutput, 0);
    }
  }
  //cout<<endl;
  return 1;
}

int bloom_check(BLOOM *bloom, const char *s) {
  size_t n;
  //cout<<"Checking :"<<s<<endl;
  POLY irrPoly = 1675850375;
  // converts string to int
  int hashVal = hashIrrPoly(s, irrPoly);
  for(n=0; n<bloom->nfuncs; ++n) {		
	int t =0;
        t =bloom_update(bloom, (((bloom->para_a[n])*hashVal) + (bloom->para_b[n]))%(bloom->asize), 1);
	if(!t)
	  return 0;
        cout<<"Count = "<<t<<endl;
    
      }
  return 1;
}

//DO NOT USE THIS
int bloom_multi(BLOOM *bloom, vector<int> &query) {
  size_t n;
  map<int, int> query_map;
  vector<int>::iterator element;
  for(element=query.begin();element!=query.end();++element){
    int hashVal=*element;
//    cout<<"Poly : "<<hashVal<<endl;
    for(n=0;n<bloom->nfuncs;++n){
      //int temp=(((bloom->para_a[n])*hashVal)+(bloom->para_b[n]))%(bloom->asize);
      size_t temp=((((bloom->para_a[n])*hashVal)+(bloom->para_b[n])) % RAND_MAX) %(bloom->asize);
      query_map[temp]++;    
    }
  }

  map<int, int>::iterator iter;
  for(iter=query_map.begin();iter!=query_map.end();++iter){
    int key=(*iter).first;
    int query_num=(*iter).second;
    int data_num=bloom_update(bloom,key, 1);
 //   cout<<"Checking <"<<key<<"> for atleast :"<<query_num<<" and it is :"<<data_num<<endl;
    if(data_num<query_num)
      return 0;
  }
  return 1;  
}

int bloom_multi(BLOOM *bloom, vector<POLY> &query) {
  size_t n;
  map<size_t, int> query_map;
  vector<POLY>::iterator element;
  for(element=query.begin();element!=query.end();++element){
    POLY hashVal=*element;
    for(n=0;n<bloom->nfuncs;++n){
      //size_t temp=(((bloom->para_a[n])*hashVal)+(bloom->para_b[n]))%(bloom->asize);
      size_t temp=((((bloom->para_a[n])*hashVal)+(bloom->para_b[n])) % RAND_MAX) % (bloom->asize);
      query_map[temp]++;    
    }
  }
  map<size_t, int>::iterator iter;
  for(iter=query_map.begin();iter!=query_map.end();++iter){
    size_t key=(*iter).first;
    int query_num=(*iter).second;
    int data_num=bloom_update(bloom,key, 1);
    cout<<"Checking <"<<key<<"> for at least :"<<query_num<<" and res:"<<data_num<<endl;
    if(data_num<query_num)
      return 0;
  }
  return 1;  
}

void bloom_write(BLOOM *bloom, const char *path) {
  FILE *fp = fopen(path, "w");
  assert(fp);
  fprintf(fp, "%d\n",bloom->range);
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

void bloom_meta_write(BLOOM *bloom, const char *path) {
  FILE *fp = fopen(path, "w");
  assert(fp);
  fprintf(fp, "%d\n",bloom->range);
  fprintf(fp, "%lu\n",bloom->asize);
  fprintf(fp, "%lu\n",bloom->nfuncs);
  for(int k=0;k<bloom->nfuncs;k++) {
    fprintf(fp,"%d ",bloom->para_a[k]);
  }
  fprintf(fp,"\n");
  for(int k=0;k<bloom->nfuncs;k++) {
    fprintf(fp,"%d ",bloom->para_b[k]);
  }
  // fprintf(fp,"\n");
  // fwrite(bloom->a, sizeof(unsigned char), bloom->asize, fp);  
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
  int range = atoi(&buff[0]);
  getline(ifs,buff);
  size_t size = atoi(&buff[0]);
  getline(ifs,buff);
  size_t nfuncs = atoi(&buff[0]); 
  
  if(!(bloom=(BLOOM*)malloc(sizeof(BLOOM)))) return NULL;
  bloom->range=range;

  if(!(bloom->a=(unsigned char*)calloc(((size+CHAR_BIT-1)*bloom->range)/CHAR_BIT, sizeof(char)))) {		
    free(bloom);
    cout<<"Unable to create the bloom, program terminating !!"<<endl;
    exit(2);
    return NULL;
  }

 if(!(bloom->para_a=(int*)malloc(nfuncs*sizeof(int)))) {
    free(bloom->a);
    free(bloom);
    cout<<"Unable to create the bloom, program terminating !!"<<endl;
    exit(2);
		return NULL;	
  }
  if(!(bloom->para_b=(int*)malloc(nfuncs*sizeof(int)))){
    free(bloom->a);
    free(bloom);
    cout<<"Unable to create the bloom, program terminating !!"<<endl;
    exit(2);	
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

  int temp=(((size*bloom->range)+CHAR_BIT-1)/CHAR_BIT)*sizeof(char);
  if(!ifs.read((char*)bloom->a,temp)){
    cout<<"Error:Unable to read filter from file "<<path<<". Terminating the program!"<<endl;
    exit(2);
  }

  bloom->asize=size;
  bloom->nfuncs=nfuncs;	
  ifs.close();
  return bloom;
}

BLOOM* bloom_read_mmap(const char *path) {
  struct stat sb;
  char *p;
  int fd;
  BLOOM *bloom; 

  /*
  string buff;
  ifstream ifs(path,ios::binary);
  if(!ifs.is_open()) {
    cout<<"Error:Unable to open file "<<path<<". Terminating the program!"<<endl;
    exit(2);
  }
  */

  fd = open(path, O_RDONLY);
  if (fd == -1) {
    cout << "Unable to open file\n";
    return NULL;
  }

  if (fstat(fd, &sb) == -1) {
    cout << "Unable to fstat file\n";
    return NULL;
  }

  if (!S_ISREG(sb.st_mode)) {
    cout << path << " is not a regular file\n";
    return NULL;
  }

  p = (char *)mmap(0, sb.st_size, PROT_READ, MAP_SHARED, fd, 0);
  if (p == MAP_FAILED) {
    cout << "Unable to mmap\n";
    return NULL;
  }

  if (close(fd) == -1) {
    cout << "Unable to close file\n";
    return NULL;
  }

  /*
  getline(ifs,buff);
  int range = atoi(&buff[0]);
  getline(ifs,buff);
  size_t size = atoi(&buff[0]);
  getline(ifs,buff);
  size_t nfuncs = atoi(&buff[0]); 
  
  if(!(bloom=(BLOOM*)malloc(sizeof(BLOOM)))) return NULL;
  bloom->range=range;

  if(!(bloom->a=(unsigned char*)calloc(((size+CHAR_BIT-1)*bloom->range)/CHAR_BIT, sizeof(char)))) {		
    free(bloom);
    cout<<"Unable to create the bloom, program terminating !!"<<endl;
    exit(2);
    return NULL;
  }

 if(!(bloom->para_a=(int*)malloc(nfuncs*sizeof(int)))) {
    free(bloom->a);
    free(bloom);
    cout<<"Unable to create the bloom, program terminating !!"<<endl;
    exit(2);
		return NULL;	
  }
  if(!(bloom->para_b=(int*)malloc(nfuncs*sizeof(int)))){
    free(bloom->a);
    free(bloom);
    cout<<"Unable to create the bloom, program terminating !!"<<endl;
    exit(2);	
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

  int temp=(((size*bloom->range)+CHAR_BIT-1)/CHAR_BIT)*sizeof(char);
  if(!ifs.read((char*)bloom->a,temp)){
    cout<<"Error:Unable to read filter from file "<<path<<". Terminating the program!"<<endl;
    exit(2);
  }

  bloom->asize=size;
  bloom->nfuncs=nfuncs;	
  ifs.close();
  */

  return bloom;
}

BLOOM* bloom_meta_read(const char *path) {
  BLOOM *bloom; 
  string buff;
  ifstream ifs(path,ios::binary);
  if(!ifs.is_open()) {
    cout<<"Error:Unable to open file "<<path<<". Terminating the program!"<<endl;
    exit(2);
  }
  getline(ifs,buff);
  int range = atoi(&buff[0]);
  getline(ifs,buff);
  size_t size = atoi(&buff[0]);
  getline(ifs,buff);
  size_t nfuncs = atoi(&buff[0]); 
  
  if(!(bloom=(BLOOM*)malloc(sizeof(BLOOM)))) return NULL;
  bloom->range=range;

  if(!(bloom->a=(unsigned char*)calloc(((size+CHAR_BIT-1)*bloom->range)/CHAR_BIT, sizeof(char)))) {		
    free(bloom);
    cout<<"Unable to create the bloom, program terminating !!"<<endl;
    exit(2);
    return NULL;
  }

 if(!(bloom->para_a=(int*)malloc(nfuncs*sizeof(int)))) {
    free(bloom->a);
    free(bloom);
    cout<<"Unable to create the bloom, program terminating !!"<<endl;
    exit(2);
		return NULL;	
  }
  if(!(bloom->para_b=(int*)malloc(nfuncs*sizeof(int)))){
    free(bloom->a);
    free(bloom);
    cout<<"Unable to create the bloom, program terminating !!"<<endl;
    exit(2);	
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

  //  int temp=(((size*bloom->range)+CHAR_BIT-1)/CHAR_BIT)*sizeof(char);
  //  if(!ifs.read((char*)bloom->a,temp)){
  //   cout<<"Error:Unable to read filter from file "<<path<<". Terminating the program!"<<endl;
  //  exit(2);
  // }

  bloom->asize=size;
  bloom->nfuncs=nfuncs;	
  ifs.close();
  return bloom;
}

int bloom_extract(BLOOM *bloom, size_t n) {
 
  unsigned char num;
  num=0;

  size_t source=(((n)*bloom->range)/CHAR_BIT);
  memcpy(&num,&bloom->a[source],1);
  //cout<<"Raw Byte value (before1) :"<<(short)num<<endl;

  //Extracting the bloom counter
  unsigned char dup_num;
  dup_num=num;
  size_t position=((n*bloom->range)%CHAR_BIT);

  int left_shift=(CHAR_BIT-position-bloom->range);
  int right_shift=(CHAR_BIT-bloom->range);
  dup_num=dup_num<<left_shift;
  dup_num=dup_num>>right_shift;

  //returning the count to bloom_check
   return dup_num;
}

void bloom_details(BLOOM *bloom) {
  //cout<<" Bloom size :"<<strlen(bloom->a);
  //cout<<" Bloom is :"<< ":"<<endl;

   size_t loop=bloom->asize;
   for(size_t i=0;i<loop;i++)
     cout<<"Size of counter "<<i<<"  :"<<bloom_extract(bloom,i)<<endl;  
}

