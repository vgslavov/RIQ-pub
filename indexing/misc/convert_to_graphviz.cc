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
#include <string.h>

#include "utils.h" // contains all the apis required to manipulate polys
using namespace std;

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

  char buf[4 * 1024];
  int docId;

  fprintf(fpout, "digraph myGraph {\n");
  while (1) {
    char subj[4 * 1024], pred[4 * 1024], obj[4 * 1024];
    
    if (fgets(buf, sizeof(buf), fpin) == NULL) break;
    int n = sscanf(buf, "%s %s %[^\n]s", subj, pred, obj); 
    if (n != 3) {
      cout << "Skipping line..." << endl;
    }
    obj[strlen(obj)-1] = '\0';
    fprintf(fpout, "%s -> %s [ label = \"%s\" ];\n", subj, obj, pred);
  }
  fprintf(fpout, "}");
  fclose(fpin);
  fclose(fpout);
  return 0;
}
