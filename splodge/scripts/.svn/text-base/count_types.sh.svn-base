#!/bin/sh
#------------------------------------------------------------------------------
# Count the number of occurences of types (objects of rdf:type triples/quads).
###############################################################################

echo "processing $# files..." >&2;
for i in $@; do
  echo ": $i" >&2;
  gzip -dc $i;
done | \
awk '{if ($2 == "<http://www.w3.org/1999/02/22-rdf-syntax-ns#type>") p[$3]++} END{for(i in p) print p[i],i}' | perl -pe 's/[<>]//g' | sort -rn
