#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sys/types.h>
#include <dirent.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <getopt.h>

#include "utils.h"
#include "poly.h"
//#include "bloom_counter.h"
//#include "bloom_filter.h"

// tell g++ it's a C library!
extern "C" {
  #include "dablooms.h"
}

using namespace std;

extern char *__progname;
extern long int warning_num;

DabloomsType dablooms_t = DACBF;
bool verbose = false;

void usage(void);

int main(int argc, char *argv[])
{
  int ch;
  int CAPACITY = 0;
  int MAX_CAPACITY = 0;
  double ERROR_RATE = 0.0;
  bool vflag = false;
  bool use_union_cap = false;
  string metafile = "";
  string idxdir = "";

  while ((ch = getopt(argc, argv, "c:d:e:M:Vv")) != -1)
    switch(ch) {
    case 'c':
      CAPACITY = strtol(optarg, NULL, 10);
      break;
    case 'd':
      idxdir = optarg;
      break;
    case 'e':
      ERROR_RATE = strtod(optarg, NULL);
      break;
    case 'M':
      metafile = optarg;
      break;
    case 'V':
      verbose = true;
      break;
    case 'v':
      vflag = true;
      break;
    case 'h':
    case '?':
    default:
      usage();
      break;
    }
  argc -= optind;
  argv += optind;

  if (vflag) {
      cout << "v2.0, DBIS Lab, CSEE, UMKC, 2014\n";
      exit(0);
  }

  MAX_CAPACITY = CAPACITY;
  use_union_cap = true;

  ofstream ofs(metafile.c_str(), ios::app);
  if (!ofs.is_open()) {
    cout << "Can't open META file\n";
    exit(2);
  }

  cout << "capacity (# of inserts): " << CAPACITY << endl;
  cout << "use as MAX capacity: " << use_union_cap << endl;
  cout << "error rate (%): " << ERROR_RATE * 100 << endl;

  // set def. values for old code to compile
  int filterSize = 90000000;
  int numHashFunctions = 4;
  int counterSize = 8;

  vector<POLY> sig;

  string unionSigsFile = idxdir + "/" + string(UNIONSIGSDIR);
  struct dirent **namelist;
  // namelist is sorted!
  int n = scandir(unionSigsFile.c_str(), &namelist, 0, alphasort);
  if (n < 0)
    cout << "scandir error\n";
  else {
    double startTime = getgtod();

    for (int k = 0; k < n; k++) {
      if (strcmp(namelist[k]->d_name, ".") == 0 || strcmp(namelist[k]->d_name, "..") == 0) {
        free(namelist[k]);
        continue;
      }

      cout << "Union sig: " << namelist[k]->d_name;

      string filename = string(namelist[k]->d_name);
      unsigned found = filename.find_last_of(".");
      string graphid = filename.substr(0, found);
      string ext = filename.substr(found+1);

      // read the union and then compute Counting Bloom Filters
      string sigFile = idxdir + "/" + string(UNIONSIGSDIR) + "/" + string(namelist[k]->d_name);

      FILE *fp = fopen(sigFile.c_str(), "r");

      //assert(fp);
      if (!fp) {
        cerr << "Error, could not open file " << sigFile.c_str() << ": "
             << strerror(errno) << endl;
        continue;
      }

      int val;
      if (fread(&val, sizeof(int), 1, fp) != 1) {
        cerr << "No signatures in the file..." << endl;
        assert(0);
      }

      POLY e;
      sig.clear();
      for (int i = 0; i < val; i++) {
        if (fread(&e, sizeof(POLY), 1, fp) != 1) {
          cerr << "Bad signature..." << endl;
          assert(0);
        }
        sig.push_back(e);
      }
      fclose(fp);

      cout << " size: " << sig.size() << endl;

      if (use_union_cap) {
        if (sig.size() > MAX_CAPACITY) {
          CAPACITY = MAX_CAPACITY;
        } else {
          CAPACITY = sig.size();
        }
        ofs << CAPACITY << endl;
      }

      /*
      // sp895 BF
      // TODO: modify Dablooms to support BF
      //if (ext == "spo")
      if (ext == "xxx") {
        BF *bf = bf_create(filterSize, numHashFunctions);
        assert(bf);
        bf_add(bf, sig);
        cout << "Writing to file (BF) " << namelist[k]->d_name << endl;
        string fileName = idxdir + "/" + string(BLOOMCOUNTERSDIR) + "/" + string(namelist[k]->d_name);
        bf_write(bf, fileName.c_str());
        bf_destroy(bf);

        // TODO in BF
        //if (warning_num)
        //cout << "Overflow warnings: " << warning_num << endl;
      */

      // Dablooms CBF
      if (dablooms_t == DACBF) {
        counting_bloom_t *dabloom;
        //cout << "sizeof(counting_bloom_t): " << sizeof(counting_bloom_t) << endl;

        string fileName = idxdir + "/" + string(BLOOMCOUNTERSDIR) + "/" + string(namelist[k]->d_name);

        if (!(dabloom = new_counting_bloom(CAPACITY, ERROR_RATE, fileName.c_str()))) {
          cerr << "Could not create counting bloom filter\n";
          continue;
        }

        /*
        const char *sigstr[sig.size()];
        for (int i = 0; i < sig.size(); i++) {
          sigstr[i] = to_string(sig[i]).c_str();
        }
        counting_bloom_add_vec(dabloom, sigstr, sig.size());
        */

        for (int i = 0; i < sig.size(); i++) {
          string polystr = to_string(sig[i]);
          counting_bloom_add(dabloom, polystr.c_str(), polystr.length());
        }

        bitmap_flush(dabloom->bitmap);
        free_counting_bloom(dabloom);
      // Dablooms Scaling CBF
      } else if (dablooms_t == DASCLCBF) {
        scaling_bloom_t *dabloom;
        //cout << "sizeof(scaling_bloom_t): " << sizeof(scaling_bloom_t) << endl;

        string fileName = idxdir + "/" + string(BLOOMCOUNTERSDIR) + "/" + string(namelist[k]->d_name);
        if (!(dabloom = new_scaling_bloom(CAPACITY, ERROR_RATE, fileName.c_str()))) {
          cerr << "Could not create scaling bloom filter\n";
          continue;
        }

        /*
        const char *sigstr[sig.size()];
        for (int i = 0; i < sig.size(); i++) {
          sigstr[i] = to_string(sig[i]).c_str();
        }
        counting_bloom_add_vec(dabloom, sigstr, sig.size());
        */

        for (int i = 0; i < sig.size(); i++) {
          string polystr = to_string(sig[i]);
          scaling_bloom_add(dabloom, polystr.c_str(), polystr.length(), i);
        }

        bitmap_flush(dabloom->bitmap);
        free_scaling_bloom(dabloom);
      }

      /*
      } else {
      // sp895 CBF
        BLOOM *bloom = bloom_create(filterSize, numHashFunctions, counterSize);
        assert(bloom);
        bloom_add(bloom, sig);
        cout << "Writing to file (BFC) " << namelist[k]->d_name << endl;
        string fileName = idxdir + "/" + string(BLOOMCOUNTERSDIR) + "/" + string(namelist[k]->d_name);
        bloom_write(bloom, fileName.c_str());
        bloom_destroy(bloom);
        if (warning_num)
          cout << "Overflow warnings: " << warning_num << endl;
      }
      */

      free(namelist[k]);
    }
    double endTime = getgtod();
    cout << "Total CBF time: " << endTime - startTime << endl;
    free(namelist);
    ofs.close();
  }
  return 0;
}

void usage(void)
{
  cout << "Usage: " << __progname << " [-h] [options...]\n\n";

  cout << "OPTIONS:\n"
       << "   -d <index dir>\n"
       << "   -c <max capacity (inserts)>\n"
       << "   -e <false positive error rate>\n"
       << "   -M <meta file>\n"
       << "   -V verbose\n"
       << "   -v print version\n"
       << "   -h print help/usage\n";

  exit(0);
}
