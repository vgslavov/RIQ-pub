// RIQ - RDF Indexing on Quads
// DBIS Lab: Praveen Rao, Vasil Slavov
// University of Missouri-Kansas City, 2014

#include "riq.h"
#include "utils.h"
#include "poly.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string>
#include <math.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <utility>
#include <db_cxx.h>
#include <assert.h>
#include <climits>
#include <dirent.h>
#include <unistd.h>
#include <libgen.h>
#include <set>
#include <bitset>
#include <queue>
#include <map>
#include <unordered_map>
#include <stack>
#include <list>
#include <cctype>
#include <algorithm>

#include <google/sparse_hash_map>
#include <rasqal.h>

//#include "bloom_counter.h"
//#include "bloom_filter.h"
#include "lsh.h"

// tell g++ it's a C library!
extern "C" {
    #include "dablooms.h"
}

// TODO
/* Safe casts: widening a value */
#define RASQAL_GOOD_CAST(t, v) (t)(v)
/* Unsafe casts: narrowing a value */
#define RASQAL_BAD_CAST(t, v) (t)(v)

using namespace std;

extern char *__progname;

int out;

// CBF parameters
DabloomsType dablooms_t = DACBF;
SpoOrderType spoorder_t = DEF_ORDER;
int CAPACITY = 0;
double ERROR_RATE = 0.0;
string metafile = "";
string idxdir = "";

bool verbose = false;
bool isdev = false;
// bits for length of cc id
const int CCBITS = 16;

// LSH parameters
int lshk = 16;
int lshl = 4;
int lshp = 1675850375;
// degree 22
POLY irrpoly = 2176081699;
vector<int> hasha;
vector<int> hashb;

// CBF capacities read from META file
vector<int> cbf_capacities;

// sorted GRAPHIDS
vector<string> graphids;

// mapping b/w SPO list and num
// TODO: don't use a map?
unordered_map<string, int> ext2list;
// XXX: rearrange order in CBF.META
unordered_map<string, int> ext2list_cbfmeta;
//unordered_map<int, string> list2ext;
vector<string> list2ext;

char *list2ext_char[SPOLISTS];
/*
// doesn't compile. c++11?
const char *list2ext_char[] =
{
    [SPO_LIST] = "spo",
    [SP_LIST] = "sp",
    [SO_LIST] = "so",
    [PO_LIST] = "po",
    [S_LIST] = "s",
    [P_LIST] = "p",
    [O_LIST] = "o"
};
*/

//typedef unordered_map<int, vector<int> > int2ints;
//int2ints edge_set;
//typedef unordered_map<int, set<int> > int2sets;
//int2sets edge_set;

// mapping b/w LSH hashes and graph/docIds
// SHA-1 hash
//typedef unordered_map<string, vector<int> > hash2ints;
//typedef map<string, vector<int> > hash2ints;
// POLY hash
//typedef unordered_map<POLY, vector<int> > hash2ints;
//typedef map<POLY, vector<int> > hash2ints;

// Google hash table
//typedef google::sparse_hash_map<int, vector<int> > hash2ints;
typedef google::sparse_hash_map<POLY, vector<int> > hash2ints;

typedef map<string, int> hashType;
hashType hashvalues;

// SPARQL query & sig
// bgp 2 SPOLISTS
typedef vector<vector<vector<string> > > querystrType;
typedef vector<vector<vector<POLY> > > querypolyType;
typedef vector<vector<vector<string> > > querypolystrType;
// bgp 2 SPOPVS sizes
typedef vector<vector<int> > querysizesType;
typedef vector<vector<int> > queryorderType;

static const int PAGE_SIZE = 8192;
static const int NUM_BUFFERS = 2048;
static const int MAX_NUMSIGS_UNION = 250;
int MAX_CC_UNION_SIZE = 100000000;
int MAX_CC_SIZE = 1000;
int MIN_CC_SIZE = 100;
static const int MAXSIGLEN = 16000 * 1024;
//static const int MAXSIGLEN = 4000 * 1024;
static const int MAX_QUERY_ERROR_REPORT_LEN = 512;

Db *myDB;
static const char *const SIGDB = "sigindex";

const char *RNODE = "root";
// Document id or node id
struct ChildID {
    int docId;
    char *nodeId;
};

struct Node {
    Node() : id(0) {}
    string data;
    int id;
    Node *parent;
    list<Node> children;
}; 

struct Disjunction {
    Disjunction() : val(false) {}
    void operator()(bool in) { val = (val || in); }
    bool val;
};

typedef list<Node>::const_iterator node_citr;
typedef list<Node>::iterator node_itr;

std::map<std::string, std::vector<int> > groups;
std::map<std::string, int> idList;

// querying
bool build_tree(string &, Node *);
void print_tree(const Node &, bool = true, bool = false);
void print_bgps(const querystrType &);
void print_bgps(const querypolyType &);
void print_order(const queryorderType &);
bool str2poly_bgps(const querystrType &, querypolyType &, querysizesType &,
                   bool = true);
bool poly2polystr_bgps(const querypolyType &, querypolystrType &);
bool eval_tree(const Node &, vector<bool> &, querypolystrType &,
               queryorderType &, int);
int print_eval_tree(const Node &, const vector<bool> &);
bool prune_tree(Node tree, const vector<bool> &eval, vector<Node *> &bgps2del);
bool prune_tree_helper(Node &tree, node_itr tree_itr, const vector<bool> &eval,
                       vector<Node *> &bgps2del);
bool del_subtree(Node &tree, vector<Node *> &bgps2del);
string fix_query_str(unsigned char *query_str_p);
bool check_bgp(const querypolystrType &, const queryorderType &, int, int);
bool sparql2sig(string &, string &, vector<vector<POLY> >&);
bool sparql2str(string &, string &);
string check_var(const string&);
bool check_prefix(string&);
bool queryBloomCounters(const querypolystrType &, const queryorderType &,
                        vector<string> &, int = -1, int = 0);
bool set_spo_order(querysizesType &, queryorderType &);
void set_mappings();
vector<string> unionize_candidates(const vector<string> &,
                                   const vector<string> &);

// rasqal
rasqal_query* r_init_query(rasqal_world *world, const char *ql_name,
                           const char *ql_uri,
                           const unsigned char *query_string,
                           raptor_uri *base_uri);
void r_print_query_to_file_handle(rasqal_query *rq,
                                  raptor_world *raptor_world_ptr,
                                  raptor_uri *base_uri, FILE *handle);
unsigned char *r_print_query_to_string(rasqal_query *rq,
                                       raptor_world *raptor_world_ptr,
                                       raptor_uri *base_uri);
void r_query_walk(rasqal_query *rq, Node *&tree, querystrType &strbgps);
void r_query_rewrite(rasqal_query *rq, vector<Node *> &bgps2del);
Node r_graph_pattern_walk(rasqal_graph_pattern *gp, int gp_index, int &size,
                          Node *parent, querystrType &strbgps);
void r_graph_pattern_rewrite(rasqal_graph_pattern *gp, int gp_index,
                             vector<Node *> &bgps2del);
void lit2str(rasqal_literal *lit, string &strlit);

// LSH
void build_graph(hash2ints &,vector<set<int> >&);
int bfs(int, vector<set<int> >&, vector<bool>&, vector<bool>&, vector<int>&,
        int&, int&, int&, vector<int>&, vector<int>&, vector<int>&);
void process_vertex_late(int);
void process_vertex_early(int, int, vector<int>&);
void process_edge(int, int);
int connected_components(vector<set<int> >&, vector<int>&, vector<int>&,
                         vector<int>&);
int lshsig(int, vector<vector<POLY> >, vector<bool>, hash2ints &);
void addhashv(vector<string>);
bool storeComponent(const char*, DbEnv&, Db*, string, vector<int>&);
bool constructUnionOptCC(const char *, DbEnv&, unordered_map<string, int>&);
string dec2bitstr(int);

// union construction
bool storeInDB(Db *, std::vector<POLY>&, ChildID&, int);
bool storeInDB(Db *, std::vector<std::vector<POLY> >&, ChildID&, int);
bool storePartition(const char *, DbEnv&, Db*, string&, multimap<int, int>&);
bool constructUnion(const char *, DbEnv&, map<std::string, int>&);
bool constructUnionOpt(const char *, DbEnv&, map<std::string, int>&);
void partitionO(const char *, DbEnv&, Db*, std::multimap<int, int>&, int,
                std::string&);
int pSim(std::vector<POLY>&, std::vector<POLY>&, std::vector<POLY>&, bool);
int pSimOpt(std::vector<POLY>&, std::vector<POLY>&, std::vector<POLY>&, bool);
int pSimOptNew(std::vector<POLY>&, std::vector<POLY>&, std::vector<POLY>&,
               bool);
int pSimOptNew(std::vector<std::vector<POLY> >&,
               std::vector<std::vector<POLY> >&,
               std::vector<std::vector<POLY> >&, bool);
void verifyProcess(char *, std::vector<std::vector<POLY> >&, int);

// utilities
bool list_files(const string &, vector<string> &);
void usage(void);

const char *NEXTNODEID = "nextnodeid";
NODEID nextNodeID;
NODEID rootNodeID = 1;
int queryID;

bool insertError;
bool retrieveError;

// hash table statistics
int numReads;
int numWrites;

// pSim
//bool metric;

// to avoid repeated node visits during query processing
// can happen due to concurrent operations
// can be avoided by periodic cleanup
std::map<std::string, bool> listOfVisitedNodes;

// Node split related stuff
std::vector<int> group1, group2;
std::vector<POLY> lcm1, lcm2;

unsigned int seedForRand;

// For verification
std::map<int, std::vector<int> > docMatchList;

// Data transfered
int dataFetched = 0, dataStored = 0;

// Edit cost for approximate matching
int v_R = 0, ve_D = 0, ve_I = 0, e_D = 0, e_I = 0;

// for picking seeds
int seedPickSize = 0;

int
compare_ID(Db *dbp, const Dbt *a, const Dbt *b)
{
    // Returns:
    // < 0 if a < b
    // = 0 if a = b
    // > 0 if a > b
    //assert(a->get_ulen() == b->get_ulen());
    //std::cout << "key 1: " << (char *) a->get_data()
    //                  << " key 1 size: " << a->get_size()
    //                  << " key 2: " << (char *) b->get_data()
    //                  << " key 2 size: " << b->get_size()
    //               << std::endl;
    if (a->get_size() != b->get_size()) {
        return (a->get_size() - b->get_size());
    }

    return memcmp(a->get_data(), b->get_data(), a->get_size());
}

