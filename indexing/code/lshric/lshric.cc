/* $Id: lshric.cc,v 1.4 2012/08/30 21:56:19 vsfgd Exp $ */

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <map>
#include <string>
#include <vector>

#include <assert.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>

#include "bloom.h"
#include "utils.h"
//#include "ris.pb.h"

using namespace std;

#define _DEBUG_

static char rcsid[] = "$Id: lshric.cc,v 1.4 2012/08/30 21:56:19 vsfgd Exp $";
extern char *__progname;

bool verbose = false;
// a.k.a. bands
int lshk = 4;
// a.k.a. rows
int lshl = 10;
int lshseed = 1122941;
int nflag = 0;
vector<POLY> irrnums;
vector<int> hasha;
vector<int> hashb;

// sig, (<freq,weight> | <avg,avg,...>)
typedef map<vector<POLY>, vector<double>, CompareSig> mapType;
typedef vector<mapType> vecomap;
vecomap allT;

typedef map<string, int> hashType;
hashType hashvalues;

void addhashv(vector<string>);
void calcfreq(vector<vector <POLY> >);
int getdir(string, vector<string>&);
int lshsig(vecomap, int listnum = 0, int col = 0);
int printlist(vecomap, int, int, bool hdr = true);
bool processquery(string);
void readmeta(string, string);
//void readlshindex(const ris::LSHindex&);
void readindex(string, vector<vector <POLY> > &, int nsigs = -1);
void readsig(string, vector<vector <POLY> > &, int nsigs = -1);
bool sig2str(vector<POLY>, string &);
void tokenize(const string &, vector<string> &, const string &);
void usage(void);

