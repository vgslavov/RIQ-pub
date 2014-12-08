#!/bin/sh
#------------------------------------------------------------------------------
# Sort the RDF quads in SPOC or CSPO order and remove duplicates.
###############################################################################

# usage message
usage() {
cat << EOF
usage: $0 options [quad_file.gz [...]]

OPTIONS:
   -h      Show this message
   -c      Use CSPO instead of SPOC sort order
   -s      Set memory size for sorting (~40% of max mem if -z is used)
   -t      Set temp dir for sorting
   -z      Enable gzip compression for temp files
   -o      Set output file (gzipped)
EOF
}

# speed up sorting: compare ASCII byte values instead of UTF characters.
# (bash can also set variables just for an immediate subprocess, e.g.
#  alias sort='LANG=C sort'; in tcsh: env LANG=C sort)
export LANG=C;  # affects LC_COLLATE and all subprocesses

# parse arguments
while getopts "hco:s:t:z" OPTION; do
  case $OPTION in
    h) usage; exit 1 ;;
    c) CSPO=1 ;;
    o) OUTFILE=$OPTARG ;;
    s) MEMSIZE="-S $OPTARG" ;;
    t) TMP=$OPTARG ;;
    z) COMPRESS="--compress-program=gzip" ;;
  esac
done
shift $(( OPTIND-1 )) # shift consumed arguments

# check temp dir settings
if [ ! -z "$TMP" ]; then
  if [ -e "$TMP" ] && [ -d "$TMP" ]; then
    TMP="-T $TMP";
  else
    echo "invalid temp directory: $TMP";
    exit 1;
  fi
fi

# prepare sort command (reorder quads for CSPO)
SORT="sort -u $MEMSIZE $TMP $COMPRESS";
#SORT="sort $MEMSIZE $TMP $COMPRESS";
if [ "$CSPO" = "1" ]; then
  SORT="awk '{print \$(NF-1),\$0}' | $SORT | cut -f2- -d' '";
fi

# set name for sort order and default output file
[ "$CSPO" = "1" ] && ORDER="cspo" || ORDER="spoc";
if [ -z $OUTFILE ]; then
  OUTFILE="sorted_$ORDER.gz"
fi

# do the sorting
echo "sorting $# files... [$ORDER]" >&2;
for i in $@; do
  echo `date +%X` "$i" >&2;
  gzip -dc $i
done \
| eval $SORT | gzip >$OUTFILE

echo `date +%X` "done. sorted data [$ORDER] written to $OUTFILE" >&2;
