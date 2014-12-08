// Author: Praveen Rao
#ifndef _NODEPARAMS_H_
#define _NODEPARAMS_H_

// index node related defs
const int MAXENTRIES = 300;

enum InsertType {
  UPDATE = -1,
  SPLIT = -2,
  UPDATEHDRLOCK = -3,
  UPDATEHDR = -4,
  NONE = -5,
  REPLACE = -6,
  SPLITSPECIAL = -7,
  REPLACESIG = -8,
  UPDATEIFPRESENT = -9,
  APPEND = -10,
  UPDATEVAL = -11,
  UPDATEVALIFPRESENT
};
#endif
