#!/bin/bash

if [ $# -ne 3 ]; then
        echo "Usage: $0 [list file]|[nt dir] [idx dir] [log dir]"
        exit 1;
fi

SRC=$1
INDEXDIR=$2
LOGDIR=$3

#USER=`whoami`
USER="vsfgd"
HOME="/home/$USER"
#SUFFIX="nt"
SUFFIX="n3"
#RDF3XLOAD="rdf3xload"
RDF3XLOAD="$HOME/rdf3x-0.3.7-vsfgd/bin/rdf3xload"
TIME="/usr/bin/time -v"

if [[ -d ${SRC} ]]; then
	n=0
	for i in `ls $SRC`; do
		n=`expr $n + 1`
		echo $i: $n
		outfile=`basename $i .$SUFFIX`
		$TIME -o $LOGDIR/time-$outfile.log $RDF3XLOAD $INDEXDIR/$outfile.db $SRC/$i &> $LOGDIR/$outfile.log
		#echo $i.idx >> $LOGFILE
	done
elif [[ -f ${SRC} ]]; then
	n=0
	while read line; do
		n=`expr $n + 1`
		echo $line: $n
		outfile=`basename $line .$SUFFIX`
		$TIME -o $LOGDIR/time-$outfile.log $RDF3XLOAD $INDEXDIR/$outfile.db $line &> $LOGDIR/$outfile.log
	done < "$SRC"
else
	echo "invalid input"
fi

echo "# of files processed: $n"
