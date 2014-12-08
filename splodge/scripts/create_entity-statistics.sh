#!/bin/sh
#------------------------------------------------------------------------------
# Creates entity statistics for URIs and BNodes in subject/object position of
# RDF quads. Counts occurences of predicates and contexts (sources).
#
# PARAMS: list of gzipped input files, predicate and context dictionary.
# OUTPUT: gzipped sorted text file with statistics.
#         Format: three space-separated columns: URI obj-stats subj-stats, i.e.
#                 URI [(predID:ctxID:count),...] [(predID:ctxID:count),...]
###############################################################################

# usage message
usage() {
cat << EOF
usage: $0 options [quad_file.gz [...]]

OPTIONS:
   -h      Show this message
   -o      Set output file (gzipped)
   -p      Set predicate dictionary file (one URI per line)
   -c      Set context dictionary file (one URI per line)
   -t      Set temp dir (for sorting)
EOF
}

# speed up sorting: compare ASCII byte values instead of UTF characters.
# note: bash can also set variables just for the immediate subprocess.
#       (e.g. alias sort='LANG=C sort'; in tcsh: env LANG=C sort)
export LANG=C;  # affects LC_COLLATE for all subprocesses

# set default output files
STATFILE="entity-stats.gz";
PATHFILE="path-stats.gz";

# parse arguments
while getopts "ho:p:c:t:" OPTION; do
  case $OPTION in
    h) usage; exit 1 ;;
    o) STATFILE=$OPTARG ;;
    p) PDICT=$OPTARG ;;
    c) CDICT=$OPTARG ;;
    t) TMP=$OPTARG ;;
  esac
done
shift $(( OPTIND-1 )) # shift consumed arguments

# check dictionary settings
if [ -z "$PDICT" ]; then
  echo "WARNING: a predicate dictionary must be supplied (use -p flag, or -h for help).";
  exit 1;
fi
if [ -z "$CDICT" ]; then
  echo "WARNING: a context dictionary must be supplied (use -c flag, or -h for help).";
  exit 1;
fi
# check temp dir settings
if [ ! -z "$TMP" ]; then
  if [ -e "$TMP" ] && [ -d "$TMP" ]; then
    export TMPDIR=$TMP; # setting '-T $TMP' for sort does not seem to work for merge
  else
    echo "invalid temp directory: $TMP";
    exit 1;
  fi
fi


# handle all files from argument list
echo "processing $# files..." >&2;
for i in $@; do

  outfile=${i%.*}_entity-stats.txt;
  statfiles=$statfiles" "$outfile;

  # check if statistic file for current chunk already exists
  if [ -e $outfile ] && [ $(stat -c%s "$outfile") -gt "0" ]; then
    echo "skipped '$i': stat file already exists.";
    continue;
  fi

  # create entity statistics for current chunk
  echo `date +%X` "$i" >&2;
  gzip -dc $i | perl -ne '
  BEGIN {
    # prepare predicate dictionary (predicate -> ID)
    open FILE, "'"$PDICT"'" or die "ERROR: cannot load dictionary '"$PDICT"': $!";
    while (<FILE>) { chomp; $pdict{"<$_>"} = $. }
    close FILE;
    open FILE, "'"$CDICT"'" or die "ERROR: cannot load dictionary '"$CDICT"': $!";
    while (<FILE>) { chomp; $cdict{"<$_>"} = $. }
    close FILE;
  } {
    # count predicates (ID) for incoming and outgoing edges of entities (URI/BNode)
    my ($s, $p, $o, @rest) = split;
    $pID = $pdict{$p}              or warn "WARNING: invalid predicate \"$p\" for subject $s at <> line $.\n" and next;
    $ctx = $cdict{$rest[$#rest-1]} or warn "WARNING: invalid context $rest[$#rest-1] for subject $s at <> line $.\n" and next;
    $stat->{$s}[1]->{$pID}->{$ctx}++;                                  # increase counter for (s,pID,ctx)
    $stat->{$o}[0]->{$pID}->{$ctx}++ if ($#rest == 1 && $o !~ "^\"");  # increase counter for (pID,o,ctx); no literals
  } END {
    # print predicate lists (IDs) with predicate frequency for incoming and outgoing edges of each entity
    # serialization of entity->predicate->context->count
    print ((join " ", $_, map {my $pmap=$_; defined $_ ? join ",", map {my $p=$_; map {"$p:$_:$pmap->{$p}->{$_}"} keys %{$pmap->{$p}}} sort keys %$_ : ""} @{$stat->{$_}})."\n") for (keys %$stat);
  }' \
  | tr -d '<>' | sort >$outfile;  # remove URI brackets and save sorted entitity statistics
done


# aggregate statistic files and merge duplicates
echo `date +%X` "merging entity statistics" >&2;

# check if output statistic file already exists
if [ -e $STATFILE ] && [ $(stat -c%s "$STATFILE") -gt "0" ]; then
  echo "skipped merging: '$STATFILE' already exists.";
else
  sort -m $statfiles | perl -ne '{
    chomp; ($uri, @stat) = split /[ ]/;
    if ($last_uri ne $uri) {
      print "$last_uri @last_stat\n" if ($last_uri);
      ($last_uri, @last_stat) = ($uri, @stat);
      next;
    }
    map {map {($p, $ctx, $val) = split /:/; $data[$idx]->{$p}->{$ctx}+=$val;} split /,/; $idx = ++$idx&1; } @stat[0..1], @last_stat[0..1];
    # serialization of entity->predicate->context->count
    @last_stat = map {my $pmap=$_; defined $_ ? join ",", map {my $p=$_; map {"$p:$_:$pmap->{$p}->{$_}"} keys %{$pmap->{$p}}} sort keys %$_ : ""} @data;  # sort { $a <=> $b } is slower
    @data = ();
  } END {
    print "$last_uri @last_stat\n";
  }' | gzip >$STATFILE

  echo `date +%X` "done. entity statistics written to $STATFILE" >&2;
fi

# testing the generated entity statistics (sum of all counts (s,p,ctx)
#gzip -dc $STATFILE | perl -lne '{$sum += (split /:/)[2] for split /,/, (split / /)[2]} END {print $sum}'  # fastest
#gzip -dc $STATFILE | awk -F"[ ]" '{if ($3) {split ($3,a,","); for (i in a) {split (a[i],b,":"); sum+=b[3] } } } END {print sum}'  # 3x slower

