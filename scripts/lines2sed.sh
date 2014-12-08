#!/bin/bash

if [ $# -ne 3 ]; then
	echo "Usage: $0 [print|del] [line # file] [src file]"
	exit 1;
fi

ACTION=${1}
LINESFILE=${2}
SRCFILE=${3}
NLINES=`wc -l $LINESFILE | awk '{print $1}'`

n=0
for i in `cat $LINESFILE`; do
	n=`expr $n + 1`
	echo "$i ($n/$NLINES)"

	# print
  if [ $ACTION == "print" ]; then
    EARGS="$EARGS -e ${i}p"
	# delete
  elif [ $ACTION == "del" ]; then
    #sed '$iq;d' $SRCFILE
    # or
    EARGS="$EARGS -e ${i}d"
  else
    echo "invalid action"
    exit 1;
  fi
done

echo "EARGS:$EARGS"

# print
if [ $ACTION == "print" ]; then
  sed -n $EARGS $SRCFILE
# delete
elif [ $ACTION == "del" ]; then
  sed -i.bak $EARGS $SRCFILE
fi
