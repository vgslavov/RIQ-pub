// UMKC, 2014
#ifndef _RIQ_H_
#define _RIQ_H_

const char *NODEISFULL = "node.full";

enum DHTStatus {
  SUCC = 0,
  FULL = 1,
  FAIL = 2,
  NODESPLIT = 3,
  REINSERT = 4,
  FIRST = 5,
  NOTFIRST = 6
};

const int maxLeafEntries = 50;
const int SLEEPTIME = 2;
const int MAXRETRIES = 5;

const int MAXSIGSPERQUERY = 5;
const char *LEAFNODE = "node.leaf";
const char *NONLEAFNODE = "node.nonleaf";
const char *ROOTNODE = "node.root";
const char *ROOTNODEID = "root.node.id";

#endif