int
main(int argc, char *argv[])
{
    int ch, nsigs, bloomfuncs, bloomsize;
    int kflag, lflag, Bflag, bflag, cflag, dflag, Fflag, Hflag, jflag, Lflag, qflag, sflag, vflag, rflag;
    //int logfd;
    double beginTime, endTime;
    static char *hashfile;
    static char *irrpolyfile;
    static char *lshfile;
    static char *logfile;
    struct stat statbuf;
    string datafile;
    string metafile;
    string queryfile;
    BLOOM *bloom;
    //ris::LSHindex lsh_index;

    string sigdir;
    vector<string> sigfiles;
    vector<vector<POLY> > sigList;

    nsigs = -1;
    kflag = lflag = Bflag = bflag = cflag = dflag = Fflag = Hflag = jflag = Lflag = qflag = sflag = vflag = rflag = 0;

    while ((ch = getopt(argc, argv, "B:b:cd:F:f:H:hj:k:L:l:n:q:rs:Vv")) != -1)
        switch(ch) {
        case 'B':
            Bflag = 1;
            bloomsize = strtol(optarg, NULL, 10);
            break;
        case 'b':
            bflag = 1;
            bloomfuncs = strtol(optarg, NULL, 10);
            break;
        case 'c':
            cflag = 1;
            break;
        case 'd':
            dflag = 1;
            lshseed = strtol(optarg, NULL, 10);
            break;
        case 'F':
            Fflag = 1;
            hashfile = optarg;
            break;
        case 'H':
            Hflag = 1;
            lshfile = optarg;
            break;
        case 'j':
            jflag = 1;
            irrpolyfile = optarg;
            break;
        case 'k':
            kflag = 1;
            lshk = strtol(optarg, NULL, 10);
            break;
        case 'L':
            Lflag = 1;
            logfile = optarg;
            break;
        case 'l':
            lflag = 1;
            lshl = strtol(optarg, NULL, 10);
            break;
                case 'n':
            nflag = 1;
                        nsigs = strtol(optarg, NULL, 10);
                        break;
                case 'q':
                        qflag = 1;
                        queryfile = optarg;
                        break;
        case 'r':
            rflag = 1;
            break;
        case 's':
            sflag = 1;
            datafile = optarg;
            break;
        case 'V':
            verbose = true;
            break;
        case 'v':
            vflag = 1;
            break;
        case 'h':
        case '?':
        default:
            usage();
            break;
        }
    argc -= optind;
    argv += optind;

    if (vflag == 1) {
        cout << "rcsid: " << rcsid << "\n";
        exit(0);
    }

    // create or read LSH index
    if (cflag == 0 && rflag == 0) usage();

    // create LSH index reqs
    if (cflag == 1 && (sflag == 0 || Bflag == 0 || bflag == 0)) usage();

    // read LSH index reqs
    if (rflag == 1 && qflag == 0) usage();
        
    // common reqs
    if (Fflag == 0 || Hflag == 0 || jflag == 0 || kflag == 0 || lflag == 0) usage();

    // protobuf
    /*
    if (rflag == 1) {
        fstream input(lshfile, ios::in | ios::binary);
        if (!lsh_index.ParseFromIstream(&input)) {
            cerr << "Failed to parse lsh index in rflag." << endl;
            cout<< "Failed"<<endl;
            return -1;
        }
        cout<<"succ"<<endl;
                cout<<"Query file :"<<queryfile<<endl;
            readlshindex(lsh_index);
            //processquery(queryfile);
                

        // Optional:  delete all global objects allocated by libprotobuf
        google::protobuf::ShutdownProtobufLibrary();

        return 0;
    }

    if (Hflag == 1) {
        fstream input(lshfile, ios::in | ios::binary);
        if (!input) {
            cout << lshfile << ": File not found. Creating a new file." << endl;
        // modify existing one?
        } else if (!lsh_index.ParseFromIstream(&input)) {
            cerr << "Failed to parse lsh index in Hflag." << endl;
            return -1;
        }
    }
    */

    FILE *hashfp = NULL;
    string acc;
    if (Fflag == 1) {
        acc = "w+";
        if ((hashfp = fopen(hashfile, acc.c_str())) == NULL) {
            cerr << "can't write hash file " << hashfile << "\n";
            exit(1);
        }

        int random_integer_a;
        int random_integer_b;
        int lowest_a = 1, highest_a = -9;
        int lowest_b = 0, highest_b = -9;
        highest_a = highest_b = lshseed;
        int range_a = (highest_a - lowest_a) + 1;
        int range_b = (highest_b - lowest_b) + 1;
        hasha.clear();
        hashb.clear();
        srand(lshseed);
        int totalfuncs = lshl * lshk;
        for (int i = 0; i < totalfuncs; i++) {
            // TODO: verify randomness
            random_integer_a  = lowest_a + int((double)range_a*rand()/(RAND_MAX + 1.0));
            random_integer_b  = lowest_b + int((double)range_b*rand()/(RAND_MAX + 1.0));
            hasha.push_back(random_integer_a);
            hashb.push_back(random_integer_b);
        }
        cout << "writing " << hashfile << "...\n";
        for (int i = 0; i < (int)hasha.size(); i++) {
            fprintf(hashfp, "%d\n", hasha[i]);
        }

        for (int i = 0; i < (int)hashb.size(); i++) {
            fprintf(hashfp, "%d\n", hashb[i]);
        }
        fclose(hashfp);
    }

    if (jflag == 1) {
        if (stat(irrpolyfile, &statbuf) != 0) {
            cout << "'" << irrpolyfile << "' does not exist" << "\n";
            exit(2);
        }

        irrnums.clear();
        std::ifstream polystream;
        polystream.open(irrpolyfile);
        POLY num;
        while (polystream >> num) {
            irrnums.push_back(num);
        }
        polystream.close();
    }

    // TODO: logging
    if (Lflag == 1) {
        // overwrite existing log file
        /*
        logfd = open(logfile, O_CREAT|O_WRONLY|O_TRUNC, 0644);
        if (logfd < 0) {
            cout << "can't open log file " << logfile << "\n";
            exit(0);
        }
        lseek(logfd, 0, SEEK_END);
        errfd = logfd;
        */
    }

    // set random local seed
    char host[256];
    struct timeval tv;
    unsigned int loseed;
    gethostname(host, 256);
    gettimeofday(&tv, NULL);
    loseed = ((long)tv.tv_sec + (long)tv.tv_usec) / (long)getpid();
    srandom(loseed);

    cout << "rcsid: " << rcsid << "\n";
    cout << "host: " << host << "\n";
    cout << "pid: " << getpid() << "\n";
    cout << "lsh seed: " << lshseed << "\n";
    cout << "local seed: " << loseed << "\n";
    cout << "lshk: " << lshk << "\n";
    cout << "lshl: " << lshl << "\n";

    /*
    // TODO: dir of sigs
    if (sflag == 1) {
        sigList.clear();
        beginTime = getgtod();
        getdir(sigdir, sigfiles);

        cout << "reading signatures from files...\n";
        for (unsigned int i = 0; i < sigfiles.size(); i++) {
            readsig(sigfiles[i], sigList, nsigs);
        }

        endTime = getgtod();
        cout << "read time: " << endTime - beginTime << "\n";
    }
    */

    if (cflag == 1) {
        sigList.clear();
        beginTime = getgtod();

        // read all sigs in memory
        readindex(datafile, sigList, nsigs);

        endTime = getgtod();
        cout << "read time: " << endTime - beginTime << "\n";
        cout << "adding to map...\n";
        beginTime = getgtod();
        cout << "sigList.size() (all): " << sigList.size() << "\n";

        // put sigs in map
        calcfreq(sigList);

        endTime = getgtod();
        cout << "map time: " << endTime - beginTime << "\n";

        if (verbose == true) {
            printlist(allT, 0, -1);
        }

        // apply LSH on sigs and store hash values in map
        lshsig(allT);

        cout << "hashvalues.size(): " << hashvalues.size() << "\n";

        bloom = bloom_create(bloomsize, bloomfuncs);
        for (hashType::iterator itr = hashvalues.begin(); itr != hashvalues.end(); itr++) {
            bloom_add(bloom, itr->first.c_str());

            /*
            // protobuf
            ris::HashValue* hash_value = lsh_index.add_hashvalue();
            cout << "hash: " << itr->first << " freq: " << itr->second << "\n";
            hash_value->set_hash(itr->first);
            hash_value->set_freq(itr->second);
            */
        }

        bloom_write(bloom, lshfile);
        bloom_destroy(bloom);

        /*
        // protobuf
        fstream output(lshfile, ios::out | ios::trunc | ios::binary);
        if (!lsh_index.SerializeToOstream(&output)) {
            cerr << "Failed to write lsh index." << endl;
            return -1;
        }

        // Optional:  delete all global objects allocated by libprotobuf
        google::protobuf::ShutdownProtobufLibrary();
        */
    } else if (rflag == 1) {
        bloom = bloom_read(lshfile);

        sigList.clear();
        beginTime = getgtod();

        readindex(queryfile, sigList, nsigs);

        endTime = getgtod();
        cout << "read time: " << endTime - beginTime << "\n";
        cout << "adding to map...\n";
        beginTime = getgtod();
        cout << "sigList.size() (all): " << sigList.size() << "\n";

        // put sigs in map
        calcfreq(sigList);

        endTime = getgtod();
        cout << "map time: " << endTime - beginTime << "\n";

        if (verbose == true) {
            printlist(allT, 0, -1);
        }

        // apply LSH on sigs and store hash values in map
        lshsig(allT);

        cout << "hashvalues.size(): " << hashvalues.size() << "\n";

        bool bcheck = false;
        bool finalresult = true;
        int n = 0;
        for (hashType::iterator itr = hashvalues.begin(); itr != hashvalues.end(); itr++) {
            bcheck = bloom_check(bloom, itr->first.c_str());
            cout << "hashvalue: " << itr->first << " bloom check: " << bcheck << endl;
            // all bchecks have to be true
            if (n == 0 && bcheck == false) {
                finalresult = false;
                n = 1;
            }
        }

        cout << "queryresult: " << finalresult << endl;
        //string falsehash = "klajsdfkljewo";
        //bcheck = bloom_check(bloom, falsehash.c_str());
        //cout << "hashvalue: " << falsehash << " bloom check: " << bcheck << endl;
    }

    return 0;
}