int main(int argc, char *argv[])
{
    // getopt/getsubopt
    int ch;
    char *subopts, *value;

    // Jaccard, union
    // TODO: move
    int numBuffers = NUM_BUFFERS;
    int maxCount;
    int fanout;

    // flags
    bool doindex, doquery, doqopt, usejaccard, uselsh;
    bool vflag, aflag, Aflag, Mflag, qflag, dflag, rflag, Rflag;

    // files, etc.
    string dbname;
    string dbenvdir;
    string queryfile;
    string querylispfile;
    string querysparqlfile;
    string pvfile;
    string hashfile;
    string queryorder;
    string rewritedir;

    string pvskip = "";
    vector<bool> pvskiplists(SPOLISTS, false);

    // XXX: set ext to list and vice versa mappings
    set_mappings();

    doindex = doquery = doqopt = usejaccard = uselsh = false;
    vflag = aflag = Aflag = Mflag = qflag = dflag = rflag = false;

    while ((ch = getopt(argc, argv,
           "A:a:b:C:c:D:d:E:e:f:H:hIi:Jk:Ll:M:m:N:OP:p::Qq:R:r:S:s:U:Vvz"))
           != -1)
        switch(ch) {
        case 'A':
            Aflag = true;
            querysparqlfile = optarg;
            break;
        case 'a':
            aflag = true;
            querylispfile = optarg;
            break;
        case 'b':
            numBuffers = strtol(optarg, NULL, 10);
            break;
        case 'C':
            MAX_CC_SIZE = strtol(optarg, NULL, 10);
            break;
        case 'c':
            CAPACITY = strtol(optarg, NULL, 10);
            break;
        case 'D':
            dbname = optarg;
            break;
        case 'd':
            dflag = true;
            idxdir = optarg;
            break;
        case 'E':
            dbenvdir = optarg;
            break;
        case 'e':
            ERROR_RATE = strtod(optarg, NULL);
            break;
        case 'f':
            fanout = strtol(optarg, NULL, 10);
            break;
        case 'U':
            MAX_CC_UNION_SIZE = strtol(optarg, NULL, 10);
            break;
        case 'H':
            hashfile = optarg;
            break;
        case 'I':
            doindex = true;
            break;
        case 'i':
            irrpoly = strtol(optarg, NULL, 10);
            break;
        case 'J':
            usejaccard = true;
            break;
        case 'k':
            lshk = strtol(optarg, NULL, 10);
            break;
        case 'l':
            lshl = strtol(optarg, NULL, 10);
            break;
        case 'L':
            uselsh = true;
            break;
        case 'M':
            Mflag = true;
            metafile = optarg;
            break;
        case 'm':
            maxCount = strtol(optarg, NULL, 10);
            break;
        case 'N':
            MIN_CC_SIZE = strtol(optarg, NULL, 10);
            break;
        case 'O':
            doqopt = true;
            break;
        case 'P':
            pvfile = optarg;
            break;
        case 'p':
            lshp = strtol(optarg, NULL, 10);
            break;
        case 'Q':
            doquery = true;
            break;
        case 'q':
            qflag = true;
            queryfile = optarg;
            break;
        case 'R':
            Rflag = true;
            rewritedir = optarg;
            break;
        case 'r':
            rflag = true;
            queryorder = optarg;
            break;
        case 'S':
            subopts = optarg;
            while (*subopts != '\0')
                switch (getsubopt(&subopts, list2ext_char, &value)) {
                    case SPO_LIST:
                        pvskip = list2ext[SPO_LIST];
                        cout << "skipping PV list: " << pvskip << endl;
                        pvskiplists[SPO_LIST] = true;
                        break;
                    case SP_LIST:
                        pvskip = list2ext[SP_LIST];
                        cout << "skipping PV list: " << pvskip << endl;
                        pvskiplists[SP_LIST] = true;
                        break;
                    case SO_LIST:
                        pvskip = list2ext[SO_LIST];
                        cout << "skipping PV list: " << pvskip << endl;
                        pvskiplists[SO_LIST] = true;
                        break;
                    case PO_LIST:
                        pvskip = list2ext[PO_LIST];
                        cout << "skipping PV list: " << pvskip << endl;
                        pvskiplists[PO_LIST] = true;
                        break;
                    case S_LIST:
                        pvskip = list2ext[S_LIST];
                        cout << "skipping PV list: " << pvskip << endl;
                        pvskiplists[S_LIST] = true;
                        break;
                    case P_LIST:
                        pvskip = list2ext[P_LIST];
                        cout << "skipping PV list: " << pvskip << endl;
                        pvskiplists[P_LIST] = true;
                        break;
                    case O_LIST:
                        pvskip = list2ext[O_LIST];
                        cout << "skipping PV list: " << pvskip << endl;
                        pvskiplists[O_LIST] = true;
                        break;
                    default:
                        cout << "uknown PV list" << endl;
                        break;
                }
            break;
        case 's':
            seedPickSize = strtol(optarg, NULL, 10);
            break;
        case 'V':
            verbose = true;
            break;
        case 'v':
            vflag = true;
            break;
        case 'z':
            isdev = true;
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
        cout << "v2.1, DBIS Lab, CSEE, UMKC, 2014\n";
        return 0;
    }

    // wtf is this?
    v_R = 0;
    ve_D = 0;
    ve_I = 0;
    e_D = 0;
    e_I = 0;

    // either query or index, but not both
    if (!doquery == !doindex) usage();

    // either use jaccard or lsh when indexing, but not both
    if (doindex && (!uselsh == !usejaccard)) usage();

    // either sig or sparql text file
    if (doquery && (!Aflag == !qflag)) usage();

    // META file and idxdir required when querying
    if (doquery && (!Mflag && !dflag)) usage();

    // TODO: check if idxdir and rewritedir exist

    // read META file
    if (Mflag) {
        int sizes;
        cout << "Reading META file: " << metafile << endl;
        ifstream ifs(metafile, ifstream::in);
        if (!ifs) {
            cerr << "can't open META file" << endl;
            return EXIT_FAILURE;
        }

        while (ifs >> sizes) {
            cbf_capacities.push_back(sizes);
        }
    }

    // read GRAPHIDS when querying
    if (dflag && doquery) {
        stringstream ss;
        ss << idxdir << "/" << string(GRAPHIDSDIR) << "/";
        string graphidsdir = ss.str();

        bool ret;
        cout << "Creating list of GRAPHIDS..." << endl;
        ret = list_files(graphidsdir, graphids);
        if (!ret) {
            cerr << "can't list index dir" << endl;
            return EXIT_FAILURE;
        }

        //cout << "GRAPHIDS:" << endl;
        //for (size_t i = 0; i != graphids.size(); ++i) {
        //  cout << graphids[i] << endl;
        //}
    }

    // TODO: store in object?
    // all are required for query processing
    querystrType strbgps;
    querypolyType polybgps;
    querypolystrType polystrbgps;
    querysizesType bgp2sposizes;
    queryorderType spo_order;
    Node *parse_tree;
    string querystr;

    // set up rasqal
    rasqal_world *world = NULL;
    raptor_world *raptor_world_ptr = NULL;
    rasqal_query *rq = NULL;
    raptor_uri *base_uri = NULL;
    // can be: sparql, sparql11, laqrs
    const char *ql_name = "sparql11";
    char *ql_uri = NULL;
    raptor_uri *uri = NULL;
    unsigned char *uri_string = NULL;

    // SPARQL query
    if (Aflag) {
        cout << "Reading query SPARQL file: " << querysparqlfile << endl;

        bool ret = sparql2str(querysparqlfile, querystr);
        if (!ret) {
            cerr << "can't read SPARQL query to string" << endl;
            return EXIT_FAILURE;
        }

        cout << "\nquery string:\n" << querystr << endl;

        world = rasqal_new_world();
        if (!world || rasqal_world_open(world)) {
            cerr << "rasqal_world init failed" << endl;
            rasqal_free_world(world);
            return EXIT_FAILURE;
        }
        raptor_world_ptr = rasqal_world_get_raptor(world);

#if 0
        // TODO: use something else for base?
        // (optional)
        uri_string = raptor_uri_filename_to_uri_string(querysparqlfile.c_str());
        uri = raptor_new_uri(raptor_world_ptr, uri_string);
        //base_uri = raptor_uri_copy(uri);
#endif

        rq = r_init_query(world, ql_name, ql_uri,
                          (unsigned char*)querystr.c_str(), base_uri);

        if (rq) {
            r_print_query_to_file_handle(rq, raptor_world_ptr,
                                         base_uri, stdout);

            cout << "\nstructure format:\n";
            parse_tree = new Node();
            r_query_walk(rq, parse_tree, strbgps);

            int size = parse_tree->id;
            cout << "parse_tree size: " << size << endl;
            cout << "parse_tree root data: " << parse_tree->data << endl;
            //cout << "parse_tree root parent: " << parse_tree->parent << endl;
            print_tree(*parse_tree, true, true);
            cout << endl;

            cout << "string BGPs:\n";
            print_bgps(strbgps);

            str2poly_bgps(strbgps, polybgps, bgp2sposizes);
            cout << "POLY BGPs:\n";
            print_bgps(polybgps);

            cout << "query SPO order: ";
            if (rflag) {
                if (queryorder == "min") {
                    spoorder_t = MIN_ORDER;
                    cout << "min-sized list" << endl;
                } else if (queryorder == "max") {
                    spoorder_t = MAX_ORDER;
                    cout << "max-sized list" << endl;
                } else if (queryorder == "default") {
                    spoorder_t = DEF_ORDER;
                    cout << "default" << endl;
                } else {
                    cerr << "invalid order" << endl;
                    return EXIT_FAILURE;
                }
            } else {
                cout << "default" << endl;
            }

            set_spo_order(bgp2sposizes, spo_order);

            cout << "spo_order:" << endl;
            print_order(spo_order);

            // TODO: compare POLY and to_string(POLY)
            // in CBF checking (hashing) timing
            poly2polystr_bgps(polybgps, polystrbgps);
        }

        // TODO: package?
        if (uri)
            raptor_free_uri(uri);
        if (uri_string)
            raptor_free_memory(uri_string);
    }

    if (isdev) {
        int size = parse_tree->id;
        vector<bool> eval(size, false);
        cout << endl << "evaluating query..." << endl;
        bool ret = eval_tree(*parse_tree, eval, polystrbgps, spo_order, 0);
        cout << "eval_tree(): " << ret << endl;
        print_eval_tree(*parse_tree, eval);

        rasqal_query *rqmod;
        // TODO: how to duplicate rq instead of re-parsing query?
        rqmod = r_init_query(world, ql_name, ql_uri,
                             (unsigned char *)querystr.c_str(), base_uri);

        vector<Node *> bgps2del;
        if (rqmod) {
            cout << "pruning parse tree..." << endl;
            prune_tree(*parse_tree, eval, bgps2del);

            cout << "bgps2del: ";
            for (size_t i = 0; i != bgps2del.size(); ++i) {
                cout << (bgps2del[i])->id << ", ";
            }
            cout << endl;

            cout << "printing original parse tree..." << endl;
            print_tree(*parse_tree);
            cout << endl;

            cout << endl << "rewriting query..." << endl;
            r_query_rewrite(rqmod, bgps2del);

            cout << endl << "printing modified query..." << endl;
            r_print_query_to_file_handle(rqmod, raptor_world_ptr,
                                         base_uri, stdout);

            unsigned char *rqmod_str_p;
            rqmod_str_p = r_print_query_to_string(rqmod, raptor_world_ptr,
                                                  base_uri);
            if (rqmod_str_p) {
                cout << endl << "rqmod_str_p:" << endl << rqmod_str_p;
                string rqmod_str_fixed(fix_query_str(rqmod_str_p));
                if (rqmod_str_fixed.empty()) {
                    cout << "fix_query_str failed" << endl;
                    return EXIT_FAILURE;
                }
                cout << endl << "rqmod_str_fixed:" << endl
                     << rqmod_str_fixed << endl;

                // TODO: use boost
                char *querysparqlfile_p = new char[querysparqlfile.size()+1];
                strncpy(querysparqlfile_p, querysparqlfile.c_str(),
                        querysparqlfile.size());
                string querysparqlfile_base(basename(querysparqlfile_p));

                stringstream ss;
                ss << rewritedir << "/" << querysparqlfile_base << ".rqmod";
                ofstream out(ss.str());
                if (!out) {
                    free(rqmod_str_p);
                    free(querysparqlfile_p);
                    cerr << "can't create rewritten query file" << endl;
                    return EXIT_FAILURE;
                }
                out << rqmod_str_fixed;
                out.close();

                free(rqmod_str_p);
                free(querysparqlfile_p);
            } else {
                cout << "error converting query to string" << endl;
            }
        }

        cout << endl << "printing original query..." << endl;
        r_print_query_to_file_handle(rq, raptor_world_ptr, base_uri, stdout);

        // free memory
        // TODO: free all the individual Nodes
        if (parse_tree)
                delete parse_tree;
        if (rq)
            rasqal_free_query(rq);
        if (rqmod)
            rasqal_free_query(rqmod);
        if (world)
            rasqal_free_world(world);
        if (base_uri)
            raptor_free_uri(base_uri);

        return 0;
    }

    // deprecated
    vector<string> querylispstr;
    if (aflag) {
        cout << "Reading query parse tree file: " << querylispfile << endl;
        ifstream ifslisp(querylispfile);
        if (!ifslisp.is_open()) {
            cerr << "Error opening parse tree file" << endl;
            return EXIT_FAILURE;
        }

        string line;
        while (getline(ifslisp, line)) {
            querylispstr.push_back(line);
        }

        vector<Node> parsetrees;
        for (size_t i = 0; i != querylispstr.size(); ++i) {
            cout << querylispstr[i] << endl;
            Node tree;
            int size = 0;
            bool ret = build_tree(querylispstr[i], &tree);
            if (!ret) {
                cout << "error building tree\n";
                continue;
            } else {
                size = tree.id;
                cout << "tree size: " << size << endl;
                print_tree(tree);
                cout << endl;
                print_tree(tree, false);
                cout << endl;
                parsetrees.push_back(tree);
            }
#if 0
            vector<bool> eval(size, false);
            // TODO: call eval somewhere else
            int cc = 0;
            ret = eval_tree(tree, eval, cc);
            cout << "eval_tree(): " << ret << endl;
            print_eval_tree(tree, eval);
#endif
        }
    }

    // sig query
    if (qflag) {
        numReads = numWrites = 0;
        double totalTime = 0;
        FILE *qfp = fopen(queryfile.c_str(), "r");
        assert(qfp);

        int count = 1;
        // max # of queries to process
        maxCount = 2000;

        while (count <= maxCount) {
            int numSigs;
            if (fread(&numSigs, sizeof(numSigs), 1, qfp) != 1) {
                break;
            }
            assert(numSigs > 0);

            cout << "numSigs: " << numSigs << endl;
            polybgps.resize(numSigs);

            int docId;
            if (fread(&docId, sizeof(docId), 1, qfp) != 1) {
                assert(0);
            }

            cout << "query graph id: " << docId << endl;

            for (int t = 0; t < numSigs; t++) {
                for (int j = 0; j < SPOLISTS; j++) {
                    POLY *buf;
                    int size;
                    if (fread(&size, sizeof(int), 1, qfp) != 1) {
                        assert(0);
                    }

                    cout << "list " << j << " size: " << size << endl;

                    std::vector<POLY> list;
                    list.clear();
                    if (size == 0) {
                        polybgps[t].push_back(list);
                        continue;
                    }

                    buf = new POLY[size];
                    assert(buf);
                    if (fread(buf, sizeof(POLY), size, qfp) != (size_t) size) {
                        assert(0);
                    }

                    for (int i = 0; i < size; i++) {
                        list.push_back(buf[i]);
                        cout << "POLY: " << buf[i] << endl;
                    }

                    polybgps[t].push_back(list);
                    delete[] buf;
                }
            }
            cerr << "processing query " << docId << "\n";
            queryID = docId;

            count++;
            numReads = numWrites = 0;
            fclose(qfp);
        }

        cout << "Number of query signatures: " << polybgps.size() << "\n";
        bool ret = poly2polystr_bgps(polybgps, polystrbgps);
        if (!ret) {
            cerr << "error converting to polystr" << endl;
            return EXIT_FAILURE;
        }
    }

    if (usejaccard) {
        cerr << "fanout: " << fanout << "\n";
        cerr << "root node id: " << rootNodeID << "\n";
    } else if (uselsh) {
        cout << "using LSH\n";
        cout << "k: " << lshk << endl;
        cout << "l: " << lshl << endl;
        cout << "p: " << lshp << endl;
        cout << "irrpoly: " << irrpoly << endl;
        cout << "max # of sigs read: " << maxCount << endl;
        cout << "max # of sigs per union: " << MAX_NUMSIGS_UNION << endl;
        cout << "max CC size: " << MAX_CC_SIZE << endl;
        cout << "min CC size: " << MIN_CC_SIZE << endl;
        cout << "max CC union size: " << MAX_CC_UNION_SIZE << endl;

        // generate file with hash values for functions
        FILE *hashfp = NULL;
        string acc = "w+";
        if ((hashfp = fopen(hashfile.c_str(), acc.c_str())) == NULL) {
            cerr << "can't write hash file " << hashfile << endl;
            return EXIT_FAILURE;
        }

        int random_integer_a;
        int random_integer_b;
        int lowest_a = 1, highest_a = -9;
        int lowest_b = 0, highest_b = -9;
        highest_a = highest_b = lshp;
        int range_a = (highest_a - lowest_a) + 1;
        int range_b = (highest_b - lowest_b) + 1;
        hasha.clear();
        hashb.clear();
        srand(lshp);
        int totalfuncs = lshl * lshk;
        for (int i = 0; i < totalfuncs; i++) {
            // TODO: verify randomness
            random_integer_a = lowest_a + int((double)range_a*rand()/(RAND_MAX
                               + 1.0));
            random_integer_b = lowest_b + int((double)range_b*rand()/(RAND_MAX
                               + 1.0));
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

    DbEnv myEnv(0);
    if (doquery) {
        cout << "MAX capacity (# of inserts): " << CAPACITY << endl;
        cout << "error rate (%): " << ERROR_RATE * 100 << endl;
        if (doqopt) {
            double beginTime = getgtod();
            int ncandidates = 0;
            for (size_t i = 0; i != graphids.size(); ++i) {
                cout << "processing GRAPHID: " << graphids[i] << endl;
                int size = parse_tree->id;
                vector<bool> eval(size, false);
                bool ret = eval_tree(*parse_tree, eval, polystrbgps,
                                     spo_order, i);
                cout << "eval_tree(): " << ret << endl;
                print_eval_tree(*parse_tree, eval);

                if (ret == 0) {
                    cout << "non-candidate GRAPHID: " << graphids[i] << endl;
                    continue;
                } else {
                    cout << "candidate GRAPHID: " << graphids[i] << endl;
                    ++ncandidates;
                }

                rasqal_query *rqmod;
                // TODO: how to duplicate rq instead of re-parsing query?
                rqmod = r_init_query(world, ql_name, ql_uri,
                                     (unsigned char *)querystr.c_str(),
                                     base_uri);

                vector<Node *> bgps2del;
                if (rqmod) {
                    cout << "pruning parse tree..." << endl;
                    prune_tree(*parse_tree, eval, bgps2del);

                    cout << "bgps2del: ";
                    for (size_t i = 0; i != bgps2del.size(); ++i) {
                        cout << (bgps2del[i]->id) << ", ";
                    }
                    cout << endl;

                    cout << "printing original parse tree..." << endl;
                    print_tree(*parse_tree);
                    cout << endl;

                    cout << endl << "rewriting query..." << endl;
                    r_query_rewrite(rqmod, bgps2del);

                    //cout << endl << "printing modified query..." << endl;
                    //r_print_query_to_file_handle(rqmod, raptor_world_ptr,
                    //base_uri, stdout);
                    unsigned char *rqmod_str_p;
                    rqmod_str_p = r_print_query_to_string(rqmod,
                                                          raptor_world_ptr,
                                                          base_uri);
                    if (rqmod_str_p) {
                        //cout << endl << "rqmod_str_p:" << endl << rqmod_str_p;
                        string rqmod_str_fixed(fix_query_str(rqmod_str_p));
                        if (rqmod_str_fixed.empty()) {
                            cout << "fix_query_str failed, skipping" << endl;
                            continue;
                        }
                        //cout << endl << "rqmod_str_fixed:" << endl
                        //       << rqmod_str_fixed << endl;

                        // TODO: use boost
                        char *querysparqlfile_p = new char[querysparqlfile.size()+1];
                        strncpy(querysparqlfile_p, querysparqlfile.c_str(),
                                querysparqlfile.size());
                        string querysparqlfile_base(basename(querysparqlfile_p));

                        stringstream ss;
                        ss << rewritedir << "/" << querysparqlfile_base
                             << "." << graphids[i] << ".rqmod";
                        ofstream out(ss.str());
                        if (!out) {
                            free(rqmod_str_p);
                            free(querysparqlfile_p);
                            cerr << "can't create rewritten query file,"
                                 << " skipping" << endl;
                            continue;
                            //return EXIT_FAILURE;
                        }
                        out << rqmod_str_fixed;
                        out.close();

                        free(rqmod_str_p);
                        free(querysparqlfile_p);
                    } else {
                        cout << "error converting query to string, skipping"
                             << endl;
                    }
                }

                //cout << endl << "printing original query..." << endl;
                //r_print_query_to_file_handle(rq, raptor_world_ptr,
                //base_uri, stdout);

                // free memory
                if (rqmod)
                    rasqal_free_query(rqmod);
            }

            cout << "Candidates: " << ncandidates << endl;
            cout << "Total query time (opt): "
                 << getgtod() - beginTime << endl;
        } else {
            double totalTime = 0;
            vector<string> union_candidates;
            // defaults to BGP 0 (a 1-BGP query)
            int nbgps = polystrbgps.size();
            for (size_t i = 0; i != nbgps; ++i) {
                double beginTime = getgtod();
                vector<string> candidates;
                bool ret = queryBloomCounters(polystrbgps, spo_order,
                                              candidates, -1, i);
                // empty BGP
                if (!ret)
                    continue;

                cout << "Candidates [bgp #" << i << "]: "
                     << candidates.size() << endl;
                double endTime = getgtod();
                totalTime += endTime - beginTime;
                cout << "Query time (no opt) [bgp #" << i << "]: "
                     << endTime - beginTime << endl;

                // TODO: already sorted?
                //sort(candidates.begin(), candidates.end());
                //sort(union_candidates.begin(), union_candidates.end());

                union_candidates = unionize_candidates(candidates,
                                                       union_candidates);
            }
            cout << "Number of groups matched [all bgps]: "
                 << union_candidates.size() << endl;
            cout << "Total query time (no opt) [all bgps]: "
                 << totalTime << endl;
        }
    } else if (doindex) {
        if (pvskip == "") cout << "skipping PV lists: none" << endl;

        cout << "# of buffers: " << numBuffers << "\n";

        myEnv.set_cachesize(0, PAGE_SIZE * numBuffers, 0);
        myEnv.set_data_dir(".");

        try {
            myEnv.open(dbenvdir.c_str(), DB_CREATE | DB_INIT_MPOOL, 0);
        } catch (DbException &e) {
            cerr << "Exception occurred: " << e.what() << endl;
            return EXIT_FAILURE;
        }

        // handle for the berkeley db
        myDB = new Db(&myEnv, 0);
        myDB->set_pagesize(PAGE_SIZE);

        /*
        if (strcmp(cmd, "-L") && strcmp(cmd, "-Q") && strcmp(cmd, "-l")) {
            myDB->set_bt_compare(compare_ID);
        }
        */

        double startTime = getgtod();
        seedForRand = (unsigned int) startTime;

        FILE *sigfp = fopen(pvfile.c_str(), "r");
        assert(sigfp);

        std::multimap<int, int> sortedList;

        std::string myGroupID = std::string("0");
        std::string sigs(dbname);
        sigs += ".input";
        try {
            cout << sigs << std::endl;
            myDB->open(NULL, sigs.c_str(), myGroupID.c_str(),
                       DB_RECNO, DB_CREATE, 0);
        } catch(DbException &e) {
            cerr << "Failed to open DB object" << e.what() << endl;
            return EXIT_FAILURE;
        }

        double startIndexTime = getgtod();
        startTime = startIndexTime;

        int count = 1;
        // new format:
        // <sig ver> <graph id> <list size> <spo POLY list>...
        std::vector<std::vector<POLY> > siglists;
        siglists.clear();
        int listsize;

        // mapping b/w LSH hashes and graph/docIds
        hash2ints lsh2gids;
        vector<int> gid2size;
        vector<vector<int> > cc2gids;
        vector<int> cc2unionsize;
        unordered_map<string, int> ccname2size;
        // vector idx: vertex (graph id); vector val: component #
        vector<int> component;

        while (count <= maxCount) {

            // DONT use readData to retrieve signatures from input files...
            // since the size filed uses POLY as a basic unit and not byte...
            // Read numSigs <it should be 1> for data signatures...
            int numSigs;
            if (fread(&numSigs, sizeof(int), 1, sigfp) != 1) {
                break;
            }
            // sig v2
            assert(numSigs == 2);

            // Read the document id (graph id)
            int docId;
            if (fread(&docId, sizeof(docId), 1, sigfp) != 1) {
                assert(0);
            }
            //cout << "graph id: " << docId << endl;

            POLY e;
            if (fread(&listsize, sizeof(int), 1, sigfp) != 1) {
                assert(0);
            }
            siglists.resize(SPOLISTS);
            for (int j = 0; j < SPOLISTS; j++) {
                for (int i = 0; i < listsize; i++) {
                    if (fread(&e, sizeof(POLY), 1, sigfp) != 1) {
                        assert(0);
                    }
                    siglists[j].push_back(e);
                }
            }

            int degree = getDegreeNew(siglists);
            int bytesize = siglists[0].size() * SPOLISTS * sizeof(POLY);
            //std::cout << "Processing signature... " << docId;
            //cout << " size: " << siglists[0].size();
            //cout << " mem: " << bytesize;
            //cout << " count: " << count << endl;
            if (bytesize > MAXSIGLEN) {
                cout << "Skipping sigid: " << docId
                     << " mem: " << bytesize << endl;
                siglists.clear();
                // don't increment count if you skip!
                //count++;
                //gid2size.push_back(0);
                continue;
            }
            sortedList.insert(std::pair<int,int>(degree,count));

            ChildID cId;

            cId.docId = docId;
            cId.nodeId = NULL;
            storeInDB(myDB, siglists, cId, count);

            if (uselsh) {
                // ATTN: don't store docId 0!
                int size = lshsig(docId+1, siglists, pvskiplists, lsh2gids);
                gid2size.push_back(size);
            }

            siglists.clear();
            // includes one extra for vertex id 0
            ++count;
        }
        // close the DB
        myDB->close(0);

        fclose(sigfp);

        double finishTime = getgtod();
        cout << "Reading PVs (+ LSH) time: "
             << finishTime - startTime << endl;

        if (uselsh) {
            cout << "lsh2gids.size(): " << lsh2gids.size() << endl;
            if (verbose) {
                hash2ints::iterator itr;
                for (itr = lsh2gids.begin(); itr != lsh2gids.end(); itr++) {
                    cout << itr->first << ": ";
                    int len = itr->second.size();
                    for (int i = 0; i < len; i++) {
                        cout << itr->second[i] << " ";
                    }
                    if (len > 1)
                        cout << " len: " << len;
                    cout << endl;
                }
            }

            // count == total # of graph ids + 1
            // (ignore vertex id 0)
            vector<set<int> > edge_set(count);
            startTime = getgtod();
            build_graph(lsh2gids, edge_set);
            finishTime = getgtod();
            cout << "Building graph time: " << finishTime - startTime << endl;

            cout << "edge_set.size(): " << edge_set.size() << endl;
            if (verbose) {
                for (int i = 0; i < edge_set.size(); i++) {
                    cout << i;
                    set<int>::iterator sitr;
                    for (sitr = edge_set[i].begin(); sitr != edge_set[i].end();
                         sitr++) {

                        cout << " " << *sitr;
                    }
                    cout << endl;
                }
            }

            component.resize(edge_set.size());

            startTime = getgtod();
            int ccnum = connected_components(edge_set, component,
                                             gid2size, cc2unionsize);
            finishTime = getgtod();
            cout << "Connected components time: "
                 << finishTime - startTime << endl;

            cout << "Total connected components: " << ccnum << endl;
            // why?
            cc2gids.resize(ccnum+1);

            cout << "component.size(): " << component.size() << endl;
            for (int i = 0; i < component.size(); i++) {
                if (verbose)
                    cout << "Vertex " << i << " is in component "
                         << component[i] << endl;
                if (component[i] != -1) {
                    try {
                        cc2gids.at(component[i]).push_back(i);
                    } catch (const std::out_of_range& e) {
                        cout << "cc2gids idx out of range: "
                             << component[i] << endl;
                    }
                }
            }

            //if (verbose) {
                cout << "cc2gids.size(): " << cc2gids.size() << endl;
                for (int i = 0; i < cc2gids.size(); i++) {
                    cout << "Component " << i
                             << ": graphs: " << cc2gids[i].size()
                             << " POLYs: " << cc2unionsize[i] << endl;
                }
            //}
        }

        //cout << "Exiting before union construction\n";
        //return 0;

        //std::vector<std::vector<POLY> > topSigs;

        idList.clear();

        Db* sigDB = new Db(&myEnv, 0);
        sigDB->set_pagesize(PAGE_SIZE);
        // OPEN the DB
        try {
            sigDB->open(NULL, sigs.c_str(), myGroupID.c_str(),
                        DB_RECNO, DB_RDONLY, 0);
        } catch(DbException &e) {
            cerr << "Failed to open DB object (1)" << e.what() << endl;
            return EXIT_FAILURE;
        }

        if (uselsh) {
            startTime = getgtod();

#if 0
            // TODO: combinging small groups

            // component # -> # of max-sized chunks
            vector<int> maxchunks(cc2gids.size());
            // component # -> leftover size
            vector<int> leftovers(cc2gids.size());
            int i = 0;
            while (i < cc2gids.size()) {
                int ccsize = cc2gids[i].size();
                cout << "CC: " << i << " (before): " << ccsize << endl;

                int rem = ccsize % MAX_CC_SIZE;
                int max = (int)floor(ccsize / MAX_CC_SIZE);

                // can fit in one bucket, put in leftovers
                if (ccsize < MAX_CC_SIZE) {
                    // TODO: is this efficient?
                    leftovers.insert(leftovers.begin() + i, ccsize);
                // can fit in exactly N buckets
                } else if (rem == 0){
                    maxchunks.insert(maxchunks.begin() + i, max);
                // can't fit and there is leftover
                } else {
                    maxchunks.insert(maxchunks.begin() + i, max);
                    leftovers.insert(leftovers.begin() + i, rem);
                }
                ++i;
            }

            for (int i = 0; i < cc2gids.size(); i++) {
                cout << "CC: " << i << " maxchunks: " << maxchunks[i]
                         << " leftovers: " << leftovers[i] << endl;
            }
            finishTime = getgtod();
            cout << "Calculate regrouping time: "
                 << finishTime - startTime << std::endl;
            //cout << "Balance groups time: "
            //     << finishTime - startTime << std::endl;

            /*
            for (int i = 0; i < cc2gids.size(); i++) {
                cout << i << " (after): " << cc2gids[i].size() << endl;
            }
            */
#endif

            startTime = getgtod();
            for (int i = 0; i < cc2gids.size(); i++) {
                string ccname = dec2bitstr(i);
                int ccsize = cc2gids[i].size();
                //if (verbose)
                //  cout << ccname << " (" << i << "): " << ccsize << endl;
                cout << ccname << " (" << i << "): " << ccsize << endl;
                storeComponent(dbname.c_str(), myEnv, sigDB,
                               ccname, cc2gids[i]);
                ccname2size[ccname] = ccsize;
            }
            finishTime = getgtod();
            cout << "Store components time: "
                 << finishTime - startTime << std::endl;

            if (verbose) {
                unordered_map<string, int>::iterator itr;
                for (itr = ccname2size.begin(); itr != ccname2size.end();
                     itr++) {

                    cout << itr->first << " --> " << itr->second << endl;
                }
            }
        } else {
            startTime = getgtod();
            partitionO(dbname.c_str(), myEnv, sigDB, sortedList,
                       fanout, myGroupID);
            finishTime = getgtod();
            cout << "Partitioning + storing time: "
                 << finishTime - startTime << std::endl;

            if (verbose) {
                for (std::map<std::string, int>::iterator itr = idList.begin();
                     itr != idList.end(); itr++) {

                    cout << "--> " << itr->first << " "
                         << itr->second << endl;
                }
            }
        }

        startTime = getgtod();
        if (uselsh) {
            constructUnionOptCC(dbname.c_str(), myEnv, ccname2size);
        } else {
            constructUnionOpt(dbname.c_str(), myEnv, idList);
        }
        finishTime = getgtod();
        cout << "Union construction time: "
             << finishTime - startTime << endl;

        // Delete the main DB... later
        sigDB->close(0);
        delete sigDB;

        finishTime = getgtod();
        cout << "Total indexing time: "
             << finishTime - startIndexTime << endl;
    } else {
        cout << "Invalid option" << endl;
    }

    // Not opened for verification
    if (doindex) {
        cout << "Already closed..." << endl;
        cout << "Printing Statistics" << endl;
        myEnv.memp_stat_print(DB_STAT_CLEAR);
        //myEnv.memp_stat_print(DB_STAT_CLEAR);
        //myEnv.remove(dbenvdir.c_str(), DB_FORCE);
        myEnv.close(0);
        delete myDB;
    }

    // TODO: package all free/del

#if 0
    // segfaults
    // TODO: call delete for each Node?
    vector<Node *> del;
    if (parse_tree) {
        del_subtree(*parse_tree, del);
        delete parse_tree;
    }
#endif

    if (rq)
        rasqal_free_query(rq);
    if (world)
        rasqal_free_world(world);
    if (base_uri)
        raptor_free_uri(base_uri);

    return 0;
}

// TODO: better way?
vector<string> unionize_candidates(const vector<string> &candidates,
                                   const vector<string> &union_candidates)
{
    vector<string> new_union;

    set_union(candidates.begin(), candidates.end(),
              union_candidates.begin(), union_candidates.end(),
              back_inserter(new_union));

    return new_union;
}

rasqal_query* r_init_query(rasqal_world *world,
                           const char *ql_name,
                           const char *ql_uri,
                           const unsigned char *query_string,
                           raptor_uri *base_uri)
{
    rasqal_query* rq;

    rq = rasqal_new_query(world, (const char *)ql_name,
                          (const unsigned char*)ql_uri);
    if (!rq) {
        cerr << "Failed to create query name\n";
        return NULL;
    }

    if (rasqal_query_prepare(rq, query_string, base_uri)) {
        size_t len = strlen((const char*)query_string);

        cout << "Parsing query '";
        if(len > MAX_QUERY_ERROR_REPORT_LEN) {
            (void)fwrite(query_string,
                         RASQAL_GOOD_CAST(size_t, MAX_QUERY_ERROR_REPORT_LEN),
                         sizeof(char), stderr);
            fprintf(stderr, "...' (%d bytes) failed\n",
                    RASQAL_BAD_CAST(int, len));
        } else {
            (void)fwrite(query_string, len, sizeof(char), stderr);
            fputs("' failed\n", stderr);
        }

        rasqal_free_query(rq); rq = NULL;
    }

    return rq;
}

void r_print_query_to_file_handle(rasqal_query *rq,
                                  raptor_world *raptor_world_ptr,
                                  raptor_uri *base_uri,
                                  FILE *handle)
{
    //cout << "\ndebug format:\n";
    //rasqal_query_print(rq, handle);

    raptor_iostream* output_iostr;
    output_iostr = raptor_new_iostream_to_file_handle(raptor_world_ptr,
                                                      handle);
    cout << "sparql format (file handle):\n";
    rasqal_query_write(output_iostr, rq, NULL, base_uri);
    raptor_free_iostream(output_iostr);
}

// inconsistent (see above), but returning string as an arg is a pain
unsigned char *r_print_query_to_string(rasqal_query *rq,
                                       raptor_world *raptor_world_ptr,
                                       raptor_uri *base_uri)
{
    void *string;
    size_t length;

    raptor_iostream* string_iostr;
    string_iostr = raptor_new_iostream_to_string(raptor_world_ptr,
                                                 &string,
                                                 &length,
                                                 (raptor_data_malloc_handler)malloc);
    //cout << "sparql format (string):\n";
    rasqal_query_write(string_iostr, rq, NULL, base_uri);
    raptor_free_iostream(string_iostr);

    return (unsigned char *)string;
}

void r_query_walk(rasqal_query *rq,
                  Node *&tree,
                  querystrType &strbgps)
{
    rasqal_graph_pattern *gp;
    gp = rasqal_query_get_query_graph_pattern(rq);
    if (gp) {
        // IMP: start at 0 so Rasqal's BGP # match the parse trees #
        int size = 0;
        // root
        tree->children.push_back(r_graph_pattern_walk(gp, -1, size,
                                                      NULL, strbgps));
        tree->id = size;
    }
}

Node r_graph_pattern_walk(rasqal_graph_pattern *gp,
                          int gp_index,
                          int &size,
                          Node *parent,
                          querystrType &strbgps)
{
    Node *p;

    rasqal_graph_pattern_operator op;
    op = rasqal_graph_pattern_get_operator(gp);

    string pattern = rasqal_graph_pattern_operator_as_string(op);
    cout << pattern << " graph pattern";

    p = new Node();
    p->id = size;
    p->parent = parent;
    if (pattern == "Graph") {
        p->data = "graphGraphPattern GRAPH";
    } else if (pattern == "Group") {
        p->data = "groupGraphPattern";
    } else if (pattern == "Union") {
        p->data = "groupOrUnionGraphPattern UNION";
    } else if (pattern == "Optional") {
        p->data = "optionalGraphPattern OPTIONAL";
    } else if (pattern == "Basic") {
        p->data = "triplesBlock";
    }
    // TODO: add filter, exists, notExists, expression when Rasqal supports it
    ++size;

    int idx = rasqal_graph_pattern_get_index(gp);
    cout << "[" << idx << "]" << endl;

    //int ret = rasqal_graph_pattern_print(gp, stdout);

    // TODO: call these within if/else above?

    // GRAPH literal
    rasqal_literal *literal;
    literal = rasqal_graph_pattern_get_origin(gp);
    if (literal) {
        cout << "origin ";
        rasqal_literal_print(literal, stdout);
        cout << endl;
    }

    // triples
    int triple_index = 0;
    // store s,p,o in separate lists
    vector<vector<string> > bgp(3);
    while (1) {
        rasqal_triple *t;

        t = rasqal_graph_pattern_get_triple(gp, triple_index);
        if (!t)
            break;

        cout << "triples: " << triple_index << " ";
        rasqal_triple_print(t, stdout);
        cout << endl;

        string s;
        cout << "subject ";
        lit2str(t->subject, s);
        cout << "predicate ";
        string p;
        lit2str(t->predicate, p);
        cout << "object ";
        string o;
        lit2str(t->object, o);

        bgp[0].push_back(s);
        bgp[1].push_back(p);
        bgp[2].push_back(o);
        //spo2lists(s, p, o, strbgps, idx);

        ++triple_index;
    }
    // TODO: don't store empty graph patterns
    // includes empty graph patterns such that bgp # would match idx
    strbgps.push_back(bgp);

    // sub-graph patterns
    raptor_sequence *seq;
    seq = rasqal_graph_pattern_get_sub_graph_pattern_sequence(gp);
    if (seq && raptor_sequence_size(seq) > 0) {
        cout << "sub-graph patterns: " << raptor_sequence_size(seq) << endl;

        gp_index = 0;
        while (1) {
            rasqal_graph_pattern *sgp;
            sgp = rasqal_graph_pattern_get_sub_graph_pattern(gp, gp_index);
            if (!sgp) {
                break;
            }

            p->children.push_back(r_graph_pattern_walk(sgp, gp_index, size,
                                                       p, strbgps));
            ++gp_index;
        }
    }

    // FILTER
    rasqal_expression *expr;
    expr = rasqal_graph_pattern_get_filter_expression(gp);
    if (expr) {
        cout << "filter ";
        rasqal_expression_print(expr, stdout);
        cout << endl;
    }

    return *p;
}

void r_query_rewrite(rasqal_query *rq, vector<Node *> &bgps2del)
{
    rasqal_graph_pattern *gp;
    gp = rasqal_query_get_query_graph_pattern(rq);
    if (gp) {
        r_graph_pattern_rewrite(gp, -1, bgps2del);
    }
}

void r_graph_pattern_rewrite(rasqal_graph_pattern *gp,
                             int gp_index,
                             vector<Node *> &bgps2del)
{
    rasqal_graph_pattern_operator op;
    op = rasqal_graph_pattern_get_operator(gp);

    string pattern = rasqal_graph_pattern_operator_as_string(op);
    cout << endl << pattern << " graph pattern";

    int idx = rasqal_graph_pattern_get_index(gp);
    cout << "[" << idx << "]" << endl;

    cout << "gp_index: " << gp_index << " idx: " << idx << endl;

    // XXX: better way to figure out if this seq contains bgp2del?
    bool parent_gp = false;
    int bgp2del_idx = -1;
    for (size_t i = 0; i != bgps2del.size(); ++i) {
        // this is the parent gp of the bgp to be del.
        if ((bgps2del[i]->parent->id) == idx) {
            parent_gp = true;
            bgp2del_idx = bgps2del[i]->id;
            cout << "found parent gp of bgp2del: " << bgp2del_idx << endl;
            break;
        }
    }

    //int ret = rasqal_graph_pattern_print(gp, stdout);

    // GRAPH literal
    rasqal_literal *literal;
    literal = rasqal_graph_pattern_get_origin(gp);
    if (literal) {
        cout << "origin ";
        rasqal_literal_print(literal, stdout);
        cout << endl;
    }

    // ignore triples

    // sub-graph patterns
    raptor_sequence *seq;
    raptor_sequence *seq2mod;
    seq = rasqal_graph_pattern_get_sub_graph_pattern_sequence(gp);
    if (seq && raptor_sequence_size(seq) > 0) {
        int seq_size = raptor_sequence_size(seq);
        cout << "sub-graph patterns: " << seq_size << endl;
        cout << "printing sequence (before):\n";
        raptor_sequence_print(seq, stdout);
        cout << endl;

        if (parent_gp) {
            // TODO: leaking seq2mod?
            int gap_idx = -1;
            for (int i = 0; i != seq_size; ++i) {
                rasqal_graph_pattern *i_gp = (rasqal_graph_pattern*)
                                             raptor_sequence_get_at(seq, i);
                cout << "get_at igp:\n";
                rasqal_graph_pattern_print(i_gp, stdout);
                cout << endl;
                int igp_idx = rasqal_graph_pattern_get_index(i_gp);
                cout << "igp idx [" << igp_idx << "]" << endl;

                // fill gap of deleted gp by shifting gps forward
                // XXX: bgp ids are not valid any more
                if (gap_idx != -1) {
                    int ret = raptor_sequence_set_at(seq, gap_idx,
                                                     (rasqal_graph_pattern *)
                                                     i_gp);

                    // on failure, stop trying to shift
                    if (ret) {
                        gap_idx = -1;
                        cout << "failed to set gp" << endl;
                        // TODO: what to do?
                        continue;
                    }
                    cout << "successfully set gp" << endl;
                    cout << "deleting shifted gp" << endl;
                    rasqal_graph_pattern *del_gp = (rasqal_graph_pattern*)raptor_sequence_delete_at(seq, i);
                    // keep shifting
                    gap_idx = i;
                }

                // delete gp
                if (igp_idx == bgp2del_idx) {
                    rasqal_graph_pattern *del_gp = (rasqal_graph_pattern*)
                                                    raptor_sequence_delete_at
                                                    (seq, i);
                    cout << "delete_at igp:\n";
                    rasqal_graph_pattern_print(del_gp, stdout);
                    cout << endl;
                    gap_idx = i;
                }

            }
            cout << "after delete_at: " << raptor_sequence_size(seq) << endl;
            cout << "printing sequence (after):\n";
            raptor_sequence_print(seq, stdout);
            cout << endl;
        }
        gp_index = 0;
        while (1) {
            rasqal_graph_pattern *sgp;
            sgp = rasqal_graph_pattern_get_sub_graph_pattern(gp,
                                                             gp_index);
            if (!sgp) {
                cout << "sgp is NULL" << endl;
                break;
            }

            r_graph_pattern_rewrite(sgp, gp_index, bgps2del);
            ++gp_index;
        }
    }

    // FILTER
    rasqal_expression *expr;
    expr = rasqal_graph_pattern_get_filter_expression(gp);
    if (expr) {
        cout << "filter ";
        rasqal_expression_print(expr, stdout);
        cout << endl;
    }
}

// TODO: better way?
void lit2str(rasqal_literal *lit, string &strlit)
{
    stringstream ss;
    string str, substr;
    int start_pos = 0;

    if (!lit)
        return;

    switch(lit->type) {
        case RASQAL_LITERAL_URI:
            cout << "(uri): ";
            ss << rasqal_literal_as_string(lit);
            str = ss.str();

            // XXX: quick and dirty
            substr = "file://";
            start_pos = str.find(substr);
            //cout << "start_pos: " << start_pos
            //       << " substr.size(): " << substr.size() << endl;

            if (start_pos != string::npos) {
                str.erase(start_pos, substr.size());
            }

            ss.clear();
            ss.str(std::string());
            ss << "<" << str << ">";

            break;
        // TODO
        case RASQAL_LITERAL_BLANK:
            cout << "(blank)" << endl;
            break;
        case RASQAL_LITERAL_STRING:
        case RASQAL_LITERAL_XSD_STRING:
            cout << "(string): ";
            ss << "\"" << rasqal_literal_as_string(lit) << "\"";
            if (lit->language) {
                ss << "@" << lit->language;
            }
            if (lit->datatype) {
                ss << "^^" << "<" << raptor_uri_as_string(lit->datatype)
                   << ">";
            }
            break;
        case RASQAL_LITERAL_VARIABLE:
            cout << "(var): ";
            rasqal_variable *var;
            var = rasqal_literal_as_variable(lit);
            ss << "?" << var->name;
            break;
        // TODO: add more?
        case RASQAL_LITERAL_UNKNOWN:
            cout << "(unknown)" << endl;
            break;
    }

    if (!ss.str().empty()) {
        strlit = ss.str();
        cout << strlit << endl;
    }
}

// TODO: make template?
void print_bgps(const querystrType &bgps)
{
    for (size_t i = 0; i != bgps.size(); ++i) {
        cout << "bgp # " << i << ":" << endl;
        for (size_t j = 0; j != bgps[i].size(); ++j) {
            if (j == 0) cout << "subjects";
            if (j == 1) cout << "predicates";
            if (j == 2) cout << "objects";
            cout << "(" << bgps[i][j].size() << "): ";
            for (size_t k = 0; k != bgps[i][j].size(); ++k) {
                cout << bgps[i][j][k] << ", ";
            }
            cout << endl;
        }
    }
}

// TODO: make template?
void print_bgps(const querypolyType &bgps)
{
    for (size_t i = 0; i != bgps.size(); ++i) {
        cout << "bgp # " << i << ":" << endl;
        for (size_t j = 0; j != bgps[i].size(); ++j) {
            cout << "SPOLIST["
                     << list2ext[j] << ":" << j << "]"
                     << "(" << bgps[i][j].size() << "): ";
            for (size_t k = 0; k != bgps[i][j].size(); ++k) {
                cout << bgps[i][j][k] << ", ";
            }
            cout << endl;
        }
    }
}

void print_order(const queryorderType &spo_order)
{
    for (size_t i = 0; i != spo_order.size(); ++i) {
        cout << "bgp # " << i << ":" << endl;
        for (size_t j = 0; j != spo_order[i].size(); ++j) {
            cout << "order: " << j
                     << " SPOLIST["
                     << list2ext[spo_order[i][j]] << ":"
                     << spo_order[i][j] << "]" << endl;
        }
    }
}

bool poly2polystr_bgps(const querypolyType &polybgps,
                       querypolystrType &polystrbgps)
{
    if (polystrbgps.empty())
        polystrbgps.resize(polybgps.size());
    else
        return false;

    // bgp #
    for (size_t i = 0; i != polybgps.size(); ++i) {
        polystrbgps[i].resize(SPOLISTS);
        // SPOLIST
        for (size_t j = 0; j != polybgps[i].size(); ++j) {
            // POLY
            for (size_t k = 0; k != polybgps[i][j].size(); ++k) {
                polystrbgps[i][j].push_back(to_string(polybgps[i][j][k]));
            }
        }
    }

    return true;
}

bool str2poly_bgps(const querystrType &strbgps,
                   querypolyType &polybgps,
                   querysizesType &bgp2sposizes,
                   bool dosort)
{
    string s, p, o;

    if (polybgps.empty())
        polybgps.resize(strbgps.size());
    else
        return false;

    if (bgp2sposizes.empty())
        bgp2sposizes.resize(strbgps.size());
    else
        return false;

    // bgp #
    for (size_t i = 0; i != strbgps.size(); ++i) {
        if (strbgps[i].size() != 3) {
            cout << "warning: bgp # " << i << " is empty\n";
            continue;
        }

        // TODO: don't do it on the fly?
        polybgps[i].resize(SPOLISTS);
        bgp2sposizes[i].resize(SPOLISTS);

        // s/p/o
        for (size_t j = 0; j != strbgps[i][0].size() &&
                           j != strbgps[i][1].size() &&
                           j != strbgps[i][2].size(); ++j) {

            s = check_var(strbgps[i][0][j]);
            p = check_var(strbgps[i][1][j]);
            o = check_var(strbgps[i][2][j]);

            stringstream ss;
            ss << "( " << s << " , " << p << " , " << o << " )";
            string store = ss.str();

            if (verbose)
                cout << store << endl;

            // verified correctness
            // TODO: use the other one instead?
            POLY irrPoly = lshp;

            POLY pol;
            pol = hashIrrPoly(&store[0], irrPoly);
            if (verbose)
                cout << "Poly: " << pol << endl;

            // TODO: use SPOPVS enum?
            // ( s, p, o )
            if (s != "?" && p != "?" && o != "?") {
                polybgps[i][0].push_back(pol);
            // ( s, p, ? )
            } else if (s != "?" && p != "?" && o == "?") {
                polybgps[i][1].push_back(pol);
            // ( s, ?, o )
            } else if (s != "?" && p == "?" && o != "?") {
                polybgps[i][2].push_back(pol);
            // ( ?, p, o )
            } else if (s == "?" && p != "?" && o != "?") {
                polybgps[i][3].push_back(pol);
            // ( s, ?, ? )
            } else if (s != "?" && p == "?" && o == "?") {
                polybgps[i][4].push_back(pol);
            // ( ?, p, ? )
            } else if (s == "?" && p != "?" && o == "?") {
                polybgps[i][5].push_back(pol);
            // ( ?, ?, o )
            } else if (s == "?" && p == "?" && o != "?") {
                polybgps[i][6].push_back(pol);
            } else {
                cout << "warning: invalid pattern: all vars\n";
            }
        }

        if (dosort) {
            for (size_t k = 0; k != polybgps[i].size(); ++k) {
                sort(polybgps[i][k].begin(), polybgps[i][k].end());
                bgp2sposizes[i][k] = polybgps[i][k].size();
            }
        }
    }

    return true;
}

// TODO: more efficient way?
// sorts SPOs in query BGPS: min/max/default
bool set_spo_order(querysizesType &bgp2sposizes, queryorderType &spo_order)
{
    if (!spo_order.empty())
        return false;
    else
        spo_order.resize(bgp2sposizes.size());

    // bgp #
    for (size_t i = 0; i != bgp2sposizes.size(); ++i) {

        multimap<int, int> size2spo;
        spo_order[i].resize(SPOLISTS);
        // SPOLISTS
        for (size_t j = 0; j != bgp2sposizes[i].size(); ++j) {
            if (spoorder_t == DEF_ORDER) {
                spo_order[i][j] = j;
            } else {
                size2spo.insert(pair<int, int>(bgp2sposizes[i][j], j));
            }
        }

        if (spoorder_t == DEF_ORDER)
            continue;

        // TODO: better way?
        int k;
        multimap<int, int>::iterator itr;
        if (spoorder_t == MIN_ORDER) {
            k = 0;
            for (itr = size2spo.begin();
                 itr != size2spo.end() && k != SPOLISTS; ++itr) {

                spo_order[i][k] = itr->second;
                ++k;
            }
        } else if (spoorder_t == MAX_ORDER) {
            k = SPOLISTS;
            for (itr = size2spo.begin();
                 itr != size2spo.end() && k != 0; ++itr) {

                --k;
                spo_order[i][k] = itr->second;
            }
        }
    }

    return true;
}

// TODO: change CBF.META order to be the same as rdf2spovec
void set_mappings()
{
    // CBF.META order
    // (scandir alpha-sorted order)
    ext2list_cbfmeta["spo"] = 6;
    ext2list_cbfmeta["sp"] = 5;
    ext2list_cbfmeta["so"] = 4;
    ext2list_cbfmeta["s"] = 3;
    ext2list_cbfmeta["po"] = 2;
    ext2list_cbfmeta["p"] = 1;
    ext2list_cbfmeta["o"] = 0;

    // rdf2spovec order
    ext2list["spo"] = SPO_LIST;
    ext2list["sp"] = SP_LIST;
    ext2list["so"] = SO_LIST;
    ext2list["po"] = PO_LIST;
    ext2list["s"] = S_LIST;
    ext2list["p"] = P_LIST;
    ext2list["o"] = O_LIST;

    // rdf2spovec order
    list2ext.resize(SPOLISTS);
    list2ext[SPO_LIST] = "spo";
    list2ext[SP_LIST] = "sp";
    list2ext[SO_LIST] = "so";
    list2ext[PO_LIST] = "po";
    list2ext[S_LIST] = "s";
    list2ext[P_LIST] = "p";
    list2ext[O_LIST] = "o";

    // rdf2spovec order
    // for getsubopt pvskiplist parsing
    // removed warning: deprecated conversion from string constant to ‘char*’
    // by using -Wno-write-strings
    list2ext_char[SPO_LIST] = "spo";
    list2ext_char[SP_LIST] = "sp";
    list2ext_char[SO_LIST] = "so";
    list2ext_char[PO_LIST] = "po";
    list2ext_char[S_LIST] = "s";
    list2ext_char[P_LIST] = "p";
    list2ext_char[O_LIST] = "o";
}

// Signature and docid as an int
// sig v1
bool storeInDB(Db* myDB, std::vector<POLY>& sig, ChildID& childId, int id)
{
    db_recno_t recid = id;
    Dbt *key = new Dbt(&recid, sizeof(u_int32_t));

    char *dbuf;
    int dLen;

    // Integer
    if (childId.docId >= 0) {
        makeKeyValue(&dbuf, dLen, sig, childId.docId);
    }
    else if (childId.nodeId) {
        makeKeyValue(&dbuf, dLen, sig, childId.nodeId);
    }

    Dbt *data = new Dbt(dbuf, dLen);

    int ret = myDB->put(NULL, key, data, DB_APPEND);

    if (ret != 0) {
        std::cerr << "Problem inserting record" << std::endl;
        cleanup(dbuf);
        delete key;
        delete data;
        return false;
    }

    //std::cout << "Record id: " << *((db_recno_t *) key->get_data()) << std::endl;
    cleanup(dbuf);

    delete key;
    delete data;

    return true;
}

// Signature and docid as an int
// sig v2
bool storeInDB(Db* myDB, std::vector<std::vector<POLY> >& siglists,
                             ChildID& childId, int id)
{
    db_recno_t recid = id;
    Dbt *key = new Dbt(&recid, sizeof(u_int32_t));

    char *dbuf;
    int dLen;

    /*
    int tmpdocid;
    std::vector<std::vector<POLY> > vlists;
    vlists.clear();
    vlists.resize(SPOLISTS);
    */

    // Integer
    if (childId.docId >= 0) {
        /*
        cout << "before makeKeyValue (in storeInDB)\n";
        for (int z = 0; z < SPOLISTS; z++) {
            cout << z << ":\n";
            for (int y = 0; y < siglists[z].size(); y++) {
    cout << siglists[z][y] << endl;
            }
        }
        */

        makeKeyValue(&dbuf, dLen, siglists, childId.docId);

        /*
        getKeyValue(dbuf, tmpdocid, vlists);
        cout << "after getKeyValue (in storeInDB)\n";
        for (int z = 0; z < SPOLISTS; z++) {
            cout << z << ":\n";
            for (int y = 0; y < vlists[z].size(); y++) {
                cout << vlists[z][y] << endl;
            }
        }
        */

    } else if (childId.nodeId) {
        makeKeyValue(&dbuf, dLen, siglists, childId.nodeId);
    }

    Dbt *data = new Dbt(dbuf, dLen);
        
    int ret = myDB->put(NULL, key, data, DB_APPEND);

    if (ret != 0) {
        std::cerr << "Problem inserting record" << std::endl;
        cleanup(dbuf);
        delete key;
        delete data;
        return false;
    }
        
    //std::cout << "Record id: " << *((db_recno_t *) key->get_data()) << std::endl;
    cleanup(dbuf);

    delete key;
    delete data;

    return true;
}

// Convert a bit string into a number
char *getNumber(std::string& b)
{
    static char number[128];
        
    int value = 0;

    for (int i = 0; i < (int) b.length(); i++) {
        std::cout << "I: " << b.at(i) << std::endl;
        if (b.at(i) == '1') {
            value |= 0x1;
        }
        value = value << 1;
    }

    std::cout << "NUMBER: " << value << std::endl;
    sprintf(number, "%d", value);
    return number;
}

// sig v2
bool constructUnionOptCC(const char *dbName, DbEnv& myEnv, unordered_map<string, int>& idList)
{
    char *myBuf = new char[MAXSIGLEN];
    Db* rootDB = new Db(&myEnv, 0);
    rootDB->set_pagesize(PAGE_SIZE);
        
    try {
        rootDB->open(NULL, dbName, RNODE, DB_RECNO, DB_CREATE, 0);
    } catch(DbException &e) {
        cerr << "Failed to open DB object (1)" << e.what() << endl;
        return EXIT_FAILURE;
    }

    int count = 1;
                
    unordered_map<string, int>::iterator itr;
    for (itr = idList.begin(); itr != idList.end(); itr++) {
        Db* myDB = new Db(&myEnv, 0);
        myDB->set_pagesize(PAGE_SIZE);
                                
        try {
            //cerr << "OPEN: " << itr->first.c_str() << "\n";
            myDB->open(NULL, dbName, itr->first.c_str(), DB_RECNO, DB_RDONLY, 0);
        } catch(DbException &e) {
            cerr << "Failed to open DB object (1)" << e.what() << endl;
            return EXIT_FAILURE;
        }

        std::vector<std::vector<POLY> > lcmSig;
        lcmSig.clear();
        lcmSig.resize(SPOLISTS);

        //std::vector<POLY> unionSig;
        //unionSig.clear();

        string graphIDFile = idxdir + "/" + string(GRAPHIDSDIR) + "/" + itr->first;

#ifdef _NOOPT_
        std::vector<std::vector<std::vector<POLY> > > listOfSigs;
#else
        std::vector<std::vector<POLY> > listOfSigs_0;
        std::vector<std::vector<POLY> > listOfSigs_1;
        std::vector<std::vector<POLY> > listOfSigs_2;
        std::vector<std::vector<POLY> > listOfSigs_3;
        std::vector<std::vector<POLY> > listOfSigs_4;
        std::vector<std::vector<POLY> > listOfSigs_5;
        std::vector<std::vector<POLY> > listOfSigs_6;
#endif 

#ifdef _NOOPT_
        listOfSigs.clear();
#else
        listOfSigs_0.clear();
        listOfSigs_1.clear();
        listOfSigs_2.clear();
        listOfSigs_3.clear();
        listOfSigs_4.clear();
        listOfSigs_5.clear();
        listOfSigs_6.clear();
#endif

#ifdef _NOOPT_
        std::vector<std::vector<std::vector<POLY> > > partialUnionOfSigs;
#else
        std::vector<std::vector<POLY> > partialUnionOfSigs_0;
        std::vector<std::vector<POLY> > partialUnionOfSigs_1;
        std::vector<std::vector<POLY> > partialUnionOfSigs_2;
        std::vector<std::vector<POLY> > partialUnionOfSigs_3;
        std::vector<std::vector<POLY> > partialUnionOfSigs_4;
        std::vector<std::vector<POLY> > partialUnionOfSigs_5;
        std::vector<std::vector<POLY> > partialUnionOfSigs_6;
#endif

#ifdef _NOOPT_
        partialUnionOfSigs.clear();
#else
        partialUnionOfSigs_0.clear();
        partialUnionOfSigs_1.clear();
        partialUnionOfSigs_2.clear();
        partialUnionOfSigs_3.clear();
        partialUnionOfSigs_4.clear();
        partialUnionOfSigs_5.clear();
        partialUnionOfSigs_6.clear();
#endif

        FILE *graphfp = fopen(graphIDFile.c_str(), "w");
        assert(graphfp);

        // Read each entry and gather the list of signatures
        for (int r = 1; r <= itr->second; r++) {
            Dbt key;
            Dbt data(myBuf, MAXSIGLEN);
            db_recno_t recid;
                        
            key.set_flags(DB_DBT_USERMEM);
            key.set_data(&recid);
            key.set_ulen(sizeof(recid));
                        
            recid = r;
                        
            // TODO: catch exception
            if (myDB->get(NULL, &key, &data, 0) != 0) {
                cerr << "Unable to get signature for : " << recid << endl;
                return EXIT_FAILURE;
            }
                        
            //std::cout << "Recid fetched: " << recid << std::endl;
            std::vector<std::vector<POLY> > vlists;
            vlists.clear();
            vlists.resize(SPOLISTS);
                        
            int myDocid;
                        
            getKeyValue((const char *) data.get_data(), myDocid, vlists); 
            fprintf(graphfp, "%d\n", myDocid);
            //lcmNew(unionSig, v);

            /*
            cout << "after getKeyValue (in constructUnionOpt)\n";
            for (int z = 0; z < SPOLISTS; z++) {
                cout << list2ext[z] << ":\n";
                for (int y = 0; y < vlists[z].size(); y++) {
                    cout << vlists[z][y] << endl;
                }
            }
            */

#ifdef _NOOPT_
            listOfSigs.push_back(vlists);
#else
            listOfSigs_0.push_back(vlists[0]);
            listOfSigs_1.push_back(vlists[1]);
            listOfSigs_2.push_back(vlists[2]);
            listOfSigs_3.push_back(vlists[3]);
            listOfSigs_4.push_back(vlists[4]);
            listOfSigs_5.push_back(vlists[5]);
            listOfSigs_6.push_back(vlists[6]);
#endif

            // Compute union over groups of signatures

#ifdef _NOOPT_
            if (listOfSigs.size() == MAX_NUMSIGS_UNION || r == itr->second) {
                std::vector<std::vector<POLY> > e;
                partialUnionOfSigs.push_back(e);
#else
            if (listOfSigs_0.size() == MAX_NUMSIGS_UNION || r == itr->second) {
                std::vector<POLY> e;
                partialUnionOfSigs_0.push_back(e);
                partialUnionOfSigs_1.push_back(e);  
                partialUnionOfSigs_2.push_back(e);  
                partialUnionOfSigs_3.push_back(e);  
                partialUnionOfSigs_4.push_back(e);
                partialUnionOfSigs_5.push_back(e);
                partialUnionOfSigs_6.push_back(e);
#endif
                double beginTime = getgtod();

#ifdef _NOOPT_
                lcmGroup(partialUnionOfSigs[partialUnionOfSigs.size()-1], listOfSigs);
#else
                cout << "OPT code" << endl;
                lcmGroup(partialUnionOfSigs_0[partialUnionOfSigs_0.size()-1], listOfSigs_0);
                lcmGroup(partialUnionOfSigs_1[partialUnionOfSigs_1.size()-1], listOfSigs_1);
                lcmGroup(partialUnionOfSigs_2[partialUnionOfSigs_2.size()-1], listOfSigs_2);
                lcmGroup(partialUnionOfSigs_3[partialUnionOfSigs_3.size()-1], listOfSigs_3);
                lcmGroup(partialUnionOfSigs_4[partialUnionOfSigs_4.size()-1], listOfSigs_4);
                lcmGroup(partialUnionOfSigs_5[partialUnionOfSigs_5.size()-1], listOfSigs_5);
                lcmGroup(partialUnionOfSigs_6[partialUnionOfSigs_6.size()-1], listOfSigs_6);
#endif
                double endTime = getgtod();
                cout << "Partial union time: " << endTime - beginTime << endl;

#ifdef _NOOPT_
                listOfSigs.clear();
#else
                listOfSigs_0.clear();
                listOfSigs_1.clear();
                listOfSigs_2.clear();
                listOfSigs_3.clear();
                listOfSigs_4.clear();
                listOfSigs_5.clear();
                listOfSigs_6.clear();
#endif
            }
        }

        fclose(graphfp);

        double startlcmTime = getgtod();
        // Optimized union 
        //lcmGroup(lcmSig, listOfSigs);

#ifdef _NOOPT_
        lcmGroup(lcmSig, partialUnionOfSigs);
#else
        lcmGroup(lcmSig[0], partialUnionOfSigs_0);
        lcmGroup(lcmSig[1], partialUnionOfSigs_1);
        lcmGroup(lcmSig[2], partialUnionOfSigs_2);
        lcmGroup(lcmSig[3], partialUnionOfSigs_3);
        lcmGroup(lcmSig[4], partialUnionOfSigs_4);
        lcmGroup(lcmSig[5], partialUnionOfSigs_5);
        lcmGroup(lcmSig[6], partialUnionOfSigs_6);
#endif
        double endlcmTime = getgtod();
        std::cout << "lcmGroup time: " << endlcmTime - startlcmTime << "\n";
        
        //cout << "OLD len: " << unionSig.size() << " NEW len: " << lcmSig.size() << endl;
        //assert(lcmSig == unionSig);
        
        // Now write the union to files

        for (int k = 0; k < SPOLISTS; k++) {
            std::cout << k << ") writing union: " << lcmSig[k].size() << " items" << std::endl;
            string unionsigsFile = idxdir + "/" + string(UNIONSIGSDIR) + "/" + itr->first + "." + list2ext[k];
            FILE *unionfp = fopen(unionsigsFile.c_str(), "w");

            // Write the number of items
            int itemCount = (int) lcmSig[k].size();
            fwrite(&itemCount, sizeof(int), 1, unionfp);

            //cout << list2ext[k] << " list:\n";
            for (int i = 0; i < (int) lcmSig[k].size(); i++) {
                fwrite(&lcmSig[k][i], sizeof(POLY), 1, unionfp);
                //cout << lcmSig[k][i] << endl;
            }
            
            fclose(unionfp);
        }
        count++;
        myDB->close(0);
        delete myDB;
    }

    delete[] myBuf;
    rootDB->close(0);
    delete rootDB;
        
    return true;
}

// Construct one level
// sig v2
bool constructUnionOpt(const char *dbName, DbEnv& myEnv, std::map<std::string, int>& idList)
{
    char *myBuf = new char[MAXSIGLEN];

    Db* rootDB = new Db(&myEnv, 0);
    rootDB->set_pagesize(PAGE_SIZE);
        
    try {
        rootDB->open(NULL, dbName, RNODE, DB_RECNO, DB_CREATE, 0);
    } catch(DbException &e) {
        cerr << "Failed to open DB object (1)" << e.what() << endl;
        return EXIT_FAILURE;
    }

    int count = 1;
                
    for (std::map<std::string, int>::iterator itr = idList.begin(); itr != idList.end();
             itr++) {
                
        Db* myDB = new Db(&myEnv, 0);
        myDB->set_pagesize(PAGE_SIZE);
                                
        try {
            //cerr << "OPEN: " << itr->first.c_str() << "\n";
            myDB->open(NULL, dbName, itr->first.c_str(), DB_RECNO, DB_RDONLY, 0);
        } catch(DbException &e) {
            cerr << "Failed to open DB object (1)" << e.what() << endl;
            return EXIT_FAILURE;
        }

        std::vector<std::vector<POLY> > lcmSig;
        lcmSig.clear();
        lcmSig.resize(SPOLISTS);

        //std::vector<POLY> unionSig;
        //unionSig.clear();

        string graphIDFile = idxdir + "/" + string(GRAPHIDSDIR) + "/" + itr->first;

#ifdef _NOOPT_
        std::vector<std::vector<std::vector<POLY> > > listOfSigs;
#else
        std::vector<std::vector<POLY> > listOfSigs_0;
        std::vector<std::vector<POLY> > listOfSigs_1;
        std::vector<std::vector<POLY> > listOfSigs_2;
        std::vector<std::vector<POLY> > listOfSigs_3;
        std::vector<std::vector<POLY> > listOfSigs_4;
        std::vector<std::vector<POLY> > listOfSigs_5;
        std::vector<std::vector<POLY> > listOfSigs_6;
#endif 

#ifdef _NOOPT_
        listOfSigs.clear();
#else
        listOfSigs_0.clear();
        listOfSigs_1.clear();
        listOfSigs_2.clear();
        listOfSigs_3.clear();
        listOfSigs_4.clear();
        listOfSigs_5.clear();
        listOfSigs_6.clear();
#endif

#ifdef _NOOPT_
        std::vector<std::vector<std::vector<POLY> > > partialUnionOfSigs;
#else
        std::vector<std::vector<POLY> > partialUnionOfSigs_0;
        std::vector<std::vector<POLY> > partialUnionOfSigs_1;
        std::vector<std::vector<POLY> > partialUnionOfSigs_2;
        std::vector<std::vector<POLY> > partialUnionOfSigs_3;
        std::vector<std::vector<POLY> > partialUnionOfSigs_4;
        std::vector<std::vector<POLY> > partialUnionOfSigs_5;
        std::vector<std::vector<POLY> > partialUnionOfSigs_6;
#endif

#ifdef _NOOPT_
        partialUnionOfSigs.clear();
#else
        partialUnionOfSigs_0.clear();
        partialUnionOfSigs_1.clear();
        partialUnionOfSigs_2.clear();
        partialUnionOfSigs_3.clear();
        partialUnionOfSigs_4.clear();
        partialUnionOfSigs_5.clear();
        partialUnionOfSigs_6.clear();
#endif

        FILE *graphfp = fopen(graphIDFile.c_str(), "w");
        assert(graphfp);

        // Read each entry and gather the list of signatures
        for (int r = 1; r <= itr->second; r++) {
                
            Dbt key;
            Dbt data(myBuf, MAXSIGLEN);
            db_recno_t recid;
                        
            key.set_flags(DB_DBT_USERMEM);
            key.set_data(&recid);
            key.set_ulen(sizeof(recid));
                        
            recid = r;
                        
            // TODO: catch exception
            if (myDB->get(NULL, &key, &data, 0) != 0) {
                cerr << "Unable to get signature for : " << recid << endl;
                return EXIT_FAILURE;
            }
                        
            //std::cout << "Recid fetched: " << recid << std::endl;
            std::vector<std::vector<POLY> > vlists;
            vlists.clear();
            vlists.resize(SPOLISTS);
                        
            int myDocid;
                        
            getKeyValue((const char *) data.get_data(), myDocid, vlists); 
            fprintf(graphfp, "%d\n", myDocid);
            //lcmNew(unionSig, v);

            /*
            cout << "after getKeyValue (in constructUnionOpt)\n";
            for (int z = 0; z < SPOLISTS; z++) {
                cout << list2ext[z] << ":\n";
                for (int y = 0; y < vlists[z].size(); y++) {
                    cout << vlists[z][y] << endl;
                }
            }
            */

#ifdef _NOOPT_
            listOfSigs.push_back(vlists);
#else
            listOfSigs_0.push_back(vlists[0]);
            listOfSigs_1.push_back(vlists[1]);
            listOfSigs_2.push_back(vlists[2]);
            listOfSigs_3.push_back(vlists[3]);
            listOfSigs_4.push_back(vlists[4]);
            listOfSigs_5.push_back(vlists[5]);
            listOfSigs_6.push_back(vlists[6]);
#endif

            // Compute union over groups of signatures

#ifdef _NOOPT_
            if (listOfSigs.size() == MAX_NUMSIGS_UNION || r == itr->second) {
                std::vector<std::vector<POLY> > e;
                partialUnionOfSigs.push_back(e);
#else
            if (listOfSigs_0.size() == MAX_NUMSIGS_UNION || r == itr->second) {
                std::vector<POLY> e;
                partialUnionOfSigs_0.push_back(e);
                partialUnionOfSigs_1.push_back(e);  
                partialUnionOfSigs_2.push_back(e);  
                partialUnionOfSigs_3.push_back(e);  
                partialUnionOfSigs_4.push_back(e);
                partialUnionOfSigs_5.push_back(e);
                partialUnionOfSigs_6.push_back(e);
#endif
                double beginTime = getgtod();

#ifdef _NOOPT_
                lcmGroup(partialUnionOfSigs[partialUnionOfSigs.size()-1], listOfSigs);
#else
                cout << "OPT code" << endl;
                lcmGroup(partialUnionOfSigs_0[partialUnionOfSigs_0.size()-1], listOfSigs_0);
                lcmGroup(partialUnionOfSigs_1[partialUnionOfSigs_1.size()-1], listOfSigs_1);
                lcmGroup(partialUnionOfSigs_2[partialUnionOfSigs_2.size()-1], listOfSigs_2);
                lcmGroup(partialUnionOfSigs_3[partialUnionOfSigs_3.size()-1], listOfSigs_3);
                lcmGroup(partialUnionOfSigs_4[partialUnionOfSigs_4.size()-1], listOfSigs_4);
                lcmGroup(partialUnionOfSigs_5[partialUnionOfSigs_5.size()-1], listOfSigs_5);
                lcmGroup(partialUnionOfSigs_6[partialUnionOfSigs_6.size()-1], listOfSigs_6);
#endif
                double endTime = getgtod();
                cout << "Partial union time: " << endTime - beginTime << endl;

#ifdef _NOOPT_
                listOfSigs.clear();
#else
                listOfSigs_0.clear();
                listOfSigs_1.clear();
                listOfSigs_2.clear();
                listOfSigs_3.clear();
                listOfSigs_4.clear();
                listOfSigs_5.clear();
                listOfSigs_6.clear();
#endif
            }
        }

        fclose(graphfp);

        double startlcmTime = getgtod();
        // Optimized union 
        //lcmGroup(lcmSig, listOfSigs);

#ifdef _NOOPT_
        lcmGroup(lcmSig, partialUnionOfSigs);
#else
        lcmGroup(lcmSig[0], partialUnionOfSigs_0);
        lcmGroup(lcmSig[1], partialUnionOfSigs_1);
        lcmGroup(lcmSig[2], partialUnionOfSigs_2);
        lcmGroup(lcmSig[3], partialUnionOfSigs_3);
        lcmGroup(lcmSig[4], partialUnionOfSigs_4);
        lcmGroup(lcmSig[5], partialUnionOfSigs_5);
        lcmGroup(lcmSig[6], partialUnionOfSigs_6);
#endif
        double endlcmTime = getgtod();
        std::cout << "lcmGroup time: " << endlcmTime - startlcmTime << "\n";
        
        //cout << "OLD len: " << unionSig.size() << " NEW len: " << lcmSig.size() << endl;
        //assert(lcmSig == unionSig);
        
        // Now write the union to files

        for (int k = 0; k < SPOLISTS; k++) {
            std::cout << k << ") writing union: " << lcmSig[k].size() << " items" << std::endl;
            string unionsigsFile = idxdir + "/" + string(UNIONSIGSDIR) + "/" + itr->first + "." + list2ext[k];
            FILE *unionfp = fopen(unionsigsFile.c_str(), "w");

            // Write the number of items
            int itemCount = (int) lcmSig[k].size();
            fwrite(&itemCount, sizeof(int), 1, unionfp);

            //cout << list2ext[k] << " list:\n";
            for (int i = 0; i < (int) lcmSig[k].size(); i++) {
                fwrite(&lcmSig[k][i], sizeof(POLY), 1, unionfp);
                //cout << lcmSig[k][i] << endl;
            }
            
            fclose(unionfp);
        }
        count++;
        myDB->close(0);
        delete myDB;
    }

    delete[] myBuf;
    rootDB->close(0);
    delete rootDB;
        
    return true;
}

// Construct one level
bool constructUnion(const char *dbName, DbEnv& myEnv, std::map<std::string, int>& idList)
{
    char *myBuf = new char[MAXSIGLEN];

    Db* rootDB = new Db(&myEnv, 0);
    rootDB->set_pagesize(PAGE_SIZE);
        
    try {
        rootDB->open(NULL, dbName, RNODE, DB_RECNO, DB_CREATE, 0);
    } catch(DbException &e) {
        cerr << "Failed to open DB object (1)" << e.what() << endl;
        return EXIT_FAILURE;
    }

    int count = 1;
                
    for (std::map<std::string, int>::iterator itr = idList.begin(); itr != idList.end();
             itr++) {
                
        Db* myDB = new Db(&myEnv, 0);
        myDB->set_pagesize(PAGE_SIZE);
                                
        try {
            //cerr << "OPEN: " << itr->first.c_str() << "\n";
            myDB->open(NULL, dbName, itr->first.c_str(), DB_RECNO, DB_RDONLY, 0);
        } catch(DbException &e) {
            cerr << "Failed to open DB object (1)" << e.what() << endl;
            return EXIT_FAILURE;
        }

        std::vector<std::vector<POLY> > lcmSig;
        lcmSig.clear();

        string graphIDFile = idxdir + "/" + string(GRAPHIDSDIR) + "/" + itr->first;

        FILE *graphfp = fopen(graphIDFile.c_str(), "w");
        assert(graphfp);

        // Read each entry and compute its LCM
        for (int r = 1; r <= itr->second; r++) {
                
            Dbt key;
            Dbt data(myBuf, MAXSIGLEN);
            db_recno_t recid;
                        
            key.set_flags(DB_DBT_USERMEM);
            key.set_data(&recid);
            key.set_ulen(sizeof(recid));
                        
            recid = r;
                        
            // TODO: catch exception
            if (myDB->get(NULL, &key, &data, 0) != 0) {
                cerr << "Unable to get signature for : " << recid << endl;
                return EXIT_FAILURE;
            }
                        
            //std::cout << "Recid fetched: " << recid << std::endl;
            std::vector<std::vector<POLY> > vlists;
            vlists.clear();
                        
            int myDocid;
                        
            getKeyValue((const char *) data.get_data(), myDocid, vlists); 
            fprintf(graphfp, "%d\n", myDocid);
            lcmNew(lcmSig, vlists);
        }

        fclose(graphfp);

        // Now write the union to files
        string unionsigsFile = idxdir + "/" + string(UNIONSIGSDIR) + "/" + itr->first;
        FILE *unionfp = fopen(unionsigsFile.c_str(), "w");
        std::cout << "Writing union: " << lcmSig[0].size() << " items" << std::endl;

        // Write the number of items
        int itemCount = (int) lcmSig.size();
        fwrite(&itemCount, sizeof(int), 1, unionfp);

        for (int i = 0; i < (int) lcmSig.size(); i++) {
            fwrite(&lcmSig[i], sizeof(POLY), 1, unionfp);
        }
        fclose(unionfp);
        count++;
        myDB->close(0);
        delete myDB;
    }
    delete[] myBuf;
    rootDB->close(0);
    delete rootDB;
        
    return true;
}

// Partition hopefully IO optimized
void partitionO(const char *dbName, DbEnv& myEnv, Db* myDB, std::multimap<int, int>& sortedList, int fanout, std::string& myGroupID)
{
    std::cout << "SIZE: " << sortedList.size() << " GROUPID: " << myGroupID << std::endl;

    std::multimap<int, int> group1, group2;

    std::vector<std::vector<std::vector<POLY> > > newsigList;
    std::vector<int> recidList;
    std::vector<int> docidList;

    int maxSigsReqd = (int) (0.01 * sortedList.size());
    if (maxSigsReqd < 2) maxSigsReqd = 2;
    if (maxSigsReqd > seedPickSize) maxSigsReqd = seedPickSize;
        
    std::cout << "Max reqd sigs: " << maxSigsReqd << std::endl;

    //std::multimap<int, int>::reverse_iterator ritr;
    std::multimap<int, int>::iterator itr;

    std::string group1ID = myGroupID + std::string("0");
    std::string group2ID = myGroupID + std::string("1");
        
    char *myBuf = new char[MAXSIGLEN];
        
    //for (itr = sortedList.rbegin(); itr != sortedList.rend(); itr++)
    for (itr = sortedList.begin(); itr != sortedList.end(); itr++) {
                
        //int len = sizeof(int) + sizeof(int) + 
        //      (int) ceil((double) (ritr->first+1)/8);
                
        //char *myBuf = new char[len]; 
        Dbt key;
        //Dbt data(myBuf, len * sizeof(POLY));
        Dbt data(myBuf, MAXSIGLEN);
        db_recno_t recid;
        
        key.set_flags(DB_DBT_USERMEM);
        key.set_data(&recid);
        key.set_ulen(sizeof(recid));
        
        recid = itr->second;

        // TODO: catch exception
        if (myDB->get(NULL, &key, &data, 0) != 0) {
            cerr << "Unable to get signature for : " << recid << endl;
            exit(2);
        }

        std::vector<std::vector<POLY> > vlists;
        vlists.clear();
        vlists.resize(SPOLISTS);

        int myDocid;
                
        getKeyValue((const char *) data.get_data(), myDocid, vlists);
        docidList.push_back(myDocid);

        /*
        cout << "after getKeyValue (in partitionO)\n";
        for (int z = 0; z < SPOLISTS; z++) {
            cout << z << ":\n";
            for (int y = 0; y < vlists[z].size(); y++) {
                cout << vlists[z][y] << endl;
            }
        }
        */
     
        newsigList.push_back(vlists);
        recidList.push_back(itr->second);

        //delete[] myBuf;

        maxSigsReqd--;
        if (maxSigsReqd == 0) {
            // IMPORTANT: Set itr to the next record, otherwise we see
            // duplicates graph ids in the index
            itr++;
            break;
        }
    }

    int seed1 = -1, seed2 = -1;
    std::vector<std::vector<POLY> > gcdPoly;
    int minDeg = INT_MAX;

    double startGroupingTime = getgtod();
        
    // Pick seeds
    for (int i = 0; i < (int) newsigList.size()-1; i++) {
        for (int j = i+1; j < (int) newsigList.size(); j++) {
            gcdPoly.clear();
            //assert(newsigList[i][0].size());
            //assert(newsigList[j][0].size());
            gcdNew(gcdPoly, newsigList[i], newsigList[j]);
            
            int deg = pSimOptNew(newsigList[i], newsigList[j], gcdPoly, true);
            if (deg < minDeg) {
                seed1 = i;
                seed2 = j;
                minDeg = deg;
            }
        }
    }

    // TODO: Cache previously computed pSimOpt for pickseeds
        
    std::cout << "SEED1: " << seed1 << " SEED2: " << seed2 <<
        " min deg: " << minDeg << std::endl;

    // Need to store them in their DBs
    // WARNING: donot use RECID 0 for BDB -- fails
    int group1Count = 1, group2Count = 1;

    group1Count = recidList[seed1];
    group1.insert(std::pair<int, int>(getDegreeNew(newsigList[seed1]), group1Count));

    group2Count = recidList[seed2];
    group2.insert(std::pair<int, int>(getDegreeNew(newsigList[seed2]), group2Count));
        
    std::vector<std::vector<POLY> > gcd1, gcd2;
        
    for (int i = 0; i < (int) newsigList.size(); i++) {

        if (i == seed1 || i == seed2) continue;
                
        gcd1.clear();
        gcd2.clear();
        //assert(newsigList[i][0].size());
        //assert(newsigList[seed1][0].size());
        //assert(newsigList[seed2][0].size());
        gcdNew(gcd1, newsigList[seed1], newsigList[i]);
        gcdNew(gcd2, newsigList[seed2], newsigList[i]);

        int deg1 = pSimOptNew(newsigList[seed1], newsigList[i], gcd1, true);
        int deg2 = pSimOptNew(newsigList[seed2], newsigList[i], gcd2, true);

        if (deg1 > deg2) {
            group1Count = recidList[i];
            group1.insert(std::pair<int, int>(getDegreeNew(newsigList[i]), group1Count));
        } else if (deg1 < deg2) {
            group2Count = recidList[i];
            group2.insert(std::pair<int, int>(getDegreeNew(newsigList[i]), group2Count));
        } else {
            if (group1.size() <= group2.size()) {
                group1Count = recidList[i];
                group1.insert(std::pair<int, int>(getDegreeNew(newsigList[i]), group1Count));
            } else {
                group2Count = recidList[i];
                group2.insert(std::pair<int, int>(getDegreeNew(newsigList[i]), group2Count));
            }
        }
    }

    // Now put the remaining entries into group1 or group2
    std::cout << "Testing..." << std::endl;
    //for ( ; ritr != sortedList.rend(); ritr++)
    for ( ; itr != sortedList.end(); itr++) {
        Dbt key;
        Dbt data(myBuf, MAXSIGLEN);
        db_recno_t recid;
                        
        key.set_flags(DB_DBT_USERMEM);
        key.set_data(&recid);
        key.set_ulen(sizeof(recid));
        
        recid = itr->second;
                
        // TODO: catch exception
        if (myDB->get(NULL, &key, &data, 0) != 0) {
            cerr << "Unable to get signature for : " << recid << endl;
            exit(2);
        }

        std::vector<std::vector<POLY> > vlists;
        vlists.clear();
        vlists.resize(SPOLISTS);

        int myDocid;
                
        getKeyValue((const char *) data.get_data(), myDocid, vlists); 

        gcd1.clear();
        gcd2.clear();
        //assert(newsigList[seed1][0].size());
        //assert(newsigList[seed2][0].size());
        //assert(vlists[0].size());
        gcdNew(gcd1, newsigList[seed1], vlists);
        gcdNew(gcd2, newsigList[seed2], vlists);

        //assert(gcd1[0].size());
        //assert(gcd2[0].size());
        int deg1 = pSimOptNew(newsigList[seed1], vlists, gcd1, true);
        int deg2 = pSimOptNew(newsigList[seed2], vlists, gcd2, true);

        if (deg1 > deg2) {
            group1Count = itr->second;
            group1.insert(std::pair<int, int>(itr->first, group1Count));
        } else if (deg1 < deg2) {
            group2Count = itr->second;
            group2.insert(std::pair<int, int>(itr->first, group2Count));
        } else {
            if (group1.size() <= group2.size()) {
                group1Count = itr->second;
                group1.insert(std::pair<int, int>(itr->first, group1Count));
            } else {
                group2Count = itr->second;
                group2.insert(std::pair<int, int>(itr->first, group2Count));
            }
        }
    }

    double endGroupingTime = getgtod();
    std::cout << "Group time: " << endGroupingTime - startGroupingTime << "\n";
    // Should you further recurse
    if ((int) group1.size() > fanout) {
        partitionO(dbName, myEnv, myDB, group1, fanout, group1ID);
    } else {
        idList.insert(std::pair<std::string, int>(group1ID, (int) group1.size()));
        storePartition(dbName, myEnv, myDB, group1ID, group1);
    }
        
    if ((int) group2.size() > fanout) {
        partitionO(dbName, myEnv, myDB, group2, fanout, group2ID);
    } else {
        idList.insert(std::pair<std::string, int>(group2ID, (int) group2.size()));
        storePartition(dbName, myEnv, myDB, group2ID, group2);
    }

    delete[] myBuf;
}

// Materialize the partition
bool storePartition(const char *dbName, DbEnv& myEnv, Db *myDB, std::string& groupName, std::multimap<int, int>& groupData)
{
    Db *groupDB = new Db(&myEnv, 0);

    char *myBuf = new char[MAXSIGLEN];
        
    groupDB->set_pagesize(PAGE_SIZE);
        
    // Create a new db for group1
    try {
        //cerr << "OPEN: " << groupName.c_str() << "\n";
        groupDB->open(NULL, dbName, groupName.c_str(), DB_RECNO, DB_CREATE, 0);
    } catch(DbException &e) {
        cerr << "Failed to open DB object (2)" << e.what() << endl;
        return EXIT_FAILURE;
    }

    // For each item in the group, fetch from the original database and store in the new one.
    for (std::multimap<int, int>::iterator itr = groupData.begin(); itr != groupData.end(); itr++) {
        Dbt key;
        Dbt data(myBuf, MAXSIGLEN);
        db_recno_t recid;
        
        key.set_flags(DB_DBT_USERMEM);
        key.set_data(&recid);
        key.set_ulen(sizeof(recid));
        
        recid = itr->second;

        // TODO: catch exception
        if (myDB->get(NULL, &key, &data, 0) != 0) {
            cerr << "Unable to get signature for : " << recid << endl;
            return EXIT_FAILURE;
        }

        // Put it in the groupDB
        int ret = groupDB->put(NULL, &key, &data, DB_APPEND);

        if (ret != 0) {
            std::cerr << "Problem inserting record..." << std::endl;
            //return false;
        }
    }
        
    delete[] myBuf;

    groupDB->close(0);
    delete groupDB;
        
    return true;
}

string dec2bitstr(int num)
{
    bitset<CCBITS> bt;
    stringstream ss;

    bt = num;
    for (int i = 0; i < bt.size(); i++) {
        if (bt.test(i))
            ss << "1";
        else
            ss << "0";
    }
    return ss.str();
}

bool storeComponent(const char *dbName, DbEnv& myEnv, Db *myDB,
                                        string groupName, vector<int> &gids)
{
    Db *groupDB = new Db(&myEnv, 0);
    char *myBuf = new char[MAXSIGLEN];
    groupDB->set_pagesize(PAGE_SIZE);
        
    // Create a new db for ccname
    try {
        //cerr << "OPEN: " << groupName.c_str() << "\n";
        groupDB->open(NULL, dbName, groupName.c_str(), DB_RECNO, DB_CREATE, 0);
    } catch(DbException &e) {
        cerr << "Failed to open DB object (2)" << e.what() << endl;
        return EXIT_FAILURE;
    }

    // for each gid in the component,
    // fetch from the original database and store in a new one
    for (int i = 0; i < gids.size(); i++) {
        Dbt key;
        Dbt data(myBuf, MAXSIGLEN);
        db_recno_t recid;
        
        key.set_flags(DB_DBT_USERMEM);
        key.set_data(&recid);
        key.set_ulen(sizeof(recid));
        
        // graph id in this component
        recid = gids[i];

        // TODO: catch exception?
        int ret = myDB->get(NULL, &key, &data, 0);
        if (ret != 0) {
            cout << "Error: unable to get signature for recid:"
                     << recid << " at gids[" << i << "]" << endl;
            // TODO: or exit?
            continue;
        }

        // Put it in the groupDB
        ret = groupDB->put(NULL, &key, &data, DB_APPEND);
        if (ret != 0) {
            cout << "Error: problem inserting record..." << endl;
            //return false;
        }
    }

    delete[] myBuf;
    groupDB->close(0);
    delete groupDB;
        
    return true;
}

// Verification process, the set of correct matches...
void verifyProcess(char *fileName, std::vector<std::vector<POLY> >& listOfSigs, int queryid)
{
    int numMatches = 0;
    FILE *sigfp = fopen(fileName, "r");

    std::vector<POLY> oldSig;
    std::vector<POLY> sig;
    
    int loopCount = 0;

    while (1) {
        int numSigs;
        if (fread(&numSigs, sizeof(int), 1, sigfp) != 1) {
            break;
        }
        assert(numSigs == 1);
        
        int size;
        
        if (fread(&size, sizeof(int), 1, sigfp) != 1) {
            assert(0);
        }

        // clear old sig
        sig.clear();
        POLY e;
        for (int i = 0; i < size; i++) {
            if (fread(&e, sizeof(POLY), 1, sigfp) != 1) {
    assert(0);
            }
            sig.push_back(e);
        }

        int docId;
        if (fread(&docId, sizeof(docId), 1, sigfp) != 1) {
            assert(0);
        }

        int dega = getDegree(sig);
        
        for (int j = 0; j < (int) listOfSigs.size(); j++) {
            std::vector<POLY> rem;
            // Test for divisibility!!!
            rem.clear();

            int degb = getDegree(listOfSigs[j]);

            if (dega >= degb) {
    //remainder(rem, sig, listOfSigs[j]);
    //double beginTime = getgtod();
    remainder(rem, sig, listOfSigs[j]);
    //double endTime = getgtod();
    //std::cerr << "REM time: " << endTime - beginTime << "\n";
            }

            if (rem.size() == 1 && rem[0] == (POLY) 0) {
    //cerr << "==== Found match ==== DOCID: " << docId << "\n";
    //cerr << docId << "\n";
    std::map<int, std::vector<int> >::iterator titr;
    titr = docMatchList.find(loopCount);
                
    if (titr != docMatchList.end()) {
        titr->second.push_back(queryid);
                    
    }
    else {
        std::vector<int> e;
        e.push_back(queryid);
        docMatchList[loopCount] = e;
    }
                
    numMatches++;
    break;
            }
        }

        // If any document if not matched, just add an empty entry
        if (docMatchList.find(loopCount) == docMatchList.end()) {
            std::vector<int> e;
            docMatchList[loopCount] = e;
        }
        
        loopCount++;
    }

    fclose(sigfp);
}

// Computes the similarity between two signatures
// sig v1
int pSimOptNew(std::vector<POLY>& s1, std::vector<POLY>& s2,
                 std::vector<POLY>& mygcd, bool isMetric)
{
    int retVal;
    if (isMetric) {
        std::vector<POLY> mylcm;
        lcmNew(mylcm, s1, s2);
        double val = (double) (INT_MAX-1) * getDegreeNew(mygcd) / getDegreeNew(mylcm);
        retVal = (int) val;
    }
    else {
        retVal = getDegreeNew(mygcd);
    }
    return retVal;
}

// Computes the similarity between two signatures
// sig v2
int pSimOptNew(std::vector<std::vector<POLY> >& s1, std::vector<std::vector<POLY> >& s2, std::vector<std::vector<POLY> >& mygcd, bool isMetric)
{
    int retVal;
    double val = 0;
    if (isMetric) {
        std::vector<std::vector<POLY> > mylcm;
        //assert(s1[0].size());
        //assert(s2[0].size());
        lcmNew(mylcm, s1, s2);
        for (int i = 0; i < SPOLISTS; i++) {
            assert(mylcm[i].size());
            int gcddeg = getDegreeNew(mygcd[i]);
            int lcmdeg = getDegreeNew(mylcm[i]);
            //assert(gcddeg);
            assert(lcmdeg);
            val += (double) gcddeg / lcmdeg;
            //val += (double) getDegreeNew(mygcd[i]) / getDegreeNew(mylcm[i]);
        }
        //cout << "val: " << val << endl;
        retVal = val / SPOLISTS * (INT_MAX-1);
    }
    else {
        retVal = getDegreeNew(mygcd);
    }
    //cout << "retVal: " << retVal << endl;
    return retVal;
}

// Computes the similarity between two signatures
int pSimOpt(std::vector<POLY>& s1, std::vector<POLY>& s2,
            std::vector<POLY>& mygcd, bool isMetric)
{

    int retVal;
    if (isMetric) {
        lcmSpecial(s1, s2, mygcd);
        double val = (double) (INT_MAX-1) * getDegree(mygcd) / getDegree(s1);
        retVal = (int) val;
    }
    else {
        retVal = getDegree(mygcd);
    }
    return retVal;
}

// Computes the similarity between two signatures
int pSim(std::vector<POLY>& s1, std::vector<POLY>& s2,
         std::vector<POLY>& mygcd, bool isMetric)
{
    //double startTime = getgtod();

    int retVal;
    if (isMetric) {
        std::vector<POLY> mylcm;
        mylcm.push_back((POLY) 0x1);
        lcm(mylcm, s1);
        lcm(mylcm, s2);
        double val = (double) (INT_MAX-1) * getDegree(mygcd) / getDegree(mylcm);
        retVal = (int) val;
    }
    else {
        retVal = getDegree(mygcd);
    }
    return retVal;
}

void addhashv(vector<string> minhash)
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

int lshsig(int gid,
           vector<vector<POLY> > siglists,
           vector<bool> pvskiplists,
           hash2ints &lsh2gids)
{
    //vector<string> minhash;
    vector<POLY> minhash;
    double beginTime, endTime, totalBeginTime, totalEndTime;
    int pvsize = 0;

    minhash.clear();
    totalBeginTime = getgtod();
    for (int i = 0; i < SPOLISTS; i++) {
        // count even if skipped
        // (skipping is done for LSH only)
        pvsize += siglists[i].size();
        if (pvskiplists[i]) {
            continue;
        }
        beginTime = getgtod();
        LSH *pv_lsh = new LSH(lshk, lshl, lshp, irrpoly, hasha, hashb);
        //minhash = pv_lsh->get_hash_str(siglists[i]);
        minhash = pv_lsh->get_hash_poly(siglists[i]);
        endTime = getgtod();
        if (verbose) {
            cout << "gid: " << gid << " list: " << i
                 << " LSH time: " << endTime - beginTime << endl;
        }

        for (int i = 0; i < (int)minhash.size(); i++) {
            if (verbose) {
                cout << minhash[i] << endl;
            }
            lsh2gids[minhash[i]].push_back(gid);
        }
        minhash.clear();
        delete pv_lsh;
    }
    totalEndTime = getgtod();
    cout << "gid: " << gid << " total size: " << pvsize
         << " total LSH time: " << totalEndTime - totalBeginTime << endl;

    return pvsize;
}

// TODO: move to utils.C
// http://oopweb.com/CPP/Documents/CPPHOWTO/Volume/C++Programming-HOWTO-7.html
void tokenize(const std::string& str, std::vector<std::string>& tokens,
              const std::string &delimiters = " ")
{
        // Skip delimiters at beginning.
        std::string::size_type lastPos = str.find_first_not_of(delimiters, 0);
        // Find first "non-delimiter".
        std::string::size_type pos = str.find_first_of(delimiters, lastPos);

        while (std::string::npos != pos || std::string::npos != lastPos) {
                // Found a token, add it to the vector.
                tokens.push_back(str.substr(lastPos, pos - lastPos));
                // Skip delimiters.  Note the "not_of"
                lastPos = str.find_first_not_of(delimiters, pos);
                // Find next "non-delimiter"
                pos = str.find_first_of(delimiters, lastPos);
        }
}

// TODO: move to utils.C
// copied from http://stackoverflow.com/a/217605
// trim from start
static inline std::string &ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(),
            std::not1(std::ptr_fun<int, int>(std::isspace))));
    return s;
}

// TODO: move to utils.C
// trim from end
static inline std::string &rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(),
            std::not1(std::ptr_fun<int, int>(std::isspace))).base(),
            s.end());
    return s;
}

