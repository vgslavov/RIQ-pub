// Author: Praveen Rao
// UMKC, 2008

// for help on STL refer to http://www.sgi.com/tech/stl/
#include <iostream>
#include <stdio.h>
#include <assert.h>
#include <vector>
#include <map>
#include <queue>
#include <stdlib.h>
#include <sstream>
#include <fstream>
#include <string.h>

#include "utils.h" // contains all the apis required to manipulate polys
using namespace std;
void writeOut(map<int, vector<pair<int, int> > > &, string, char *, char *, unsigned long);

int finalDocid = 0;
// Number of hops
int HopBit = -2;

struct ltstr
{
  bool operator()( std::vector<int> s1, std::vector<int> s2) const
  {
    if(s1[0] < s2[0])
      return true;
    if(s1[0] > s2[0])
      return false;

    //continue to element 2, if element 1 is equal
    if(s1[1] < s2[1])
      return true;
    if(s1[1] > s2[1])
      return false;
    
    //contine to element 3, if element 2 is also equal
    if(s1[2] < s2[2])
      return true;
    return false;
  }
};

int main(int argc, char *argv[])
{

  printf("Author: Praveen Rao, UMKC 2008.\n");
  if (argc != 8) {
    cout << "Usage: " << argv[0]
         << " <input file>"
	 << " <Queuefile>"
	 << " <CutList_ID>"
	 << " <ProgressFile>"
	 << " <VertexWeights>"
	 << " <Cutoff-Weight>"
	 << " <# of hops: negative!>"
         << endl;
    exit(1);
  }
  
    // Open files
  FILE *fpin = fopen(argv[1], "r");
  assert(fpin);

  HopBit = strtol(argv[7], NULL, 10);
  cout << "# of hops: " << HopBit << endl;

  //vector<pair<string, int> > adjVertex;

  map<int, vector<pair<int, int> > > edgeSet;

  char buf[4 * 1024];
  int docId;

  while (1) {

    if (fgets(buf, sizeof(buf), fpin) == NULL) break;
    char type; 
    int vid, vid1, vid2, elabel;
    int edgeCount;
    //char vlabel[4 * 1024], elabel[4 * 1024];

    sscanf(buf, "%c", &type);	 

    switch(type) {
    case '#':
      cout << "COMMENT line\n";
      //printf("COMMENT line\n");
      break;
    case 't':
      {
	cout << "BEGIN new: \n";
	//printf("BEGIN new: \n");
	char junk;
	sscanf(buf, "%c %c %d", &type, &junk, &docId);
	printf("%d\n", docId);
	cout << flush;
	edgeSet.clear();
      }
      break;
    case 'u':
      {
	sscanf(buf, "%c %d %d %d", &type, &vid1, &vid2, &elabel); 
	/*stringstream source, pred;
	source << vid1;
	pred << elabel; 
	string tripl = source.str() + " " + pred.str();
	pair<string, int> e(tripl, vid2);
	adjVertex.push_back(e);*/

	map<int, vector<pair<int, int> > >::iterator itr;
	itr = edgeSet.find(vid1);
	//find all the vertices with the same subject node
	if (itr != edgeSet.end()) {
	  pair<int, int> e2(vid2, elabel);
	  itr->second.push_back(e2);
	}
	else {
	  vector<pair<int, int> > v;
	  pair<int, int> e2(vid2, elabel);
	  v.push_back(e2);
	  edgeSet[vid1] = v;
	}
	//find all the vertices with the same object node
	itr = edgeSet.find(vid2);
	if (itr != edgeSet.end()) {
	  pair<int, int> e2(-1*vid1, elabel);
	  itr->second.push_back(e2);
	}
	else {
	  vector<pair<int, int> > v;
	  pair<int, int> e2(-1*vid1, elabel);
	  v.push_back(e2);
	  edgeSet[vid2] = v;
	}
      }
      break;
    case 'v':
      {
	//sscanf(buf, "%c %d %[^\n]s", &type, &vid, vlabel);
      }
      break;
    }
  }
  cout << "finished reading\n";
  cout << flush;
  //printf("finished reading\n");

	FILE *fpo = fopen(argv[5],"w");
	map<int, vector<pair<int, int> > >::iterator itr1;
        for(itr1=edgeSet.begin(); itr1 != edgeSet.end(); itr1++) {
		fprintf(fpo, "%d %lu\n",itr1->first, itr1->second.size());		}
	fclose(fpo);	


  string arg2(argv[2]);
  unsigned long cutoff = strtoul(argv[6], NULL, 0);
  writeOut(edgeSet, arg2, argv[3], argv[4], cutoff);
  //adjVertex.clear();
  edgeSet.clear();
  fclose(fpin);
  return 0;
}