int
lshsig(vecomap teamvecomap, int listnum, int col)
{
    vector<vector<string> > matrix;
    vector<string> minhash;
    vector<string> buckets;
    vector<POLY> sig;
    vector<POLY> lshsig;
    string ID;
    int n;
    double freq, beginTime, endTime;

    matrix.clear();
    minhash.clear();
    sig.clear();
    lshsig.clear();
    n = 0;
    for (mapType::iterator itr = teamvecomap[listnum].begin(); itr != teamvecomap[listnum].end(); itr++) {
        sig = itr->first;
        freq = itr->second[0];

        beginTime = getgtod();    
        lsh *myLSH = new lsh(sig.size(), lshl, lshk, lshseed, irrnums, hasha, hashb, col);
        minhash = myLSH->getHashCode(sig);
        endTime = getgtod();    
        cout << "sig" << n << ": lsh time: " << endTime - beginTime << "\n";

        //sort(minhash.begin(), minhash.end());

        addhashv(minhash);

        //cout << "minhash.size(): " << minhash.size() << "\n";
        if (verbose == true) {
            cout << "sig" << n << ": ";
            cout << "minhash IDs: " << "\n";
            for (int i = 0; i < (int)minhash.size(); i++) {
                cout << minhash[i] << "\n";
            }
        }
        // needed?
        minhash.clear();
        delete myLSH;
        ++n;
    }
    return 0;
}

