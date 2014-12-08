#!/bin/sh
#------------------------------------------------------------------------------
# Count the number of entities in RDF triples/quads.
###############################################################################

echo "CAUTION: duplicates from different chunks are not eliminated yet." >&2;
echo "processing $# files..." >&2;
for i in $@; do
  echo ": $i" >&2;
  gzip -dc $i \
  | awk '
  {
    # increase counter for subject/object URIs
    # note: "index($3,"<") == 1" is faster than "$3 ~ /^</" and "substr($3,1,1)"

    if (index($1,"<") == 1) s[$1]++;
    if (NF==5 && index($3,"<") == 1) s[$3]++;
  }
  END {
    for(i in s) print s[i],i
  }'
done \
| tr -d '<>' \
| sort -nr -S 2g -T .

