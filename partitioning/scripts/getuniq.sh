#!/bin/sh
i=0
while [ $i -lt $3 ]
do
	sort -n -k 1,1 $1$i | uniq > $2$i
	rm -f $1$i
	i=$(( $i + 1 ))
done
