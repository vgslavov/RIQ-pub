#!/bin/sh
#------------------------------------------------------------------------------
# Count the number of links between datasets, i.e. the quads where object URI
# and context URI have different domains. Prints the number of occurences of a
# link, the predicate URI of the link, and the number of different contexts.
# rdf:type is exluded.
###############################################################################

echo "processing $# files..." >&2;
for i in $@; do
  echo ": $i" >&2;
  gzip -dc $i;
done \
| awk '{if (NF==5 && index($3,"<") == 1) print $2 $3 $4}' \
| grep -v "^<http://www.w3.org/1999/02/22-rdf-syntax-ns#type" \
| perl -ne '{m!(.*?>).*//(.*?)/.*//(.*?)/!; $p{$1}{$3}++ if $2 !~ m/$3/} END {for $k (keys %p) {$cnt=keys %{$p{$k}}; $sum=0; $sum+=$p{$k}{$_} for keys %{$p{$k}}; print "$sum $k $cnt\n"}}' \
#| perl -ne '{m!(.*?>).*//(.*?)/.*//(.*?)/!; $p{$1}{$3}++ if $2 ne $3} END {for $k (keys %p) {$cnt=keys %{$p{$k}}; $sum=0; $sum+=$p{$k}{$_} for keys %{$p{$k}}; print "$sum $k $cnt\n"}}'
#| grep -v "^<http://www.w3.org/1999/02/22-rdf-syntax-ns#predicate" \
#| grep -v "^<http://www.w3.org/1999/02/22-rdf-syntax-ns#subject" \
#| grep -v "^<http://www.w3.org/1999/02/22-rdf-syntax-ns#object" \
#| grep -v "^<http://www.w3.org/2000/01/rdf-schema#subPropertyOf" \
#| grep -v "^<http://www.w3.org/2000/01/rdf-schema#subClassOf" \
#| grep -v "^<http://www.w3.org/2000/01/rdf-schema#domain" \
#| grep -v "^<http://www.w3.org/2000/01/rdf-schema#range" \
#| grep -v "^<http://www.w3.org/2002/07/owl#equivalentProperty" \
#| grep -v "^<http://www.w3.org/2002/07/owl#equivalentClass" \
#| grep -v "^<http://www.w3.org/2002/07/owl#inverseOf"
