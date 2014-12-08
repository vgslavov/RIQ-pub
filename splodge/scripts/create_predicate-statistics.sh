#!/bin/sh
#------------------------------------------------------------------------------
# Creates statistical information for predicates based on the entity statistics
# including number of occurrences in each context.
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
PREDFILE="predicate-stats.gz";

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
echo `date +%X` "computing predicate statistics" >&2;
gzip -dc $STATFILE | perl -lne '
{
  for (split /,/, (split / /)[2]) {  # must split line on single " "
    ($p, $ctx, $n) = split /:/;
    $stat->{$p}->{$ctx}+=$n;
  }
} END {
  for $p (sort {$a <=> $b} keys %$stat) {
    $count = 0;
    $count += $stat->{$p}->{$_} for keys %{$stat->{$p}};
    # print pID, pCount, ctxCount, list of ctxId:pCountPerCtx
    print join " ", $p, $count, scalar keys %{$stat->{$p}}, join ",", map {"$_:".$stat->{$p}->{$_}} keys %{$stat->{$p}};
  }
}' | gzip >"$PREDFILE"

echo `date +%X` "done. predicate statistics written to $PREDFILE" >&2;
