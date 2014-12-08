#! /bin/sh

rdf3x="./rdf3xquery"

# check input parameters
if [ $# -lt 1 ]; then echo "USAGE: ${0##*/} queries"; exit; fi

# pipe SPARQL queries (one per line) to rdf3x and count results (ignore '<empty result>')
cat $1 | while read LINE; do 
  echo "$LINE" | scripts/timeout -t 120 $rdf3x btc-db 2>/dev/null | grep -v ">" | wc -l;
done