// TODO: move to utils.C
// trim from both ends
static inline std::string &trim(std::string &s) {
    return ltrim(rtrim(s));
}

// copy parse tree, don't want to modify original parse tree
bool prune_tree(Node tree, const vector<bool> &eval, vector<Node *> &bgps2del)
{
    // don't use outside of helper
    node_itr tree_itr;
    prune_tree_helper(tree, tree_itr, eval, bgps2del);

    cout << "printing pruned parse tree..." << endl;
    print_tree(tree);
    cout << endl;

    return true;
}

bool prune_tree_helper(Node &tree,
                       node_itr tree_itr,
                       const vector<bool> &eval,
                       vector<Node *> &bgps2del)
{
    bool eval_val = false;

    if (tree.data.empty()) {
        cout << "processing root" << " [" << tree.id << "]" << endl;
    } else {
        cout << "processing " << tree.data << " [" << tree.id << "]" << endl;
        try {
            eval_val = eval.at(tree.id);
        } catch (const std::out_of_range& e) {
            cout << "Exception occurred checking tree.id: "
                     << tree.id << " " << e.what() << endl;
        }
    }

    // not root and false
    if (!tree.data.empty() && eval_val == false) {
        // TODO: FILTER is not supported so this not tested
        // check for NULL!
        if (tree.parent && tree.parent->data == "notExistsFunction") {
            return true;
        } else if (tree.data == "optionalGraphPattern OPTIONAL") {
            return false;
        // TODO: revise
        } else if (tree.data == "groupOrUnionGraphPattern UNION") {
            cout << "prune_tree_helper: deleting in UNION" << endl;
            del_subtree(tree, bgps2del);
        }

        cout << "prune_tree_helper: deleting False subtree" << endl;
        del_subtree(tree, bgps2del);
    } else {
        for (node_itr itr = tree.children.begin();
             itr != tree.children.end();
             ++itr) {

            Node &child = *itr;
            bool status = prune_tree_helper(child, itr, eval, bgps2del);
            if (status == false) {
                cout << "prune_tree_helper: deleting False leaf" << endl;
                del_subtree(tree, bgps2del);
            }
        }
    }

    return true;
}

