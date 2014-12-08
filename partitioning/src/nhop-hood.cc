#include <iostream>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <vector>
#include <map>
#include <unordered_map>
#include <sstream>
#include <string>

#include "utils.h"	// for manipulating POLYs

using namespace std;

extern char *__progname;
bool verbose = false;

double getgtod();
void revdir(int &, int &);
void usage();
void write2part(FILE *, int, int, int);

int main(int argc, char *argv[])
{
	char buf[128];
	int ch, cflag, gflag, mflag, oflag, pflag;
	int nhops, nparts, maxdegree;
	int sub, obj, pred;
	int docId;
	double beginTime, endTime;
	static char *gisf, *metisinf, *metisoutf, *cutedgesf;
	string outf;
	FILE *fpgis, *fpmetisin, *fpmetisout, *fpcutedges, *fpout;
	//typedef map<int, vector<pair<int, int> > > int2vecinthash;
	typedef unordered_map<int, vector<pair<int, int> > > int2vecinthash;
	// full graph from GIS file
	int2vecinthash edgeSet;
	// cut edges only
	int2vecinthash cutedgeSet;
	int2vecinthash::iterator itr;
	// vector index = vertex id, vpart[vid] = partition
	vector<int> vpart;
	// output files
	vector<FILE *> vfiles;

	// start at position 1!
	vpart.push_back(-1);
	cflag = gflag = mflag = oflag = pflag = 0;
	docId = 0;
	nparts = 0;
	// nhops == 0 is valid
	nhops = -1;
	maxdegree = 0;

	while ((ch = getopt(argc, argv, "hc:g:H:M:m:o:P:p:V")) != -1) {
		switch (ch) {
		case 'c':
			cflag = 1;
			cutedgesf = optarg;
			break;
		case 'g':
			gflag = 1;
			gisf = optarg;
			break;
		case 'H':
			nhops = strtol(optarg, NULL, 10);
			break;
		case 'M':
			maxdegree = strtoul(optarg, NULL, 10);
			break;
		case 'm':
			mflag = 1;
			metisinf = optarg;
			break;
		case 'o':
			oflag = 1;
			outf = optarg;
			break;
		case 'P':
			nparts = strtol(optarg, NULL, 10);
			break;
		case 'p':
			pflag = 1;
			metisoutf = optarg;
			break;
		case 'V':
			verbose = true;
			break;
		case 'h':
		case '?':
		default:
			usage();
			break;
			/* NOTREACHED */
		}
	}
	argc -= optind;
	argv += optind;

	// required files
	if (gflag == 0 || pflag == 0 || oflag == 0)
		usage();

	// required numbers
	if (nhops == -1 || nparts == 0 || maxdegree == 0)
		usage();

	if ((fpgis = fopen(gisf, "r")) == NULL) {
		fprintf(stderr, "can't read GIS file %s\n",
			gisf);
		exit(1);
	}

	if ((fpmetisout = fopen(metisoutf, "r")) == NULL) {
		fprintf(stderr, "can't read gpmetis output file %s\n",
			metisoutf);
		exit(1);
	}

	for (int i = 0; i < nparts; i++) {
		string tmpf = outf + "." + to_string(i);
		if ((fpout = fopen(tmpf.c_str(), "w")) == NULL) {
			fprintf(stderr, "can't write output file %s\n",
				tmpf.c_str());
			exit(1);
		} else {
			vfiles.push_back(fpout);
		}
	}

	if (mflag == 1) {
		if ((fpmetisin = fopen(metisinf, "r")) == NULL) {
			fprintf(stderr, "can't read gpmetis input file %s\n",
				metisinf);
			exit(1);
		}
	}

	if (cflag == 1) {
		if ((fpcutedges = fopen(cutedgesf, "r")) == NULL) {
			fprintf(stderr, "can't read cut edges file %s\n",
				cutedgesf);
			exit(1);
		}

		// read cut edges file
		cout << "reading cut edges file...\n";
		beginTime = getgtod();
		while (1) {
			if (fgets(buf, sizeof(buf), fpcutedges) == NULL) break;
			char junk;
			sscanf(buf, "%d %d %d %c", &sub, &pred, &obj, &junk);

			int2vecinthash::iterator itr;

			// find all v with same sub
			itr = cutedgeSet.find(sub);
			if (itr != cutedgeSet.end()) {
				pair<int, int> e2(obj, pred);
				itr->second.push_back(e2);
			} else {
				vector<pair<int, int> > v;
				pair<int, int> e2(obj, pred);
				v.push_back(e2);
				cutedgeSet[sub] = v;
			}
			// find all v with same obj
			itr = cutedgeSet.find(obj);
			if (itr != cutedgeSet.end()) {
				pair<int, int> e2(-1*sub, pred);
				itr->second.push_back(e2);
			} else {
				vector<pair<int, int> > v;
				pair<int, int> e2(-1*sub, pred);
				v.push_back(e2);
				cutedgeSet[obj] = v;
			}
		}
		endTime = getgtod();
		cout << "cut edges file read time: " << endTime - beginTime << endl;

		int2vecinthash::iterator citr;
		int cutmaxlen = 0;
		for (citr = cutedgeSet.begin(); citr != cutedgeSet.end(); citr++) {
			//fprintf(fpout, "%d: ", itr->first);
			cout << citr->first << ": ";
			//fprintf(fpout, " %lu\n", itr->second.size());
			if (cutmaxlen < citr->second.size())
				cutmaxlen = citr->second.size();
			for (int i = 0; i < citr->second.size(); i++) {
				//fprintf(fpout, "%d->%d ", itr->second[i].second, itr->second[i].first);
				cout << citr->second[i].second << "->" << citr->second[i].first << " ";
			}
			//fprintf(fpout, "\n");
			cout << endl;
		}
		cout << "cutedgeSet maxlen: " << cutmaxlen << endl;
	}

	// read gpmetis output file
	cout << "reading gpmetis output file...\n";
	beginTime = getgtod();
	while (1) {
		if (fgets(buf, sizeof(buf), fpmetisout) == NULL) break;
		int part = -1;
		sscanf(buf, "%d", &part);
		// TODO: check if valid
		vpart.push_back(part);
	}
	endTime = getgtod();
	cout << "gpmetis output file read time: " << endTime - beginTime << endl;
	cout << "vpart.size(): " << vpart.size() << endl;

	// read GIS file
	cout << "reading GIS file...\n";
	beginTime = getgtod();
	while (1) {
		if (fgets(buf, sizeof(buf), fpgis) == NULL) break;
		char type; 

		sscanf(buf, "%c", &type);	 

		switch(type) {
			case '#':
				cout << "COMMENT line\n";
				break;
			case 't':
				{
					cout << "BEGIN new: ";
					char junk;
					sscanf(buf, "%c %c %d", &type, &junk, &docId);
					cout << docId << endl;
					// TODO: support more than 1 graphs
				}
				break;
			// {} are important
			// to avoid "crosses initialization" errors
			case 'u':
				{
					sscanf(buf, "%c %d %d %d", &type, &sub, &obj, &pred); 
					int2vecinthash::iterator itr;

					// find all v with same sub
					itr = edgeSet.find(sub);
					if (itr != edgeSet.end()) {
						pair<int, int> e2(obj, pred);
						itr->second.push_back(e2);
					} else {
						vector<pair<int, int> > v;
						pair<int, int> e2(obj, pred);
						v.push_back(e2);
						edgeSet[sub] = v;
					}
					// find all v with same obj
					itr = edgeSet.find(obj);
					if (itr != edgeSet.end()) {
						//pair<int, int> e2(sub, pred);
						pair<int, int> e2(-1*sub, pred);
						itr->second.push_back(e2);
					} else {
						vector<pair<int, int> > v;
						//pair<int, int> e2(sub, pred);
						pair<int, int> e2(-1*sub, pred);
						v.push_back(e2);
						edgeSet[obj] = v;
					}
				}
				break;
			case 'v':
				cout << "'v' is invalid type\n";
				break;
			default:
				cout << "invalid type\n";
				break;
		}
	}
	endTime = getgtod();
	cout << "GIS file read time: " << endTime - beginTime << endl;

	if (verbose == true) {
		int maxlen = 0;
		for (itr = edgeSet.begin(); itr != edgeSet.end(); itr++) {
			//fprintf(fpout, "%d: ", itr->first);
			cout << itr->first << ": ";
			//fprintf(fpout, " %lu\n", itr->second.size());
			if (maxlen < itr->second.size())
				maxlen = itr->second.size();
			for (int i = 0; i < itr->second.size(); i++) {
				//fprintf(fpout, "%d->%d ", itr->second[i].second, itr->second[i].first);
				cout << itr->second[i].second << "->" << itr->second[i].first << " ";
			}
			//fprintf(fpout, "\n");
			cout << endl;
		}
		cout << "edgeSet maxlen: " << maxlen << endl;
	}

	bool negcutedge = false;
	bool negedge = false;
	cout << "writing output file...\n";
	for (int i = 0; i < vfiles.size(); i++) {
		fprintf(vfiles[i], "t #%d\n", i);
	}
	for (itr = edgeSet.begin(); itr != edgeSet.end(); itr++) {
		sub = itr->first;
		if (verbose == true) cout << "current v: " << sub << endl;
		for (int i = 0; i < itr->second.size(); i++) {
			obj = itr->second[i].first;
			pred = itr->second[i].second;
			if (verbose == true) cout << "pred: " << pred << " obj: " << obj << endl;
			// skip if in same partition but opposite direction
			// (will be processed later)
			if (obj < 0) {
				obj *= -1;
				if (verbose == true) cout << "skipping edge: will output later\n";
				if (vpart[sub] == vpart[obj])
					continue;
				else {
					if (verbose == true) cout << "cut edge is neg: traverse edge but don't output\n";
					negcutedge = true;
				}
			}
			// cut edge: vertices belong to different part.
			if (vpart[sub] != vpart[obj]) {
				if (nhops > 0) {
					// write only if not negative cut edge
					if (negcutedge == false) {
						// write to both part.
						if (verbose == true) cout << "1-hop edge:\n";
						if (verbose == true) cout << vpart[sub] << ":";
						write2part(vfiles[vpart[sub]], sub, obj, pred);
						if (verbose == true) cout << vpart[obj] << ":";
						write2part(vfiles[vpart[obj]], sub, obj, pred);
					} else {
						negcutedge = false;
					}
					// 1-hop outputs cut edges only
					if (nhops == 1) continue;
					int2vecinthash::iterator hitr;
					hitr = edgeSet.find(obj);
					if (hitr != edgeSet.end()) {
						int hsub = hitr->first;
						if (verbose == true) cout << "new v (1-hop away): " << hsub << endl;
						// skip high degree vertices
						if (hitr->second.size() > maxdegree) continue;
						for (int j = 0; j < hitr->second.size(); j++) {
							int hobj = hitr->second[j].first;
							int hpred = hitr->second[j].second;
							if (verbose == true) cout << "new pred: " << hpred << " new obj: " << hobj << endl;
							// reset negedge before setting it!
							negedge = false;
							if (hobj < 0) {
								negedge = true;
								hobj *= -1;
							}
							// same as cut edge above
							if (hobj == sub) {
								if (verbose == true) cout << "skipping edge: same as cut edge above\n";
								continue;
							}
							// TODO: skip cutedges which cross boundary again?
							if (vpart[hsub] == vpart[hobj]) {
								if (verbose == true) cout << "2-hop edge:\n";
								if (negedge == true) {
									revdir(hsub, hobj);
								}
								if (verbose == true) cout << vpart[sub] << ":";
								write2part(vfiles[vpart[sub]], hsub, hobj, hpred);
								if (negedge == true) {
									if (verbose == true) cout << "reverting back to original edge direction\n";
									revdir(hsub, hobj);
								}
							}
						}
					}
				// 0-hop skips cut edges
				} else {
					if (verbose == true) {
						cout << "skipping cut edge: " 
						     << sub << "-"
						     << pred << "-"
						     << obj << endl;
					}
				}
			// same part.
			} else {
				if (verbose == true) cout << vpart[sub] << ":";
				write2part(vfiles[vpart[sub]], sub, obj, pred);
			}
		}
	}

	fclose(fpgis);
	fclose(fpout);
	fclose(fpmetisout);
	if (cflag == 1) fclose(fpcutedges);
	if (mflag == 1) fclose(fpmetisin);
	
	return 0;
}

