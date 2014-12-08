#!/bin/sh
#------------------------------------------------------------------------------
# Count the number of occurences of predicates in RDF triples/quads.
###############################################################################

echo "processing $# files..." >&2;
for i in $@; do
  echo ": $i" >&2;
  gzip -dc $i;
done | \
awk '{p[$2]++} END{for(i in p) print p[i],i}' | perl -pe 's/[<>]//g' | sort -rn
#awk '{p[$2]++} END{for(i in p) print p[i],substr(i,2,length(i)-2)}' | sort -rn
#perl -ne '{$p{(m!.*? <(.*?)>!)[0]}++}END{print "$p{$_} $_\n" for keys %p}' | sort -rn   # 3 times slower