bool del_subtree(Node &tree, vector<Node *> &bgps2del)
{
    // handle children
    while (!tree.children.empty()) {
        Node &child = tree.children.front();
        del_subtree(child, bgps2del);

        cout << "del_subtree: deleting child: "
             << tree.children.front().id << endl;
        tree.children.pop_front();

        bgps2del.push_back(&child);

        // TODO: free memory?
        // children are not ptrs but are allocated with new
        //delete child;
    }

    bgps2del.push_back(&tree);

    // handle root of subtree to be removed
    Node &parent = *(tree.parent);
    // XXX: ugly
    for (node_itr itr = parent.children.begin();
         itr != parent.children.end();) {

        Node &child = *itr;
        if (child.id == tree.id) {
            cout << "del_subtree: deleting root: " << child.id << endl;
            itr = parent.children.erase(itr);
            //break;
        } else {
            ++itr;
        }
    }

    return true;
}

// WTF is Rasqal removing the { b/w WHERE and GRAPH?
string fix_query_str(unsigned char *query_str_p)
{
    string query_str = (char *)query_str_p;

    // TODO: better way?
    size_t found = query_str.find("GRAPH");
    if (found != std::string::npos) {
        query_str.insert(found, "{\n");
        query_str.append("}");
    } else {
        cout << "GRAPH not found" << endl;
        query_str = "";
    }

    return query_str;
}

