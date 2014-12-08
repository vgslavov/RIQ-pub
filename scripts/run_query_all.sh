#!/bin/bash

if [ $# -lt 4 -o $# -gt 6 ]; then
   echo "Usage: $0 <index dir> <query file> <rdf3x|tdb> <cold|warm> (<avg times> <results format>)"
   exit
fi

#set -x 

INDEX_PATH=$1
INDEX_FILE=`basename $INDEX_PATH`
QUERY=$2
QUERY_WITH=$3
CACHE=$4

if [ "$QUERY_WITH" != "rdf3x" -a "$QUERY_WITH" != "tdb" ]; then
  echo "Invalid refinement tool"
  exit
fi

if [ "$CACHE" != "cold" -a "$CACHE" != "warm" ]; then
  echo "Invalid cache setting"
  exit
fi

if [ $# -ge 5 ]; then
  MAX=$5
else
  MAX=3
fi

if [ $# -eq 6 ]; then
  FORMAT=$6
else
  FORMAT="csv"
fi

QUERY_FILE=`basename $QUERY`

USER="vsfgd"
CLEAR_CACHE="/home/${USER}/bin/run_clear_cache.sh"
TIMEV="/usr/bin/time -v"
SCL="scale=3"

# query tools
JENADIR="/home/${USER}/Jena/apache-jena-2.11.1"
JENAQUERY="$JENADIR/bin/tdbquery"
#JENASYM="--set tdb:fileMode=direct"
#JENASYM="--set tdb:fileMode=mapped"
RDF3XDIR="/home/${USER}/rdf3x-0.3.8-vsfgd"
RDF3XQUERY="$RDF3XDIR/bin/rdf3xquery"

if [ "$CACHE" == "warm" ]; then
  LOG_PRE=$QUERY_FILE.cold4warm.$QUERY_WITH.$FORMAT
  # XXX: call outside manually or from calling script
  echo "clearing cache..."
  $CLEAR_CACHE
  #echo "call run_clear_cache if not invoking from a script!"

  start_time=$(date +%s%N)

  if [ "$QUERY_WITH" == "tdb" ]; then
    echo "running Jena TDB to warm up cache..."
    # *.timev is overwritten
    # *.results and *.err ARE NOT
    $TIMEV -o $LOG_PRE.timev $JENAQUERY --results $FORMAT --time $JENASYM --loc=$INDEX_PATH --query=$QUERY >> $LOG_PRE.results 2>> $LOG_PRE.err
  else
    echo "running RDF-3X to warm up cache..."
    $TIMEV -o $LOG_PRE.timev $RDF3XQUERY $INDEX_PATH $QUERY >> $LOG_PRE.results 2>> $LOG_PRE.err
  fi

  end_time=$(date +%s%N)
  bash_time=$(($end_time-$start_time))
  bash_time=`echo "$SCL;"$bash_time"/1000000000.0" | bc`
  timev_time=`grep "Elapsed (wall clock) time" $LOG_PRE.timev|awk '{print $NF}'`
  major_pf=`grep "Major (requiring I/O) page faults" $LOG_PRE.timev|awk '{print $NF}'`
  minor_pf=`grep "Minor (reclaiming a frame) page faults" $LOG_PRE.timev|awk '{print $NF}'`

  echo -n "bash: $bash_time"
  echo -n ", time-v: $timev_time"

  echo -n "index: $INDEX_FILE" >> $LOG_PRE.time
  echo -n " bash: $bash_time" >> $LOG_PRE.time
  echo -n " time-v: $timev_time" >> $LOG_PRE.time

  if [ "$QUERY_WITH" == "tdb" ]; then
    # get last match only
    jena_time=`grep -e "^Time:" $LOG_PRE.err|tail -n1|awk '{print $2}'`
    echo -n ", jena: $jena_time"
    echo -n " jena: $jena_time" >> $LOG_PRE.time
  fi

  echo -n ", major-pf: $major_pf"
  echo -n ", minor-pf: $minor_pf"
  echo

  echo -n " major-pf: $major_pf" >> $LOG_PRE.time
  echo -n " minor-pf: $minor_pf" >> $LOG_PRE.time
  echo >> $LOG_PRE.time
fi

LOG_PRE=$QUERY_FILE.$CACHE.$QUERY_WITH.$FORMAT

total_bash_time=0
total_jena_time=0
total_major_pf=0
total_minor_pf=0
i=1
while [ $i -le $MAX ]; do
  if [ "$CACHE" == "cold" ]; then
    echo "clearing cache..."
    $CLEAR_CACHE
  fi

  echo -n "index: $INDEX_FILE" >> $LOG_PRE.$i.time

  start_time=$(date +%s%N)

  if [ "$QUERY_WITH" == "tdb" ]; then
    echo "running Jena TDB ($i/$MAX times)..."
    # *.timev is overwritten
    # *.results and *.err ARE NOT (for multiple indices of candidates)
    $TIMEV -o $LOG_PRE.$i.timev $JENAQUERY --results $FORMAT --time $JENASYM --loc=$INDEX_PATH --query=$QUERY >> $LOG_PRE.$i.results 2>> $LOG_PRE.$i.err
  else
    echo "running Jena TDB ($i/$MAX times)..."
    $TIMEV -o $LOG_PRE.$i.timev $RDF3XQUERY $INDEX_PATH $QUERY >> $LOG_PRE.$i.results 2>> $LOG_PRE.$i.err
  fi

  end_time=$(date +%s%N)
  bash_time=$(($end_time-$start_time))
  total_bash_time=$(($total_bash_time+$bash_time))
  bash_time=`echo "$SCL;"$bash_time"/1000000000.0" | bc`
  # TODO: add up timev times (requires conversions)
  timev_time=`grep "Elapsed (wall clock) time" $LOG_PRE.$i.timev|awk '{print $NF}'`

  echo -n "bash: $bash_time"
  echo -n ", time-v: $timev_time"

  echo -n " bash: $bash_time" >> $LOG_PRE.$i.time
  echo -n " time-v: $timev_time" >> $LOG_PRE.$i.time

  if [ "$QUERY_WITH" == "tdb" ]; then
    # get last match only
    jena_time=`grep -e "^Time:" $LOG_PRE.$i.err|tail -n1|awk '{print $2}'`
    total_jena_time=`echo "$SCL;$total_jena_time+$jena_time" | bc`
    echo -n ", jena: $jena_time"
    echo -n " jena: $jena_time" >> $LOG_PRE.$i.time
  fi

  major_pf=`grep "Major (requiring I/O) page faults" $LOG_PRE.$i.timev|awk '{print $NF}'`
  minor_pf=`grep "Minor (reclaiming a frame) page faults" $LOG_PRE.$i.timev|awk '{print $NF}'`
  total_major_pf=`echo "$SCL;$total_major_pf+$major_pf" | bc`
  total_minor_pf=`echo "$SCL;$total_minor_pf+$minor_pf" | bc`

  echo -n ", major-pf: $major_pf"
  echo -n ", minor-pf: $minor_pf"
  echo

  echo -n " major-pf: $major_pf" >> $LOG_PRE.$i.time
  echo -n " minor-pf: $minor_pf" >> $LOG_PRE.$i.time
  echo >>  $LOG_PRE.$i.time

  i=`expr $i + 1`
done

FINAL_LOG=$LOG_PRE.avg.time

echo -n "index: $INDEX_FILE" >> $FINAL_LOG

total_bash_time=`echo "$SCL;"$total_bash_time"/1000000000.0" | bc`
#echo -n " bash-total: $total_bash_time" >> $FINAL_LOG
#echo -n " jena-total: $total_jena_time" >> $FINAL_LOG

avg_bash_time=`echo "$SCL;"$total_bash_time"/$MAX.0" | bc`
echo -n " bash-avg: $avg_bash_time" >> $FINAL_LOG

avg_jena_time=`echo "$SCL;"$total_jena_time"/$MAX.0" | bc`
echo -n " jena-avg: $avg_jena_time" >> $FINAL_LOG

avg_major_pf=`echo "$SCL;"$total_major_pf"/$MAX.0" | bc`
avg_minor_pf=`echo "$SCL;"$total_minor_pf"/$MAX.0" | bc`
echo -n " major-pf-avg: $avg_major_pf" >> $FINAL_LOG
echo -n " minor-pf-avg: $avg_minor_pf" >> $FINAL_LOG
echo

du=`du -s $INDEX_PATH | awk '{print $1}'`
echo -n " du: $du" >> $FINAL_LOG
echo >> $FINAL_LOG

echo "run_query_all:"
cat $FINAL_LOG
