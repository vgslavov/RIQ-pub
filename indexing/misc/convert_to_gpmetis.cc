// for help on STL refer to http://www.sgi.com/tech/stl/
#include <iostream>
#include <stdio.h>
#include <assert.h>
#include <vector>
#include <map>
#include <queue>
#include <stdlib.h>
#include <sstream>
#include <climits>

#include "utils.h" // contains all the apis required to manipulate polys
using namespace std;
void writeOut(FILE *fpout, vector<string>&, vector<pair<int, int> >&, 
	      map<int, vector<pair<int, string> > > &);

int finalDocid = 0;


int main(int argc, char *argv[])
{

  cout << "Author: DBIS Lab, UMKC 2012.\n";
  if (argc < 2) {
    cout << "Usage: " << argv[0]
         << " <input file>"
         << " <output file>"
         << endl;
    exit(1);
  }

  // Open files
  FILE *fpin = fopen(argv[1], "r");
  assert(fpin);
  
  FILE *fpout = fopen(argv[2], "w");
  assert(fpout);

  vector<string> vertex;

  vector<pair<int, int> > adjVertex;

  map<int, vector<pair<int, string> > > edgeSet;

  char buf[4 * 1024];
  int docId;
  while (1) {

    if (fgets(buf, sizeof(buf), fpin) == NULL) break;
    char type; 
    int vid, vid1, vid2;
    int edgeCount;
    char vlabel[4 * 1024], elabel[4 * 1024];

    sscanf(buf, "%c", &type);	 

    switch(type) {
      case '#':
        cout << "COMMENT line\n";
        break;
      case 't':
        {
          writeOut(fpout, vertex, adjVertex, edgeSet);
          cout << "BEGIN new: \n";
          char junk;
          sscanf(buf, "%c %c %d", &type, &junk, &docId);
          //fprintf(fpout, "%c%d\n", junk, docId);
          printf("%d\n", docId);
          vertex.clear(); 
          adjVertex.clear();
	  edgeSet.clear();
        }
        break;
      case 'u':
        {
          sscanf(buf, "%c %d %d %[^\n]s", &type, &vid1, &vid2, elabel); 
          pair<int, int> e(vid1, vid2);
          adjVertex.push_back(e);

	  //pair<int, int> e2(vid2, vid1);
          //edgeLabel[e2] = string(elabel);

	  // FOR DINESH
	  // Direction of the edge is take into account -- RDF graphs
	  map<int, vector<pair<int, string> > >::iterator itr;

	  // Need to store both (2,4) and (4,2)
	  itr = edgeSet.find(vid1);
	  if (itr != edgeSet.end()) {
	    //pair<int, string> e2(vid2, string(elabel));
	    pair<int, string> e2(vid2, string(""));
	    itr->second.push_back(e2);
	  }
	  else {
	    vector<pair<int, string> > v;
	    //pair<int, string> e2(vid2, string(elabel));
	    pair<int, string> e2(vid2, string(""));
	    v.push_back(e2);
	    edgeSet[vid1] = v;
	  }
	  
	  itr = edgeSet.find(vid2);
	  if (itr != edgeSet.end()) {
	    pair<int, string> e2(vid1, string(""));
	    itr->second.push_back(e2);
	  }
	  else {
	    vector<pair<int, string> > v;
	    pair<int, string> e2(vid1, string(""));
	    v.push_back(e2);
	    edgeSet[vid2] = v;
	  }
        }
        break;
      case 'v':
        {
          sscanf(buf, "%c %d %[^\n]s", &type, &vid, vlabel);
          //cout << "Vertex label: " << vlabel << endl;
          vertex.push_back(string(vlabel));
        }
        break;
    }
  }
 
  writeOut(fpout, vertex, adjVertex, edgeSet); 
  fclose(fpin);
  fclose(fpout);
  return 0;
}

void writeOut(FILE *fpout, vector<string>& vertex, vector<pair<int, int> >& adjVertex, 
	      map<int, vector<pair<int, string> > >& edgeSet)
{
  if (vertex.size() > 0) {
    // write the # of vertices and edges
    fprintf(fpout, "%d %d\n", (int) vertex.size(), (int) adjVertex.size());

    // Traverse through edgeSet
    for (map<int, vector<pair<int, string> > >::const_iterator itr = edgeSet.begin();
	 itr != edgeSet.end(); itr++) {
      for (int i = 0; i < itr->second.size(); i++) {
	// Add 1 because GIS vertices start from 0 and GPMETIS vertices start from 1
	fprintf(fpout, "%d ", itr->second[i].first + 1);
      }
      fprintf(fpout, "\n");
    }
  }
}
