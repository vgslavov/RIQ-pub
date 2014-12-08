#!/bin/sh
#------------------------------------------------------------------------------
# Count the number of occurences of raw contexts in RDF quads.
###############################################################################

export LANG=C;  # speed up sorting

echo "processing $# files..." >&2;

for i in $@; do
  echo `date +%X` "$i" >&2;
  gzip -dc $i;
done \
| awk '{c[$(NF-1)]++} END {for(i in c) print c[i],i}' | tr -d '<>' | sort -nr
