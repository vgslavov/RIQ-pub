// Author: Praveen Rao
// UMKC, 2008

// for help on STL refer to http://www.sgi.com/tech/stl/
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <assert.h>
#include <vector>
#include <list>
#include <string.h>
#include <algorithm>

#include "utils.h" // contains all the apis required to manipulate polys
using namespace std;
void writeSig(FILE *fpout, vector<POLY>& sig, int);
void writeDegrees(FILE *fpout, vector<int>&, int);

int main(int argc, char *argv[])
{
	cout << "Author: Praveen Rao, UMKC 2008.\n";
    if (argc < 4) {
        cout << "Usage: " << argv[0]
             << " <input file>"
             << " <output file>"
             << " <irr poly file>"
             << " [irr poly for hashing]"
             << " [rank: 0|1] "
             << endl;
        exit(1);
    }
    POLY irrPoly = argv[4] ? atoi(argv[4]) : 7701;

    // Rank the vertices by their degrees in descending order
    bool computeRank = argv[5] ? atoi(argv[5]) : 0;
    vector<int> degrees;
    
    // Open files
    FILE *fpin = fopen(argv[1], "r");
    assert(fpin);
  
    FILE *fpout = fopen(argv[2], "w");
    assert(fpout);

    string degName = string(argv[2]) + ".rank";
    FILE *fpdeg = fopen(degName.c_str(), "w");
    assert(fpdeg);

    // A polynomial is stored as a STL vector of POLY data type defined in poly.h
    // If you read irreducible polynomials from a file, then use the 
    // data type POLY instead of int. use %u instead of %d for fscanf.
 
    // This creates a polynomial 0x0000000100000002  (in hex)
    // You can also initialize sig to 0x1 by sig.push_back(0x1);
    vector<POLY> sig;
    vector<string> vertex;

    // Read the list of irreducible polynomials...
    vector<POLY> irrPolyList;

    int count = 0;
    int sigSize = 0;

    FILE *fpirr = fopen(argv[3], "r");
    assert(fpirr);
    POLY numPoly = 0;
    while (numPoly < irrPoly) {
        POLY val;
        if (fscanf(fpirr, "%u", &val) == EOF) break;
   
        irrPolyList.push_back(val);
        numPoly++;
    }
    fclose(fpirr);
    cout << "num polys read: " << irrPolyList.size() << endl;

#ifdef false
    // Make sure there are enough polys...
    assert(irrPoly <= irrPolyList.size());
#endif
    char buf[128];
    int docId;

    int numEdges, numVertices;
    int totalNumEdges = 0, totalNumVertices = 0;

    double beginTime = getgtod();
    while (1) {

        if (fgets(buf, sizeof(buf), fpin) == NULL) break;
        char type; 
        int vid, vid1, vid2;
        char vlabel[128], elabel[128];

        sscanf(buf, "%c", &type);	 

        switch(type) {
        case '#':
            cout << "COMMENT line\n";
            break;
        case 't':
            {
                if (sig.size() > 0) {
                    writeSig(fpout, sig, docId);
                    if (computeRank) writeDegrees(fpdeg, degrees, docId);
                    sigSize += sig.size() * sizeof(POLY);
		    //cout << "SIG size: " << sig.size() << endl;
                    count++;
		    totalNumVertices += numVertices;
                    totalNumEdges += numEdges;
                }
                char junk;
                sscanf(buf, "%c %c %d", &type, &junk, &docId);
                //cout << "BEGIN: " << docId << endl;
                numEdges = 0;
                numVertices = 0;
                sig.clear();
#ifdef false
                sig.push_back(0x1);
#endif
                vertex.clear();
                degrees.clear();
            }
            break;
        case 'u':
            {
                sscanf(buf, "%c %d %d %s", &type, &vid1, &vid2, elabel); 
                string elabelpair;
                if (vertex[vid1] < vertex[vid2]) {
                    elabelpair = vertex[vid1] + "." + vertex[vid2];
                }
                else {
                    elabelpair = vertex[vid2] + "." + vertex[vid1];
                }

 		// If a valid edge label is present, then consider it during signature
		// generation
		if (strcmp(elabel, "e") != 0) {
		  elabelpair += ("." + string(elabel));
		}
                //int hashVal = hashString(elabelpair.c_str());
                int hashVal = hashIrrPoly(elabelpair.c_str(), irrPoly);
                //cout << "LABELPAIR: " << elabelpair.size() << " HASH id: " 
                //     << hashVal << endl;
#ifdef false
                multiplyPoly(sig, irrPolyList[hashVal]);
#else
                sig.push_back(hashVal);
#endif
                // Count the number of edges
                numEdges++;
                // update the vector of degrees
                degrees[vid1]++;
                degrees[vid2]++;
            }
            break;
        case 'v':
            sscanf(buf, "%c %d %s", &type, &vid, vlabel);
            vertex.push_back(string(vlabel));
            // Count the number of vertices
            numVertices++;
            // add an entry for degree of vertices
            degrees.push_back(0);
            break;
        }
    }
  
    // Last signature
    if (sig.size() > 0) {
        writeSig(fpout, sig, docId);
        if (computeRank) writeDegrees(fpdeg, degrees, docId);
        sigSize += sig.size() * sizeof(POLY);
	//cout << "SIG size: " << sig.size() << endl;
	//cout << "Num vertices: " << numVertices << endl; 
        //cout << "Num edges: " << numEdges << endl;
        totalNumVertices += numVertices;
        totalNumEdges += numEdges;
        count++;
    }
    double endTime = getgtod();

    cout << "Average signature size: " << sigSize * 1.0/count << endl;
    cout << "Average vertex size: " << totalNumVertices * 1.0/count << endl;
    cout << "Average edge size: " << totalNumEdges * 1.0/count << endl;
    cout << "Total time: " << endTime - beginTime << endl;
    fclose(fpin);
    fclose(fpout);
    fclose(fpdeg);
    return 0;
}

void writeDegrees(FILE *fpout, vector<int>& degrees, int docId)
{
    std::list<int> tempList;
    for (int i = 0; i < degrees.size(); i++) {
        tempList.push_back(degrees[i]);
    }
    tempList.sort();

    fprintf(fpout, "%d %d", docId, (int) degrees.size());
    
    for (list<int>::const_iterator itr = tempList.begin();
         itr != tempList.end(); itr++) {
        fprintf(fpout, " %d", *itr);
    }
    fprintf(fpout, "\n");
}

void writeSig(FILE *fpout, vector<POLY>& sig, int docId)
{

#ifdef false
    
#else
    sort(sig.begin(), sig.end());
    //for (int i = 0; i < (int) sig.size(); i++) {
    //    cout << sig[i] << " " << endl;
    //}
    //cout << endl;
#endif
    
    // Store the signature in the output file
    // n = 1 for our case
    int numSigs = 1;
    if (fwrite(&numSigs, sizeof(numSigs), 1, fpout) != 1) { 
        assert(0);
    }
    // Not number of bytes 
    int sigSize = sig.size();
    
    if (fwrite(&sigSize, sizeof(sigSize), 1, fpout) != 1) { 
        assert(0);
    }

    for (int i = 0; i < sig.size(); i++) {
        POLY val = sig[i];
        if (fwrite(&val, sizeof(val), 1, fpout) != 1) {
            assert(0);
        }
    }

    // Write the docid also
    if (fwrite(&docId, sizeof(docId), 1, fpout) != 1) {
        assert(0);
    }
}
