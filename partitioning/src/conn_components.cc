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
#include <string>

#include "utils.h" // contains all the apis required to manipulate polys
using namespace std;
void writeOut(FILE *fpout, map<int, vector<pair<int, int> > > &, string);

int finalDocid = 0;
int cc = 1;
int parti = 1;

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
    
    //continue to element 3, if element 2 is also equal
    if(s1[2] < s2[2])
      return true;
    return false;
  }
};

int main(int argc, char *argv[])
{

  printf("Author: Praveen Rao, UMKC 2008.\n");
  if (argc != 3) {
    cout << "Usage: " << argv[0]
         << " <input file>"
	 << " <output filepath>"
         << endl;
    exit(1);
  }
  
    // Open files
  FILE *fpin = fopen(argv[1], "r");
  assert(fpin);

  string arg2(argv[2]);
  string c1 = arg2 + "connected_ntriples";
  string c2 = arg2 + "connected_count";
  FILE *fpout = fopen(c1.c_str(), "w");
  assert(fpout);
  //vector<pair<string, int> > adjVertex;
  map<int, vector<pair<int, int> > > edgeSet;

  char buf[4 * 1024];
  int docId;

  while (1) {

    if (fgets(buf, sizeof(buf), fpin) == NULL) break;
    char type; 
    int vid, vid1, vid2;
    int edgeCount;
    int elabel;
    //char vlabel[4 * 1024], elabel[4 * 1024];

    sscanf(buf, "%c", &type);	 

    switch(type) {
    case '#':
      cout << "COMMENT line\n";
      //printf("COMMENT line\n");
      break;
    case 't':
      {
	writeOut(fpout, edgeSet, c2);
	cout << "BEGIN new: \n";
	char junk;
	sscanf(buf, "%c %c %d", &type, &junk, &docId);
	printf("%d\n", docId);
	cout << flush;
	//adjVertex.clear();
	edgeSet.clear();
      }
      break;
    case 'u':
      {
	sscanf(buf, "%c %d %d %d", &type, &vid1, &vid2, &elabel); 
	//stringstream source, pred;
	//source << vid1;
	//pred << elabel;
	//string tripl = source.str() + " " + pred.str();
	//pair<string, int> e(tripl, vid2);
	//adjVertex.push_back(e);

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
	  pair<int, int> e2(-1 * vid1, elabel);
	  itr->second.push_back(e2);
	}
	else {
	  vector<pair<int, int> > v;
	  pair<int, int> e2(-1 * vid1, elabel);
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
  writeOut(fpout, edgeSet, c2);
  fclose(fpout);;
  fclose(fpin);
  //adjVertex.clear();
  edgeSet.clear();
  return 0;
}

void writeOut(FILE *fpout, map<int, vector<pair<int, int> > >& edgeSet, string connect_count)
{
  if(!edgeSet.empty()) {
  map<std::vector<int>, bool, ltstr> visitedEdge;
  
  // initialize each edge to be "not visited"
  /*for (int i = 0; i < adjVertex.size(); i++) {
    stringstream sink;
    sink << adjVertex[i].second;
    string tripl = adjVertex[i].first + " " + sink.str();
    visitedEdge[tripl] = false;
    }*/

  queue<int> Q;
  map<int, vector<pair<int, int> > >::iterator itr = edgeSet.begin();
  cout << flush;

  Q.push(itr->first);
  //int cc = 1;
  FILE *fcc = fopen(connect_count.c_str(), "a");
  fprintf(fcc, "\nPartition : %d\n", parti);
  parti++;
  assert(fcc);
  while(edgeSet.size() != 0) {
    int con_Count = 0;
    // If there are elements in the queue
    while (Q.size() != 0) {
      int v1 = Q.front();
      Q.pop();    
      itr = edgeSet.find(v1);
      if (itr != edgeSet.end()) {  
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
	  //string triple = source.str() + " " + pred.str() + " " + sink.str();	
	  
	  // Visited Edge
	  if (visitedEdge.find(test_v) == visitedEdge.end()) {
	    fprintf(fpout, "%d %d %d .\n", source, pred, sink);
	    visitedEdge[test_v] = true;
	    Q.push(next);
	    con_Count++;
	  }
	}
	edgeSet.erase(itr);
      }
    }	     
    visitedEdge.clear();
    fprintf(fpout, "\n");
    fprintf(fcc, "triples in connected component %d = %d\n", cc, con_Count);
    cout << flush;    
    cc++;
    itr = edgeSet.begin();
    Q.push(itr->first);    
  }
  fclose(fcc);
  }
}