bool check_bgp(const querypolystrType &polystrbgps,
               const queryorderType &spo_order,
               int gid,
               int bgpid)
{
    cout << "checking bgp # " << bgpid
         << " on graphid " << graphids[gid] << endl;

    vector<string> candidates;
    bool ret = queryBloomCounters(polystrbgps, spo_order, candidates,
                                  gid, bgpid);
    if (!ret || candidates.empty())
        return false;

    return true;
}

// TODO: use .at() instead of []
bool eval_tree(const Node &tree,
               vector<bool> &eval,
               querypolystrType &polystrbgps,
               queryorderType &spo_order,
               int gid)
{
    for (node_citr itr = tree.children.begin();
         itr != tree.children.end();
         ++itr) {

        const Node &child = *itr;
        eval[child.id] = eval_tree(child, eval, polystrbgps, spo_order, gid);

        // using node names from SparqlParser.g4 grammar
        if (tree.data == "groupGraphPattern" && eval[child.id] == false) {
            eval[tree.id] = false;
            // skip rest of the nodes
            return false;
        }
    }

    if (tree.data == "groupOrUnionGraphPattern") {
        Disjunction d = for_each(eval.begin(), eval.end(), Disjunction());
        eval[tree.id] = d.val;
    // TODO: not supported
    } else if (tree.data == "existsFunction") {
        eval[tree.id] = eval[tree.children.front().id];
    // TODO: not supported
    } else if (tree.data == "notExistsFunction") {
        eval[tree.id] = true;
    // TODO: not supported
    // skip processing predicates
    } else if (tree.data == "expression") {
        eval[tree.id] = true;
    } else if (tree.data == "triplesBlock") {
        eval[tree.id] = check_bgp(polystrbgps, spo_order, gid, tree.id);
    // non-leaf nodes
    } else if (!tree.children.empty()) {
        eval[tree.id] = eval[tree.children.back().id];
    }
    // leaf nodes
    //} else {
    //    eval[tree.id] = true;
    //}

    if (tree.data == "optionalGraphPattern OPTIONAL") {
        return true;
    }

    return eval[tree.id];
}