/*
void
readlshindex(const ris::LSHindex& lsh_index)
{
    cout << "reading LSH index...\n";
    for (int i = 0; i < lsh_index.hashvalue_size(); i++) {
        const ris::HashValue& hash_value = lsh_index.hashvalue(i);

        cout << "Hash Value: " << hash_value.hash() << endl;
        cout << "Freq: " << hash_value.freq() << endl;
    }
}
*/

void
readmeta(string datafile, string metafile)
{
    FILE *datafp;

    /*
    ifstream meta;
    meta.open(metafile);
    if (!meta.is_open()) {
    cout << " Can't open meta file\n";
    exit(2);
    }
    */

    datafp = fopen(datafile.c_str(), "r");
    if (datafp == NULL) {
        cout << "Can't open data file\n";
        exit(2);
    }

    while (1) {
    //getline
    }
}

// TODO:
// work w/ 0-sized sigs
// process nsigs
void
readsig(string sigfile, vector<vector <POLY> > &sigList, int nsigs)
{
    vector<string> tokens;
    string sigstr;
    double startTime, finishTime;
    FILE *sigfp;

    startTime = getgtod();
    // open signatures
    //cout << "sigfile: " << sigfile.c_str() << "\n";
    sigfp = fopen(sigfile.c_str(), "r");
    // change to if?
    assert(sigfp);

    // DONT use readData to retrieve signatures from input files...
    // since the size filed uses POLY as a basic unit and not byte...
    // Read numSigs <it should be 1> for data signatures...
    int numSigs;
    if (fread(&numSigs, sizeof(int), 1, sigfp) != 1) {
        //cout << "numSigs: " << numSigs << "\n";
    }
    //cout << "NUM sigs: " << numSigs;
    assert(numSigs == 1);

    int size;

    if (fread(&size, sizeof(int), 1, sigfp) != 1) {
        assert(0);
        cout << "\ninvalid signature\n";
    }
    //cout << ", Signature size: " << size * sizeof(POLY) << " bytes\n";

    vector<POLY> sig;
    sig.clear();
    POLY e;
    //cout << "Document signature (sorted): ";
    for (int i = 0; i < size; i++) {
        if (fread(&e, sizeof(POLY), 1, sigfp) != 1) {
            assert(0);
        }
        sig.push_back(e);
    }
    sort(sig.begin(), sig.end());
    //sig2str(sig, sigstr);
    //cout << sigstr << "\n";
    sigList.push_back(sig);

    cout << "readsig: Size of sig list: " << sigList.size() << "\n";
    finishTime = getgtod();
    fclose(sigfp);
}

