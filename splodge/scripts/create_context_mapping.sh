#!/bin/sh
#------------------------------------------------------------------------------
# Aggregate domains with multiple subdomains, e.g. {bob,jane}.livejournal.com.
# 1. step: analyze contexts: frequency of common domain name, subdomain list.
# 2. step: condense contexts: replace context in quads with condensed domain.
###############################################################################

# set default statistic file
STATFILE="context-statistics.txt";
EXTENSION="_condensed-ctx.";

# handle all files from argument list
echo "analyzing context of $# files..." >&2;

# check if output statistic file already exists
if [ -e $STATFILE ] && [ $(stat -c%s "$STATFILE") -gt "0" ]; then
  echo "skipped context analysis: '$STATFILE' already exists.";
else
  for i in $@; do
    echo `date +%X` "$i" >&2;
    gzip -dc $i
  done \
  | awk '{print $(NF-1)}' | awk -F "/" '{print $3}' | perl -lne '
BEGIN {
  $tree = {};
  sub sum { my $ref = shift; my $sum = 0; $sum += ($_ ? &sum($ref->{$_}) : $ref->{$_}) for (keys %$ref); return $sum };
  sub dom { my $ref = shift; return map { my $name = $_; map { $_ ? "$_.$name" : $name } dom($ref->{$_}) if $_ } keys %$ref };
  sub aggregate {
    my ($ref, @names) = @_; 
    my @keys = keys %$ref;

    # aggregate domains which have multiple subdomains (except .au, .uk, .jp)
    if (@keys > 1 && @names > 1 && (@names > 2 || $names[0] !~ /^(au|uk|jp)$/)) {
      print sum($ref)," ",(join ".", reverse @names)," ",join ",", dom($ref);
    } else {
      # process all subdomains recursively or print details of fully traversed domain name
      for (@keys) { $_ ? aggregate($ref->{$_}, @names, $_) : print sum($ref)," ",join ".", reverse @names }
    }
  }
} {
  # add host parts to domain name tree
  my $ref = $tree; 
  map {$ref->{$_} = {} unless $ref->{$_}; $ref = $ref->{$_}} reverse split /\./; 
  $ref->{""}++;
} END {
  aggregate($tree);
}' \
  | sort -nr >$STATFILE
  echo `date +%X` "done. context statistics written to $STATFILE" >&2;
fi

# replace quad context with condensed context
echo "replacing context of $# files..." >&2;

# handle all files from argument list
for i in $@; do
  outfile=${i%.*}$EXTENSION${i##*.};

  # check if statistic file for current chunk already exists
  if [ -e $outfile ] && [ $(stat -c%s "$outfile") -gt "0" ]; then
    echo "skipped '$i': updated file already exists.";
    continue;
  fi

  echo `date +%X` "updating: $i -> $outfile" >&2;
  gzip -dc $i | perl -lne '
  BEGIN {
    # prepare predicate dictionary (predicate -> ID)
    open FILE, "'"$STATFILE"'" or die "error loading dictionary '"$STATFILE"': $!";
    while (<FILE>) {
      chomp;
      my ($count, $host, $subdomains) = split;
      if ($subdomains) {$dict{"$_.$host"} = $host for (split /,/, $subdomains)};
    }
    close FILE;
  } {
    my ($first, $host) = m!(.*<.*?//)(.*?)/!;
    print "$first".(exists($dict{$host}) ? $dict{$host} : $host)."/> .";
  }' | gzip >$outfile;
done
echo `date +%X` "done." >&2;
