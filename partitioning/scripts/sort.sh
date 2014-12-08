#! /bin/sh
var1='1'
var2='2'
#sort -n -k 1,1 $1 > $1$var1
sort -n -k 3,3 $1 > $1$var2
mv $1 $1$var1
