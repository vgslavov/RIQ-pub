#!/bin/bash

if [ $# -ne 2 ]; then
	echo "Usage: $0 [list file] [dst file]|[dst dir]"
	exit 1;
fi

INPUTFILE=${1}
DST=${2}
#TYPE=${2}
BASEURI="http://sce.umkc.edu"
OUTPUT="ntriples"
CMD="rapper -I $BASEURI -o $OUTPUT"
#cd $SRCDIR
#CMD2="find . -name '*.$TYPE' -type f -print0 | xargs -0 ls"
#echo $CMD2
#INPUT=`${CMD2}`
#echo $INPUT
#INPUT=`ls $SRCDIR/*.$TYPE`

n=0
while read line; do
	n=`expr $n + 1`
	echo $line: $n
	if [[ -d ${DST} ]]; then
		#echo "$DST is a dir"
		# overwrite!
		outfile=`basename $line .owl`
		$CMD $line > $DST/$outfile.nt
	# don't test, file doesn't exist
	#elif [[ -f ${DST} ]]; then
	else
		#echo "$DST is a file"
		$CMD $line >> $DST
		echo "" >> $DST
	fi
done <"$INPUTFILE"

echo "# of files: $n"
