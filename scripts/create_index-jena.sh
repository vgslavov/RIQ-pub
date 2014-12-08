#!/bin/bash

if [ $# -lt 4 -o $# -gt 5 ]; then
	echo "Usage: $0 [loader: 1, 2, 3] [list file]|[nt dir] [idx dir] [log dir] ([suffix])"
	exit 1;
fi

VER=$1
SRC=$2
OUTPUTDIR=$3
LOGDIR=$4

if [ $# -eq 5 ]; then
    SUFFIX=$5
else
    SUFFIX="nq"
fi

USER=`whoami`
USER="vsfgd"
HOME="/home/$USER"
JENADIR="/home/vsfgd/Jena/apache-jena-2.11.1/bin"
TIME="/usr/bin/time -v"

if [ $VER = 1 ]; then
	LOADER=$JENADIR/tdbloader
# fastest?
elif [ $VER = 2 ]; then
	LOADER=$JENADIR/tdbloader2
elif [ $VER = 3 ]; then
	LOADER=$JENADIR/tdbloader3
else
	echo "invalid version"
	exit 1;
fi

if [[ -d ${SRC} ]]; then
	NFILES=`ls $SRC | wc -l`
	n=0
	for i in `ls $SRC`; do
		n=`expr $n + 1`
		outfile=`basename $i .$SUFFIX`
		echo "$outfile ($n/$NFILES)"
		mkdir $OUTPUTDIR/$outfile
		$TIME -o $LOGDIR/time-$outfile.log $LOADER --loc=$OUTPUTDIR/$outfile $SRC/$i &> $LOGDIR/$outfile.log
	done
elif [[ -f ${SRC} ]]; then
	NFILES=`wc -l $SRC | awk '{print $1}'`
	n=0
	while read line; do
		n=`expr $n + 1`
		outfile=`basename $line .$SUFFIX`
		echo "$line: ($n/$NFILES)"
		mkdir $OUTPUTDIR/$outfile
		$TIME -o $LOGDIR/time-$outfile.log $LOADER --loc=$OUTPUTDIR/$outfile $line &> $LOGDIR/$outfile.log
	done < "$SRC"
else
	echo "invalid input"
fi

echo "# of files processed: $n"
