#!/bin/sh
#-------------------------------------------------------------------------
# Splitting quads by context and writing spo triples.
# Input must be sorted by context - will not be checked.
##########################################################################

# usage message
usage() {
cat << EOF
usage: $0 options [quad_file.gz [...]]

OPTIONS:
   -h      Show this message
   -d      Set the target directory (default: "split")
EOF
}

DSTDIR="split"

# parse arguments
while getopts "hd:p:" OPTION; do
  case $OPTION in
    h) usage; exit 1 ;;
    d) DSTDIR=$OPTARG ;;
  esac
done
shift $(( OPTIND-1 )) # shift consumed arguments

echo "splitting data by context. writing chunks to '$DSTDIR'" >&2;

# check if output directory exists
if [ ! -d "$DSTDIR" ]; then
  echo Error: directory \'$DSTDIR\' not found
  exit
fi

# handle all files from argument list
echo "processing $# files..." >&2;
for i in $@; do

  echo `date +%X` "$i" >&2;
  gzip -dc $i | perl -slne '
  my ($dot, $ctx, @ops) = reverse split;
  my $host = (split /\//, $ctx)[2];
  if ($ctx ne "$last_ctx") {
    close FILE if ($last_ctx);
    open FILE, "| gzip >>$dir/$host.nt.gz" or die "ERROR: cannot save file $dir/$host.nt.gz: $!";
    $last_ctx = $ctx;
  }
  print FILE join " ", reverse $dot, @ops;
  END {
    close FILE;
  }' -- -dir=$DSTDIR
done

echo `date +%X` "done" >&2;