void
write2part(FILE *fpout, int sub, int obj, int pred)
{
	if (verbose == true) cout << "writing: sub: " << sub << " pred: " << pred << " obj: " << obj << endl;
	fprintf(fpout, "u %d %d %d\n", sub, obj, pred);
}

void
revdir(int &sub, int &obj)
{
	//cout << "reversing direction of edge\n";

	// doesn't work if using tmp var. why?
	sub = sub + obj;
	obj = sub - obj;
	sub = sub - obj;
}

double
getgtod()
{
        static struct timeval gtod_time;
        gettimeofday(&gtod_time, NULL);
        return (double)gtod_time.tv_sec
                + (double)gtod_time.tv_usec / CLOCKS_PER_SEC;
}

void
usage()
{
	fprintf(stderr, "Usage: %s [-h] [options...]\n\n", __progname);
	fprintf(stderr, "Options:\n"
			"\tRequired Files:\n"
			"\t[-g]		[graph file in GIS]\n"
			"\t[-p]		[MetisFile.part.*: gpmetis output file]\n"
			"\t[-o]		[output file]\n\n"
			"\tOptional Files:\n"
			"\t[-c]		[cut edges file]\n"
			"\t[-m]		[MetisFile: gpmetis input file]\n\n"
			"\tNumbers:\n"
			"\t[-H]		<# of hops>\n"
			"\t[-M]		<max vertex degree>\n"
			"\t[-P]		<# of partitions>\n"
			"\tOther options:\n"
			"\t[-V]		verbose\n");
	exit(1);
}
