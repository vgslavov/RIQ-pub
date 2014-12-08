#!/bin/sh
#------------------------------------------------------------------------------
# Count the number of occurences of context hosts in RDF quads.
###############################################################################

echo "processing $# files..." >&2;

for i in $@; do
  echo ": $i" >&2;
  gzip -dc $i;
done \
| awk '{print $(NF-1)}' | awk -F "/" '{c[$3]++} END {for(i in c) print c[i],i}' | sort -nr
#| awk '{print $(NF-1)}' | awk -F "/" '{print $3}' | perl -ne '{$c{$_}++} END {print "$c[$_] $_" for keys %c}' | sort -nr
#| awk '{print $(NF-1)}' | perl -ne '{$c{(split /\//)[2]}++} END {print "$c[$_] $_\n" for keys %c}' | sort -nr
#| awk '{split($(NF-1),a,"/"); c[a[3]]++} END {for(i in c) print c[i],i}' | sort -nr   # 4 times slower
