#!/bin/bash

if [ $# -ne 2 ]; then
	echo "Usage: $0 [nt dir] [dst file]"
	exit 1;
fi

NTDIR=${1}
DSTFILE=${2}
NNTFILES=`ls $NTDIR | wc -l`

n=0
for i in `ls $NTDIR`; do
	n=`expr $n + 1`
	#echo "$i ($n/$NNTFILES)"
	ctx=`basename $i .nt`
	echo "# ctx: $ctx" >> $DSTFILE
	cat $NTDIR/$i >> $DSTFILE
	echo >> $DSTFILE
done