// copied from psi.C
void
readindex(string datafile, vector<vector <POLY> > &sigList, int nsigs)
{
    vector<string> tokens;
    string sigstr;
    double startTime, finishTime;
    int n;
    FILE *dfp;

    startTime = getgtod();
    //cout << "datafile: " << datafile.c_str() << "\n";
    dfp = fopen(datafile.c_str(), "r");
    assert(dfp);

    n = 0;
    while (1) {
        // only process nsigs
        if (nflag == 1 && n == nsigs) break;

        // DON'T use readData to retrieve signatures from input files...
        // since the size filed uses POLY as a basic unit and not byte...
        // Format is <numSigs=1><sigSize><sig><graphId>... n times...
        int numSigs;
        // read "1"
        if (fread(&numSigs, sizeof(numSigs), 1, dfp) != 1) {
            cout << "numSigs: " << numSigs << "\n";
            break;
        }
        //cout << "NUM sigs: " << numSigs << "\n";
        assert(numSigs > 0);

        for (int t = 0; t < numSigs; t++) {
            POLY *buf;
            int size;
            // read sig size
            if (fread(&size, sizeof(int), 1, dfp) != 1) {
                assert(0);
            }
            //cout << "Signature size: " << size * sizeof(POLY) << " bytes\n";
                    
            buf = new POLY[size];
            assert(buf);
            // read sig
            if (fread(buf, sizeof(POLY), size, dfp) != (size_t) size) {
                assert(0);
            }
                        
            vector<POLY> sig;
            sig.clear();
            //cout << "Data signature (sorted): ";
            for (int i = 0; i < size; i++) {
            // XXX: discard "1"s
            //if (buf[i] == 1) continue;
            sig.push_back(buf[i]);
            }
            // it's already sorted?
            //sort(sig.begin(), sig.end());
            if (verbose == true) {
                sig2str(sig, sigstr);
                cout << sigstr << "\n";
            }

            sigList.push_back(sig);

            int gid;
            // read graph id
            if (fread(&gid, sizeof(int), 1, dfp) != 1) {
                assert(0);
            }
            //cout << "Graph ID: " << gid << "\n";

            // free the allocated memory
            delete[] buf;
        }
        ++n;
    }

    fclose(dfp);
    cout << "readindex: Size of sig ist: " << sigList.size() << "\n";
    finishTime = getgtod();
}

void
addhashv(vector<string> minhash)
{
    for (int i = 0; i < (int)minhash.size(); i++) {
        hashType::iterator itr = hashvalues.find(minhash[i]);
        if (itr != hashvalues.end()) {
            itr->second += 1;
        } else {
            hashvalues[minhash[i]] = 1;
        }
    }
}

void
calcfreq(vector<vector<POLY> > sigList)
{
    mapType uniqueSigList;
    int i = 0;

    for (; i < (int)sigList.size(); i++) {
        mapType::iterator itr = uniqueSigList.find(sigList[i]);
        if (itr != uniqueSigList.end()) {
            // do not increment weight when sig has duplicates!
            // increment only freq
            itr->second[0] += 1;
        } else {
            // set freq
            uniqueSigList[sigList[i]].push_back(1);
            // set weight
            uniqueSigList[sigList[i]].push_back(1);
        }
    }
    allT.push_back(uniqueSigList);
    cout << "calcfreq: setsize: " << allT[0].size() << "\n";
}

// based on:
// http://www.linuxquestions.org/questions/programming-9/c-list-files-in-directory-379323/
// verified
int
getdir(string dir, vector<string> &files)
{
    DIR *dp;
    struct dirent *dirp;

    if ((dp = opendir(dir.c_str())) == NULL) {
        cout << "Can't read dir\n";
        exit(2);
        //return errno;
    }
    while ((dirp = readdir(dp)) != NULL) {
        if (strcmp(dirp->d_name, ".") == 0 || strcmp(dirp->d_name, "..") == 0) continue;
        // separate by / in case dir arg doesn't have a trailing slash
        files.push_back(dir + "/" + string(dirp->d_name));
    }
    closedir(dp);
    return 0;
}

bool
sig2str(vector<POLY> sig, string &buf)
{
    stringstream tmp;

    if (sig.size() <= 0) return false;

    buf.clear();
    for (int i = 0; i < (int)sig.size(); i++) {
        tmp << sig[i] << "-";
    }

    buf = tmp.str();

    return true;
}

