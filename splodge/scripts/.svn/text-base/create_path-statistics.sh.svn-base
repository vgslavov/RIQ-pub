#!/bin/sh
#------------------------------------------------------------------------------
# Creates statistical information for path join combination of predicates based
# on the entity statistics.
###############################################################################

# usage message
usage() {
cat << EOF
usage: $0 options

OPTIONS:
   -h      Show this message
   -i      Set input file (gzipped entity statistics)
   -o      Set output file (gzipped predicate statistics)
EOF
}


# set default output files
STATFILE="entity-stats.gz";
PATHFILE="path-stats.gz";

# parse arguments
while getopts "hi:o:" OPTION; do
  case $OPTION in
    h) usage; exit 1 ;;
    i) STATFILE=$OPTARG ;;
    o) PREDFILE=$OPTARG ;;
  esac
done
shift $(( OPTIND-1 )) # shift consumed arguments


# create predicate paths
echo `date +%X` "computing path statistics" >&2;
gzip -dc $STATFILE | perl -lne '
{
  ($entity, $stat1, $stat2) = split / /;
  if ($stat1 && $stat2) {
    for (split /,/, $stat1) {
      ($p1, $c1, $n1) = split /:/;
      for (split /,/, $stat2) {
        ($p2, $c2, $n2) = split /:/;
        $stat->{$p1}->{$p2}->{$c1}->{$c2}[0]++;
        $stat->{$p1}->{$p2}->{$c1}->{$c2}[1] += $n1;
        $stat->{$p1}->{$p2}->{$c1}->{$c2}[2] += $n2;
      }
    }
  }
} END {
  $time = `date +%X`;
  print STDERR "$time writing path statistics";
  for $p1 (sort {$a <=> $b} keys %$stat) {
    for $p2 (sort {$a <=> $b} keys %{$stat->{$p1}}) {
      for $c1 (sort {$a <=> $b} keys %{$stat->{$p1}->{$p2}}) {
        for $c2 (sort {$a <=> $b} keys %{$stat->{$p1}->{$p2}->{$c1}}) {
          $ref = $stat->{$p1}->{$p2}->{$c1}->{$c2};
          print join " ", $p1, $p2, $c1, $c2, @{$ref};
        }
      }
    }
  }
}' | gzip >$PATHFILE

echo `date +%X` "done. path statistics written to $PATHFILE" >&2;

#basic path stats (incl. bnodes)
#gzip -dc $STATFILE | awk -F"[ ]" '{if ($2) obj++; if ($3) subj++; if ($2 && $3) path++} END {print NR,obj,subj,path}'
#gzip -dc $STATFILE | grep "^_" | awk -F"[ ]" '{if ($2) obj++; if ($3) subj++; if ($2 && $3) path++} END {print NR,obj,subj,path}'
