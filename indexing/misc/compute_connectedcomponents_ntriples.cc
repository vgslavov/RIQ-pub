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
	    pair<int, string> e2(vid2, string(elabel));
	    itr->second.push_back(e2);
	  }
	  else {
	    vector<pair<int, string> > v;
	    pair<int, string> e2(vid2, string(elabel));
	    v.push_back(e2);
	    edgeSet[vid1] = v;
	  }
	  
	  itr = edgeSet.find(vid2);
	  if (itr != edgeSet.end()) {
	    pair<int, string> e2(vid1 == 0 ? INT_MIN : -1 * vid1, string(""));
	    itr->second.push_back(e2);
	  }
	  else {
	    vector<pair<int, string> > v;
	    pair<int, string> e2(vid1 == 0 ? INT_MIN : -1 * vid1, string(""));
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
  // Output the vertex labels first

  map<string, bool> visitedEdge;
  
  if (vertex.size() > 0) {

    //pair<int, int> e(adjVertex[0].first, adjVertex[0].second);

    queue<int> Q;
    map<int, vector<pair<int, string> > >::iterator itr = edgeSet.begin();

    // Push a vertex into the queue
    Q.push(itr->first);

    map<int, string> myVertexSet;
    vector<pair<int, int> > myEdgeSet;
    vector<string> myEdgeLabel;
    map<int, int> vertexIdMapping;

    while (edgeSet.size() != 0) {
      int graphSize = 0;
      myVertexSet.clear();
      myEdgeSet.clear();
      myEdgeLabel.clear();

      //cout << "In loop again" << endl;

      // If there are elements in the queue
      while (Q.size() != 0) {
	int v1 = Q.front();
	//cout << "#" << v1;
        
	Q.pop();
	itr = edgeSet.find(v1);

	if (itr != edgeSet.end()) {
	  for (int i = 0; i < itr->second.size(); i++) {
	    
	    //cout << "-" << itr->second[i];
            //pair<int, int> t(v1, itr->second[i]);

	    stringstream source, sink;
	    
	    // FOR DINESH
	    pair<int, int> t(0,0);
	    if (itr->second[i].first >= 0) {
	      source << v1;
	      sink << itr->second[i].first;
	      t.first = v1;
	      t.second = itr->second[i].first;
	    }
	    else {
	      Q.push(-1 * itr->second[i].first);
	      continue;
	    }

	    //pair<int, int> t(v1, itr->second[i].first);
	    
	    string triple = source.str() + "." + sink.str() + "." + itr->second[i].second;

	    // Visited Edge
	    if (visitedEdge.find(triple) == visitedEdge.end()) {
	      //cout <<  t.second << endl;
	      fprintf(fpout, "%s %s %s .\n", vertex[t.first].c_str(), itr->second[i].second.c_str(),
		      vertex[t.second].c_str()); 
	      visitedEdge[triple] = true;
	      
	      Q.push(itr->second[i].first);
	      graphSize++;
	    }
	  }
	  edgeSet.erase(itr);
	}
      }

      fprintf(fpout, "%s", "\n");

      // clean visitedEdge
      visitedEdge.clear();

      itr = edgeSet.begin();
      Q.push(itr->first);
    }
  }
}
