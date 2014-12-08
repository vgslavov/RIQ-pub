#!/bin/sh
#-------------------------------------------------------------------------
# Splitting quads by context and writing spo triples.
# Input must be sorted by context (cspo) - will not be checked.
##########################################################################

# usage message
usage() {
cat << EOF
usage: $0 options [quad_file.gz [...]]

OPTIONS:
   -h      Show this message
   -d      Set the target file (default: "split.nt")
EOF
}

#DSTDIR="split"
DSTFILE="split.nt"

# parse arguments
while getopts "hd:p:" OPTION; do
  case $OPTION in
    h) usage; exit 1 ;;
    d) DSTFILE=$OPTARG ;;
  esac
done
shift $(( OPTIND-1 )) # shift consumed arguments

echo "splitting data by context. writing chunks to '$DSTFILE'" >&2;

# check if output directory exists
#if [ ! -d "$DSTDIR" ]; then
#  echo Error: directory \'$DSTDIR\' not found
#  exit
#fi

# handle all files from argument list
echo "processing $# files..." >&2;
for i in $@; do
  echo `date +%X` "$i" >&2;
  gzip -dc $i | perl -slne '
  my ($dot, $ctx, @ops) = reverse split;
  open FILE, ">>$dstfile" or die "ERROR: cannot save file $dstfile: $!";
  if ($ctx ne "$last_ctx") {
    print FILE "\n# ctx: ", $ctx;
    $last_ctx = $ctx;
  }
  print FILE join " ", reverse $dot, $ctx, @ops;
  END {
    close FILE;
  }' -- -dstfile=$DSTFILE

  # for triples only (w/o ctx), use this:
  #print FILE join " ", reverse $dot, @ops;

    #close FILE if ($last_ctx);
    #open FILE, "| gzip >>$dir/$host.nt.gz" or die "ERROR: cannot save file $dir/$host.nt.gz: $!";
  #use URI::Escape;
  #my $host = substr(uri_escape($ctx), 0, 252);
  #my $host = (split /\//, $ctx)[2];

done

echo `date +%X` "done" >&2;