void writeOut(map<int, vector<pair<int, int> > >& edgeSet, string QFile, char *CutList_ID, char *ProgressFile, unsigned long cutoff)
{
  map<std::vector<int>, bool, ltstr> visitedEdge;
  
  cout << "# of hops (in writeOut): " << HopBit << endl;

  // initialize each edge to be "not visited"
  /*for (int i = 0; i < adjVertex.size(); i++) {
    stringstream sink;
    sink << adjVertex[i].second;
    string tripl = adjVertex[i].first + " " + sink.str() + " .";
    visitedEdge[tripl] = false;
    }*/

  queue<int> Q;
  map<int, vector<pair<int, int> > >::iterator itr = edgeSet.begin();
  int quot = 0;

  ifstream fil(&QFile[0]);
  if(!fil) {
    cout << "\nError in opening the file for queue!\n";
    exit(1);
  }
  string lin;
  while(getline(fil,lin))
    quot++;
  fil.close();
  int quotD = 100;
  int qt = quot / quotD;
  if(qt == 0) {
    quotD = 10;
    qt = quot / quotD;
  }
  if(qt == 0) {
    quotD = 1;
    qt = quot;
  }
  quot = qt;

  int iVertex;
  ifstream qfile(&QFile[0]);
  if(!qfile) {
    cout << "\nError in opening the file for queue!\n";
    exit(1);
  }
  FILE *fpout = fopen(CutList_ID, "w");
  int cutgraphsCount = 0;
  int progressCount = 0;
  cout << "\nTraversal of n-hop CutGraph started..\n";
  cout << flush;
  FILE *fpprgs = fopen(ProgressFile, "w");
  while(getline(qfile,lin)) {
    istringstream q_iss(lin);
    while(q_iss >> iVertex) {
      //Push a vertex into the queue
      Q.push(iVertex);
    }
    int a = -2;
    Q.push(a);
    bool stopqueuing = false;
    if(HopBit == -2) {
	cout << "# of hops == -2: " << HopBit << endl;
	stopqueuing = true;
    }

    int con_Count = 0;

    fprintf(fpout, "t #%d\n",cutgraphsCount);
    // If there are elements in the queue
    while (Q.size() != 0) {
      int v1 = Q.front();
      Q.pop();
      if(v1 < 0) {
	if(v1 == HopBit)
	  break;
	else{
	  //need not push vertices into queue b'coz 3 hops have been achieved already
	  if(v1-1 == HopBit)
	    stopqueuing = true;
	  Q.push(v1-1);
	  v1 = Q.front();
	  Q.pop();
	}
      }
      
      itr = edgeSet.find(v1);
      if (itr != edgeSet.end()) {	  
	if(itr->second.size() <= cutoff) {
	for (int i = 0; i < itr->second.size(); i++) {
	  //stringstream source, sink, pred;
	  int source, sink, pred, next;
	  vector<int> test_v;
	  if(itr->second[i].first >= 0) {
	    //source << v1;
	    //sink << itr->second[i].first;
	    sink = itr->second[i].first;
	    source = v1;
	    next = sink;
	  }
	  else {
	    //source << (-1 * itr->second[i].first);
	    //sink << v1;
	    source = (-1 * itr->second[i].first);
	    next = source;
	    sink = v1;
	  }
	  pred = itr->second[i].second;
	
	  test_v.push_back(source);
	  test_v.push_back(pred);
	  test_v.push_back(sink);	  
	  //string triple = source.str() + " " + pred.str() + " " + sink.str() + " .";	
	  
	  // Visited Edge
	  if (visitedEdge.find(test_v) == visitedEdge.end()) {
	    visitedEdge[test_v] = true;
	    if(!stopqueuing)
	      Q.push(next);
	    fprintf(fpout, "u %d %d %d\n", source, sink, pred);
	    con_Count++;
	  }
	}
	}
	//edgeSet.erase(itr);
      }
    }	 
    fprintf(fpout, "\n");
    visitedEdge.clear();
    cutgraphsCount++;    
    fprintf(fpprgs, "Size of cut graph %d is: %d\n", cutgraphsCount, con_Count);
    cout << flush;
    if(cutgraphsCount % quot == 0) {
      progressCount++;
      cout << "No of cut graphs processed: " << cutgraphsCount << " --> " << progressCount << "/" << quotD <<"\n";
      cout << flush;
    }
  }
  fclose(fpprgs);
  qfile.close();
  fclose(fpout);
}
  