int print_eval_tree(const Node &tree, const vector<bool> &eval)
{
    for (node_citr itr = tree.children.begin();
         itr != tree.children.end();
         ++itr) {

        const Node &child = *itr;
        cout << child.data << ":" << eval[child.id]
                 << " [" << child.id << "]" << endl;
        print_eval_tree(child, eval);
    }

}

void print_tree(const Node &tree, bool print_data, bool print_ids)
{
    // auto init to 0!
    //static int size;

    // TODO: show triples?
    if (tree.children.empty())
        cout << "triples";

    for (node_citr itr = tree.children.begin();
         itr != tree.children.end();
         ++itr) {

        const Node &child = *itr;
        cout << "(";

        if (print_data) {
            cout << child.data << " ";

            if (print_ids) {
                cout << "[id:" << child.id;
                cout << ",pid:";
                if (child.parent)
                    cout << child.parent->id;
                else
                    cout << "NULL";

                cout << "] ";
            }
        }

        print_tree(child, print_data, print_ids);
        cout << ") ";
    }
}

// based on: http://stackoverflow.com/a/12184021
bool build_tree(string &lispstr, Node *tree)
{
    stack<string> vals;
    stack<Node> nodes;
    int nid = 0;

    // root of tree
    Node root;
    root.id = nid;
    nodes.push(root);
    ++nid;

    int i = 0;
    char prev = 0;
    while (i < lispstr.size()) {
        if (lispstr[i] == '(') {
            ++i;

            stringstream ss;
            while (lispstr[i] != '(' &&
                   lispstr[i] != ')') {

                if (lispstr[i] == '{' ||
                    lispstr[i] == '}') {
                    ++i;
                    continue;
                }
                ss << lispstr[i];
                ++i;
            }

            Node n = Node();
            n.id = nid;
            ++nid;
            string s = ss.str();
            n.data = trim(s);
            nodes.push(n);
            // old way of parsing
            //nodes.push(Node());
        } else if (lispstr[i] == ')') {
            // always one empty node
            if (nodes.size() <= 1) {
                cout << "improper nesting" << endl;
                return false;
            }

            Node top = nodes.top();
            nodes.pop();
            /*
            // old way of parsing
            if (!vals.empty()) {
                top.data = trim(vals.top());
                top.id = nid;
                ++nid;
                vals.pop();
            }
            */
            nodes.top().children.push_back(top);
            ++i;
        } else if (isspace(lispstr[i]) ||
                           lispstr[i] == '{' ||
                           lispstr[i] == '}' ||
                           lispstr[i] == '.') {
            ++i;
        // TODO: include these?
        //} else if (lispstr[i] == '{' ||
        //                   lispstr[i] == '.' ||
        //                   lispstr[i] == '}') {
        //  vals.push(string(1, lispstr[i]));
        //  ++i;
        // not a part of new parse tree
        } else {
            stringstream ss;
            // messes up nesting
            //while (!isspace(lispstr[i]) &&
            while (lispstr[i] != '(' &&
                   lispstr[i] != ')') {
                   //lispstr[i] != '{' &&
                   //lispstr[i] != '}') {
                ss << lispstr[i];
                ++i;
            }
            vals.push(ss.str());
        }
        prev = lispstr[i];
    }

    while (!vals.empty()) {
        cout << vals.top() << endl;
        vals.pop();
    }

    // root is empty
    if (nodes.size() > 1) {
        return false;
    }

    *tree = nodes.top();

    tree->data = "root";
    tree->id = nid;
    //cout << tree->data << ", size: " << tree->id << endl;

    return true;
}