// copied from http://oopweb.com/CPP/Documents/CPPHOWTO/Volume/C++Programming-HOWTO-7.html
void
tokenize(const string& str, vector<string>& tokens, const string &delimiters = " ")
{
    // Skip delimiters at beginning.
    string::size_type lastPos = str.find_first_not_of(delimiters, 0);
    // Find first "non-delimiter".
    string::size_type pos = str.find_first_of(delimiters, lastPos);

    while (string::npos != pos || string::npos != lastPos) {
        // Found a token, add it to the vector.
        tokens.push_back(str.substr(lastPos, pos - lastPos));
        // Skip delimiters.  Note the "not_of"
        lastPos = str.find_first_not_of(delimiters, pos);
        // Find next "non-delimiter"
        pos = str.find_first_of(delimiters, lastPos);
    }
}

int
printlist(vecomap teamvecomap, int listnum, int seq, bool hdr)
{
    int n, totalsigs;
    double freq;
    string sigstr;
    vector<POLY> sig;
    sig.clear();

    n = totalsigs = 0;

    cout << "list T_" << listnum << " len: " << teamvecomap[listnum].size() << "\n";

    cout << "hdrB: sig freq size\n";
    for (mapType::iterator itr = teamvecomap[listnum].begin(); itr != teamvecomap[listnum].end(); itr++) {
        sig = itr->first;
        freq = itr->second[0];
        ++totalsigs;
        sig2str(sig, sigstr);
        cout << "sig" << n << ": " << sigstr;
        cout << " " << freq << "\n";
        cout << " " << sig.size() * sizeof(POLY) << "\n";
        ++n;
    }
    cout << "hdrE: sig freq size\n";
    cout << " setsize: " << teamvecomap[listnum].size() << "\n";

    return totalsigs;
}

bool
processquery(string queryfile)
{
    FILE *qfp;
    qfp = fopen(queryfile.c_str(), "r");
    assert(qfp);
    vector<POLY> qsig;
    qsig.clear();
    
       // Format is <numSigs=1><sigSize><sig><graphId=0>
    int numSigs;
        if(fread(&numSigs, sizeof(numSigs), 1, qfp) != 1){
        //      cout<<"numSigs in the query: "<<numSigs << "\n";
        assert(0);
    }
        int qsize;
    if(fread(&qsize, sizeof(qsize), 1, qfp) != 1){
            assert(0);
    }
        //cout<<"Qsize:"<<qsize<<endl;

        //sig reading
    POLY *buf = new POLY[qsize];
        assert(buf);
    if (fread(buf, sizeof(POLY), qsize, qfp) != (size_t) qsize) {
        assert(0);
         }
    for( int i = 0; i < qsize; i++){
            qsig.push_back(buf[i]);
                //cout<<buf[i]<<endl;
    }

    int qid=-1;
        if(fread(&qid, sizeof(qid), 1, qfp) != 1){
                assert(0);
        }

        if(qid != 0){
        cerr<<"Wrong qid "<<qid<<", please check again!"<<endl;
        assert(0);
    }

    delete[] buf;
                                          
    return true;
}

void
usage(void)
{
    cout << "Usage: " << __progname << " [-h] [options...]\n\n";

    cout << "ACTIONS:\n"
    << "    -c  create LSH index (requires -s, -H, -F, -j, -k, -l, -B, -b)\n"
    << "    -r  read/query LSH index (requires -q, -H, -F, -j, -k, -l)\n\n"

    << "FILES:\n"
    << "    -F  <hash funcs file>\n"
    << "    -j  <irrpoly file>\n\n"
    << "    -s  <input: signature data file>\n"
    << "    -H  <output: LSH index data file>\n"
    << "    -q  <test: query file>\n\n"
    //<< "  -L  <log file>\n"

    << "BLOOM PARAMETERS:\n"
    << "    -B  <bloom filter length>\n"
    << "    -b  <# of bloom hash functions>\n\n"

    << "LSH PARAMETERS:\n"
    << "    -d  <random prime number for LSH seed>\n"
    << "    -k  <LSH parameter>\n"
    << "    -l  <LSH parameter>\n"
    << "    -n  <how many sigs to process>\n\n"

    << "OPTIONS:\n"
    << "    -V  verbose\n"
    << "    -v  print version\n"
    << "    -h  print help/usage\n";
    exit(0);
}