// TODO: return success?
void build_graph(hash2ints &lsh2gids, vector<set<int> > &edge_set)
{
    hash2ints::iterator stritr;

    for (stritr = lsh2gids.begin(); stritr != lsh2gids.end(); stritr++) {
        int first = stritr->second[0];
        // skip hashes with no collisions
        if (stritr->second.size() > 1) {
            // don't add itself!
            for (int i = 1; i < stritr->second.size(); i++) {
                edge_set[first].insert(stritr->second[i]);
            }
        }
    }
}

// based on connected components from the Algorithms Design Manual
int connected_components(vector<set<int> > &edge_set,
                         vector<int> &component,
                         vector<int> &gid2size,
                         vector<int> &cc2unionsize)
{
    int ccnum = 0;
    int nvertices = edge_set.size();
    vector<bool> discovered(nvertices);
    vector<bool> processed(nvertices);
    // vertex id 0 is invalid and ignored
    discovered[0] = true;
    processed[0] = true;
    component[0] = -1;
    // TODO: needed?
    vector<int> parent(nvertices);

    int ccsize = 0;
    int unionsize = 0;
    for (int i = 1; i < nvertices; i++) {
        if (discovered[i] == false) {
            //cout << "Component " << ccnum << endl;
            bfs(i, edge_set, discovered, processed, parent, ccnum, ccsize,
                    unionsize, component, gid2size, cc2unionsize);
            //cout << endl;

            // optimization: prevents creating tiny CCs
            if ((ccsize >= MIN_CC_SIZE) || (unionsize >= MAX_CC_UNION_SIZE)) {
                cc2unionsize.push_back(unionsize);
                ++ccnum;
                ccsize = 0;
                unionsize = 0;
            }
        }
    }
    return ccnum;
}

// based on bfs from the Algorithms Design Manual
// assumes an undirected graph
// return size of last CC
int bfs(int start,
        vector<set<int> > &edge_set,
        vector<bool> &discovered,
        vector<bool> &processed,
        vector<int> &parent,
        int &ccnum,
        int &ccsize,
        int &unionsize,
        vector<int> &component,
        vector<int> &gid2size,
        vector<int> &cc2unionsize)
{
    // visited vertices
    queue<int> Q;
    // current vertex
    int v;
    // successor vertex
    int y;
    set<int>::iterator sitr;

    Q.push(start);
    discovered[start] = true;

    while (!Q.empty()) {
        v = Q.front();
        Q.pop();
        process_vertex_early(v, ccnum, component);
        processed[v] = true;
        ++ccsize;
        unionsize += gid2size[v];

        // optimization:
        // CC is too big, start a new one OR
        // union of POLYs is too big
        if ((ccsize >= MAX_CC_SIZE) || (unionsize >= MAX_CC_UNION_SIZE)) {
            cc2unionsize.push_back(unionsize);
            ++ccnum;
            ccsize = 0;
            unionsize = 0;
        }

        // skip vertices with no edges
        if (edge_set[v].empty())
            continue;

        sitr = edge_set[v].begin();
        while (sitr != edge_set[v].end()) {
            y = *sitr;
            if (processed[y] == false)
                process_edge(v, y);
            if (discovered[y] == false) {
                Q.push(y);
                discovered[y] = true;
                parent[y] = v;
            }
            ++sitr;
        }
        process_vertex_late(v);
    }
    return ccsize;
}

void process_vertex_late(int v)
{
}

void process_vertex_early(int v, int ccnum, vector<int> &component)
{
    //cout << "processed vertex " << v << endl;
    //cout << " " << v;
    component[v] = ccnum;
}

void process_edge(int x, int y)
{
    //cout << "processed edge (" << x << " " << y << ")\n";
}

// TODO: handle more queries?
bool sparql2str(string &sparqlfile,
                string &querystr)
{
    ifstream ifssparql(sparqlfile);
    if (!ifssparql.is_open()) {
        cerr << "Error opening SPARQL file" << endl;
        return false;
    }

    string line;
    stringstream ss;
    while (getline(ifssparql, line)) {
        ss << line << "\n";
    }

    querystr = ss.str();

    return true;
}

bool sparql2sig(string &sparqlfile,
                                string &query_string,
                                vector<vector<POLY> >&query_lists)
{
    ifstream ifssparql(sparqlfile);
    if (!ifssparql.is_open()) {
        cout << "Error opening SPARQL file" << endl;
        return false;
    }

    string line;
    stringstream query_ss;
    bool is_query = false;
    int query_num = 0;
    bool uricheck = false;
    const int SPO_BUFF = 1024 * 4;
    while (getline(ifssparql, line)) {
        query_ss << line << "\n";
        stringstream start;
        string st(line);
        start<<st;
        start>>st;

        // comments and blank lines
        if (st.substr(0,1) == "#" || line == "") {
            continue;
        // start of query
        } else if (line.substr(0,1) == "{") {
            is_query = true;
            ++query_num;
            continue;
        // end of query
        } else if (line.substr(0,1) == "}") {
            is_query = false;
        }

        if (is_query) {
            char su[SPO_BUFF],pr[SPO_BUFF],ob[SPO_BUFF];
            sscanf(&line[0],"%s %s %[^\n]s",su,pr,ob);
            string o(ob);
            string p(pr);
            string s(su);
            o = o.substr(0,o.length()-2);
            if (verbose) {
                cout << check_prefix(s);
                cout << check_prefix(p);
                cout << check_prefix(o);
                cout << endl;
            }

            // check prefix (fails if prefix exists in query)
            if (uricheck){
                if (check_prefix(s) || check_prefix(p) || check_prefix(o)) {
                        cout << "Error: Absence of \" or < in the start of literal/URI - Possible prefix."<<endl;
                        string trip_temp = "( "+s+" , "+p+" , "+o+" )";
                        cout << trip_temp << endl;
                        return false;
                }
            }

            s = check_var(s);
            p = check_var(p);
            o = check_var(o);

            string store = "( "+s+" , "+p+" , "+o+" )";

            if (verbose)
                cout << store << endl;

            // verified correctness
            // TODO: use the other one instead?
            POLY irrPoly = lshp;

            POLY pol;
            pol = hashIrrPoly(&store[0], irrPoly);
            if (verbose)
                cout << "Poly: " << pol << endl;

            // ( s, p, o )
            if (s != "?" && p != "?" && o != "?") {
                query_lists[0].push_back(pol);
            // ( s, p, ? )
            } else if (s != "?" && p != "?" && o == "?") {
                query_lists[1].push_back(pol);
            // ( s, ?, o )
            } else if (s != "?" && p == "?" && o != "?") {
                query_lists[2].push_back(pol);
            // ( ?, p, o )
            } else if (s == "?" && p != "?" && o != "?") {
                query_lists[3].push_back(pol);
            // ( s, ?, ? )
            } else if (s != "?" && p == "?" && o == "?") {
                query_lists[4].push_back(pol);
            // ( ?, p, ? )
            } else if (s == "?" && p != "?" && o == "?") {
                query_lists[5].push_back(pol);
            // ( ?, ?, o )
            } else if (s == "?" && p == "?" && o != "?") {
                query_lists[6].push_back(pol);
            } else {
                cout << "invalid pattern: all vars\n";
            }
        }
    }

    query_string = query_ss.str();

    return true;
}

string check_var(const string& var)
{
    if (var.substr(0,1) == "?")
        return "?";
    else
        return var;
}

bool check_prefix(string& str)
{
    if (str[0] == '<' || str[0] == '"' || str[0] == '?')
        return false;
    else
     return true;
}

// alpha-sorted
bool list_files(const string &dir, vector<string> &files)
{
    struct dirent **namelist;

    // namelist is sorted!
    int n = scandir(dir.c_str(), &namelist, 0, alphasort);
    if (n < 0) {
        cout << "scandir error\n";
        return false;
    } else {
        files.reserve(n);
        for (size_t k = 0; k != n; ++k) {
            if (strcmp(namelist[k]->d_name, ".") == 0 ||
                    strcmp(namelist[k]->d_name, "..") == 0) {

                free(namelist[k]);
                continue;
            }

            files.push_back(string(namelist[k]->d_name));

            free(namelist[k]);
        }
        free(namelist);
    }
    return true;
}

// sig v2
// gid: by default process ALL graph ids (gid = -1), otherwise specific graph
// bgpid: by default process ONLY bgp # 0 (bgpid = 0), otherwise specific bgp #
bool queryBloomCounters(const querypolystrType &polystrbgps,
                        const queryorderType &spo_order,
                        vector<string> &candidates,
                        int gid,
                        int bgpid)
{
    if (cbf_capacities.empty()) {
        cout << "CBF capacities are missing\n";
        return false;
    }

    if (graphids.empty()) {
        cout << "GRAPHIDS are missing\n";
        return false;
    }

    int numReadBF = 0;
    int numReadCBF = 0;
    int numSkipped = 0;
    int numMatched = 0;
    int matched_lists = 0;
    int totalCountersCheckedEst = 0;
    int totalCountersChecked = 0;
    int totalCounters = 0;
    double totalcbfConsTime = 0;
    double totalcbfGetCountersTime = 0;
    double totalcbfCheckCountersTime = 0;
    double totalcbfTime = 0;
    bool skiprest = false;

    stringstream dirss;
    dirss << idxdir << "/" << string(BLOOMCOUNTERSDIR);
    string cbfsdir = dirss.str();

    int numLists, numGraphs;
    int start_gid, end_gid;
    // specific gid
    if (gid != -1) {
        start_gid = gid;
        end_gid = gid + 1;
        numLists = gid * SPOLISTS;
        numGraphs = gid;
    // all graphs
    } else {
        start_gid = 0;
        end_gid = graphids.size();
        numLists = 0;
        numGraphs = 0;
    }

    string gidstr;
    for (size_t i = start_gid; i != end_gid; ++i) {
        gidstr = graphids[i];

        cout << "graph #: " << numGraphs << endl;

        skiprest = false;
        for (size_t j = 0; j != spo_order[bgpid].size(); ++j) {
            int list = spo_order[bgpid][j];
            string ext = list2ext[list];

            stringstream sposs;
            sposs << gidstr << "." << ext;
            string file_name = sposs.str();

            stringstream pathss;
            pathss << cbfsdir << "/" << file_name;
            string file_path = pathss.str();

            // optimization 1:
            // skip rest of lists for same graph
            if (skiprest == true) {
                // XXX: verified but prone to error
                numLists += (SPOLISTS - j);
                break;
            }

            // Dablooms CBF: use this!
            if (dablooms_t == DACBF) {
                cout << "processing CBF: " << file_name
                     << " (" << numLists << ")" << endl;

                counting_bloom_t *dabloom;
                counting_bloom_t *qdabloom;

                if (polystrbgps[bgpid][list].empty()) {
                    cout << "skipping empty " << ext << " list\n";
                    ++numSkipped;
                    ++matched_lists;
                } else {
                    double cbfstartTime = getgtod();

                    // overwrite capacity with meta file value!
                    // XXX: correct cap. is graph # * 7 + list # in CBF.META
                    // numGraphs * SPOLISTS == numLists?
                    int cap_line = (numGraphs * SPOLISTS)
                                   + ext2list_cbfmeta[ext];
                    CAPACITY = cbf_capacities[cap_line];
                    cout << "CAPACITY: " << CAPACITY
                             << " from index #: " << cap_line << endl;
                    dabloom = new_counting_bloom_from_file(CAPACITY,
                                                           ERROR_RATE,
                                                           file_path.c_str());
                    if (!dabloom) {
                        cout << "Could not create counting bloom filter from "
                             << file_path << endl;
                        ++numLists;
                        continue;
                    }
                    ++numReadCBF;

                    // create query CBF
                    string qfile = "/tmp/dablooms_query";
                    stringstream cbfss;
                    cbfss << qfile << "." << bgpid << "." << file_name;
                    string qcbf = cbfss.str();

                    double consStartTime = getgtod();
                    qdabloom = new_counting_bloom(CAPACITY, ERROR_RATE,
                                                  qcbf.c_str());
                    int nfuncs = qdabloom->nfuncs;
                    cout << "nfuncs: " << nfuncs << endl;
                    // calls tmpfile()
                    //qdabloom = new_counting_bloom(CAPACITY, ERROR_RATE,
                    //                              "tmpfile");

                    // slows down!
                    //bitmap_flush(qdabloom->bitmap);

                    // add query POLYs to query CBF
                    for (size_t k = 0; k != polystrbgps[bgpid][list].size();
                         ++k) {
                        string polystr = polystrbgps[bgpid][list][k];
                        counting_bloom_add(qdabloom, polystr.c_str(),
                                           polystr.length());
                    }
                    totalcbfConsTime += getgtod() - consStartTime;

                    // don't cache query counters because CAPACITY varies!
                    int npolys = polystrbgps[bgpid][list].size();
                    int nqcount = nfuncs * npolys;

                    // TODO: check if new failed?
                    int *qcount_vals = new int[nqcount];
                    int *qcount_idxs = new int[nqcount];

                    // go over POLYs in query's list to get counter values
                    // and check with data CBFs
                    int cur_poly = 0;
                    for (size_t k = 0; k != polystrbgps[bgpid][list].size();
                         ++k) {
                        string polystr = polystrbgps[bgpid][list][k];

                        double getCountersStart = getgtod();
                        if (!counting_bloom_get_counters_idx(qdabloom,
                                                             polystr.c_str(),
                                                             polystr.length(),
                                                             &qcount_idxs,
                                                             &qcount_vals,
                                                             cur_poly * nfuncs)) {
                            cout << "get_counters failed\n";
                        }
                        totalcbfGetCountersTime += getgtod() - getCountersStart;

                        ++cur_poly;
                    }

#if 0
                    for (int z = 0; z < nqcount; z++) {
                        cout << qcount_idxs[z] << ":"
                                 << qcount_vals[z] << " ";
                    }
                    cout << endl;
#endif

                    // TODO: or use reserve instead?
                    vector<pair<int, int> > counters(nqcount);

                    // or C++11: use transform() with lambda
                    for (size_t k = 0; k != nqcount; ++k) {
                        counters[k] = make_pair(qcount_idxs[k], qcount_vals[k]);
                    }

                    // sorts by first element in pair
                    sort(counters.begin(), counters.end());

                    int ndup = 0;
                    for (size_t k = 1; k != counters.size(); ++k) {
                        //cout << counters[k].first << ":"
                        //       << counters[k].second << " ";
                        if (counters[k].first == counters[k-1].first) {
                            ++ndup;
                        }
                    }
                    //cout << endl;

                    for (size_t k = 0; k != counters.size(); ++k) {
                        qcount_idxs[k] = counters[k].first;
                        qcount_vals[k] = counters[k].second;
                    }

                    double checkCountersStart = getgtod();
                    int checked = 0;
                    int matched_polys = counting_bloom_check_counters_idx(dabloom, qcount_idxs, qcount_vals, nqcount, &checked);
                    double checkCountersEnd = getgtod();
                    totalcbfCheckCountersTime += checkCountersEnd - checkCountersStart;
                    cout << "single CBF check counters time: "
                         << checkCountersEnd - checkCountersStart << endl;

                    delete [] qcount_idxs;
                    delete [] qcount_vals;

                    if (ndup) cout << "counters duplicate: " << ndup << endl;
                    cout << "counters unique: " << nqcount - ndup << endl;
                    cout << "counters total: " << nqcount << endl;
                    cout << "counters checked*: "
                         << nfuncs * matched_polys << endl;
                    cout << "counters checked: " << checked << endl;
                    totalCountersCheckedEst += nfuncs * matched_polys;
                    totalCountersChecked += checked;
                    totalCounters += nqcount;

                    if (matched_polys == npolys) {
                        cout << "Match: " << file_name <<endl;
                        ++matched_lists;
                        ++numMatched;
                    // optimization: skip rest
                    } else {
                        cout << "No match of list (" << matched_polys << "/"
                             << npolys << "): will skip other lists\n";
                        skiprest = true;
                    }
                    double cbfendTime = getgtod();
                    totalcbfTime += cbfendTime - cbfstartTime;
                    cout << "single CBF total time: "
                         << cbfendTime - cbfstartTime << endl;
                    free_counting_bloom(qdabloom);
                    free_counting_bloom(dabloom);
                }
            }
            ++numLists;
        }
        ++numGraphs;

        // all lists matched
        if (matched_lists == SPOLISTS) {
            // XXX: empty BGP, exit
            if (numSkipped == SPOLISTS) {
                cout << "Warning: empty BGP, exiting" << endl;
                return false;
            }
            cout << "All lists matched " << gidstr << endl;
            candidates.push_back(gidstr);
        } else {
            cout << matched_lists << " matched " << gidstr << endl;
        }

        matched_lists = 0;
    }

    cout << "total lists: " << numLists << endl;
    cout << "total groups: " << numGraphs << endl;
    cout << "total skipped (empty) lists: " << numSkipped << endl;
    cout << "total matched (non-empty) lists: " << numMatched << endl;

    cout << "total CBF construction time: " << totalcbfConsTime << endl;
    cout << "total CBF get counters time: " << totalcbfGetCountersTime << endl;
    cout << "total CBF check counters time: "
             << totalcbfCheckCountersTime << endl;
    cout << "total CBF time: " << totalcbfTime << endl;

    cout << "total CBFs checked: " << numReadCBF << endl;
    cout << "total BFs checked: " << numReadBF << endl;
    cout << "total CBF counters checked (est): "
         << totalCountersCheckedEst << endl;
    cout << "total CBF counters checked: " << totalCountersChecked << endl;
    cout << "total CBF counters: " << totalCounters << endl;

    cout << "gid: " << gid << " bgpid: " << bgpid
         << " candidates: " << candidates.size() << endl;

    return true;
}

void usage(void)
{
    cout << "Usage: " << __progname << " [-h] [options...]\n\n";

    cout << "ACTIONS:\n"
         << "       -I index [requires many...]\n"
         << "       -Q query [requires -d, -M, (-A | -q)]\n\n"

         << "Pattern Vectors (PVs):\n"
         << "       -P <PVs file>\n"
         << "       -S <comma-separated names of PVs to skip>\n"
         << "       -m <MAX # of sigs>\n"
         << "       -C <MAX size of a CC>\n"
         << "       -N <MIN size of a CC>\n"
         << "       -U <MAX size of the union of PVs in a CC>\n"
         << "       -d <index dir>\n\n"

         << "Querying options:\n"
         << "       -A <query SPARQL file>\n"
         << "       -a <query parse tree file> [deprecated]\n"
         << "       -q <query sig. file>\n"
         << "       -r <min/max/default>\n"
         << "       -R <dir to save rewritten queries>\n"
         << "       -O optimization of BGP processing\n\n"

         << "Berkeley DB:\n"
         << "       -D <db name>\n"
         << "       -E <db env dir>\n"
         << "       -b <# of buffers>\n\n"

         << "LSH:\n"
         << "       -L use LSH (approximate but fast)\n"
         << "       -k <LSH parameter>\n"
         << "       -l <LSH parameter>\n"
         << "       -p <prime>\n"
         << "       -i <irrpoly>\n"
         << "       -H <hash file>\n\n"

         << "Jaccard index:\n"
         << "       -J use Jaccard (exact but slow)\n"
         << "       -f <fanout>\n"
         << "       -s <seed size>\n\n"

         << "Bloom Filters:\n"
         << "       -c <MAX capacity (inserts)>\n"
         << "       -e <false positive error rate>\n"
         << "       -M <meta file>\n\n"

         << "OPTIONS:\n"
         << "       -V verbose\n"
         << "       -v print version\n"
         << "       -z turns on dev (experimental) code\n"
         << "       -h print help/usage\n";

    exit(0);
}
