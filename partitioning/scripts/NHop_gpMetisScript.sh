#! /bin/sh

# Arguments:
# $1 -> Path to the dataset (dir w/ sorted_yago)
# $2 -> Number of Partitions
# $3 -> Strategy Type (T1/T2/T3/T4)
# $4 -> Include Edge Weights (Y/N) --usually for hmetis it's N
#
# check for CR -> vi -b orgfile | file orgfile
# remove CR from file -> tr -d '\r' < orgfile > newfile
# Prefix -> head -5 datasetfile > Prefix
# tempfile -> sed '1,5d' datasetfile > tempfile
# sorted_yago -> sort -k1,1 tempfile > sorted_yago
# u_subj_yago -> cut -f1 -d ' ' sorted_yago | uniq > u_subj_yago

if [ $# -ne 6 ]; then
	echo "Usage: $0 <part. code dir> <data dir> <# of parts> <# of hops> <strategy: T1|T2|T3|T4> <edge weights: Y|N>"
	exit 1;
fi

# args
PROJECT=$1
DATADIR=$2
NPARTS=$3
NHOPS=$4
STRATEGY=$5
EDGEWBOOL=$6

# dirs
BIN="$PROJECT/bin"
SRC="$PROJECT/src"
LIB="$PROJECT/lib"
SCRIPTS="$PROJECT/scripts"
GPMETISDIR="$PROJECT/metis-5.0.1/build/Linux-x86_64/programs"
METIS="$DATADIR/metis"				# folder that contains metis/output related files
LOGDIR="$DATADIR/log"

# bin/src
#NHOPSRC="$SRC/nhop-bfs.cc"
NHOPSRC="$SRC/nhop-hood.cc"
#NHOPBIN="$BIN/nhop-bfs"
NHOPBIN="$BIN/nhop-hood"
GPMETIS="$GPMETISDIR/gpmetis"
CONNECTEDSRC="$SRC/conn_components.cc"
CONNECTEDBIN="$BIN/conn_components"
CONVERT2GRAPHVIZSRC="$SRC/convert_to_graphviz.cc"
CONVERT2GRAPHVIZBIN="$BIN/convert_to_graphviz"

# input
SORTEDINPUT="$DATADIR/sorted_yago"		# sorted file for whole dataset
SUBJECTS="$DATADIR/subjects"
PREDICATES="$DATADIR/predicates"
OBJECTS="$DATADIR/objects"
SORTEDV="$DATADIR/vertices"			# file w/ unique subjects and objects from non-STAR graph
ACTUALFILE="$DATADIR/ActualFile"
RDFTYPE="$DATADIR/RdfType"

# output
QFILE="$METIS/queuefile"
GISNHOP="$METIS/GIS_nhop"
GPMETISIN="$METIS/MetisFile"			# input file for gpmetis
GPMETISOUT="$METIS/MetisFile.part.$NPARTS"	# output file for gpmetis
CUTLISTID="$METIS/CutList_ID"
CUTLISTIDTMP="$METIS/CutList_ID.TMP"
CONNECTEDNT="$METIS/connected_ntriples"
OUTPART="$METIS/hop${NHOPS}part"
OUTPUTID="$METIS/Output_ID"
VERTEXW="$METIS/vertexw"
EDGEW="$METIS/edgew"
PREGRAPH="$METIS/PreGraph"
PRERDFTYPE="$METIS/PreRdfType"
EDGECUTS="$METIS/EdgeCuts"

# cmds
TIME="/usr/bin/time -v"

# parameters
CUTOFFW=30000

# redirect stderr and stdout
if [ ! -d "$LOGDIR" ] ; then
	mkdir $LOGDIR
fi
exec 1>>$DATADIR/log/log.$NPARTS.log
exec 2>>$DATADIR/log/log.$NPARTS.debug

STARTALL=$(date +%s)
echo "Start time: $(date)"

echo "Compiling code..\n"
if [ ! -d "$BIN" ] ; then
	mkdir $BIN
fi
javac -d $BIN -cp .:$LIB/jdbm-2.2.jar $SRC/gpmetis1.java
javac -d $BIN -cp .:$LIB/jdbm-2.2.jar $SRC/nHop_1.java
#javac -d $BIN -cp .:$LIB/jdbm-2.2.jar $SRC/nHop_2.java
javac -d $BIN -cp .:$LIB/jdbm-2.2.jar $SRC/PreProcess.java
# unordered_map requires c++11
g++ -std=c++0x -o $NHOPBIN $NHOPSRC
#g++ -o $NHOPBIN $NHOPSRC
g++ -o $CONNECTEDBIN $CONNECTEDSRC
g++ -o $CONVERT2GRAPHVIZBIN $CONVERT2GRAPHVIZSRC

START=$(date +%s)

echo "Actual execution started.. Take a Tea Break!\n"
if [ ! -d "$METIS" ] ; then
	mkdir $METIS
fi
# fix this
cp $SCRIPTS/sort.sh $METIS/
cp $SCRIPTS/getuniq.sh $METIS/
if [ ! -d "$METIS/Databases" ] ; then
	mkdir $METIS/Databases
fi

cut -f 1 -d ' ' $SORTEDINPUT | sort | uniq > $SUBJECTS
cut -f 2 -d ' ' $SORTEDINPUT | sort | uniq > $PREDICATES
cut -f 3- -d ' ' $SORTEDINPUT | sort | uniq > $OBJECTS
# append objects to subjects?
sed 's/\(.*\)../\1/' $OBJECTS >> $SUBJECTS
# all vertices (unique subjects+objects)
sort $SUBJECTS | uniq > $SORTEDV
# num. of vertices
NODES=$(wc -l $SORTEDV|cut -f1 -d ' ')	

END=$(date +%s)
DIFF=$(( $END - $START ))
echo "preprocessing took $DIFF seconds"

# Name: gpmetis1.java
# Purpose: Prepare gpmetis input (convert to IDs)
#
# INPUT:
# - $SORTEDINPUT (sorted_yago)
# - $SORTEDV (vertices)
# - $PREDICATES (predicates)
# OUTPUT:
# - ActualFile (sorted_yago - rdftype)
# - RdfType (sorted_yago - actualfile)
# - (EdgeWeights)
# - $GISNHOP (GIS_nhop)
# - GIS_hmetis
# - PreGraph{1:sorted-by-subject,2:sorted-by-object} (ActualFile in ID format)
# - PreRdfType{1,2}
# - $GPMETISIN (MetisFile)
#
# Usage:
# 1: Path to the original connected Graph (SplFile)
# 2: List of Unique Subjects and Objects (vertices)
# 3: Destination path
# 4: Actual File that is to be converted to a Graph (ActualFile)
# 5: File that was separated out from the Actual file (RdfType)
# 6: Type of Stragery used
# 7: Use Edge Weights (Y/N)
# 8: Path to the file that has edge weights
# 9: Complete Dataset in GIS Format (GIS_nhop)
# 10: List of Predicates
# 11: Number of vertices

START=$(date +%s)

cd $BIN
java -Xmx2g -cp .:$LIB/jdbm-2.2.jar gpmetis1 $SORTEDINPUT $SORTEDV $METIS/ $ACTUALFILE $RDFTYPE $STRATEGY $EDGEWBOOL $EDGEW $GISNHOP $PREDICATES $NODES

END=$(date +%s)
DIFF=$(( $END - $START ))
echo "gpmetis1 took $DIFF seconds"

# Name: gpmetis
# Purpose: partition graph
#
# INPUT: $GPMETISIN (MetisFile)
# OUTPUT: $GPMETISIN.part.$NPARTS (MetisFile.part.2)

START=$(date +%s)

echo "\nRunning gpmetis..\n"
$GPMETIS $GPMETISIN $NPARTS
# optimizations
#$GPMETIS -minconn -contig -objtype=vol $GPMETISIN $NPARTS
#echo "\nFinished executing gpmetis...!\n"

END=$(date +%s)
DIFF=$(( $END - $START ))
echo "gpmetis took $DIFF seconds"

# Name: nHop_1
# Purpose: convert IDs in gpmetis partitions to original data
#
# INPUT:
# - $GPMETISIN.part.$NPARTS (MetisFile.part.2)
# - PreGraph
# - PreRdfType
# - MetisFile
# OUTPUT:
# - Part0, Part1 (gpmetis partitions converted to native ID format)
# - queuefile (IDs of vertices in each partition/line)
# - //queuefile (IDs of vertices connected by cut edges)
# - //edgesID (IDs of cut edges)
#
# Usage:
# 1: File that has triples from Actual File in ID's format (PreGraph)
# 2: Output from Metis (hmetisFile.part.[?]/MetisFile.part.[?])
# 3: Destination path
# 4: Number to Partitions
# 5: Strategy Type (T1/T2)
# 6: File that has triples from RdfType File in ID's format (PreRdfType)
# 7: Input to gpmetis (MetisFile)
# 8: QueueFile (queuefile)
# 9: Use Edge Weights (Y/N)
# 10: vertices file

START=$(date +%s)

cd $BIN
java -Xmx4g -cp .:$LIB/jdbm-2.2.jar nHop_1 $PREGRAPH $GPMETISOUT $METIS/ $NPARTS $STRATEGY $PRERDFTYPE $GPMETISIN $QFILE $EDGEWBOOL $SORTEDV

END=$(date +%s)
DIFF=$(( $END - $START ))
echo "nHop_1 took $DIFF seconds"

# Name: nhop
# Purpose: perform n-hop
#
# INPUT:
# - GIS_nhop (complete dataset in GIS format)
# - queuefile
# OUTPUT:
# - CutList_ID
# - ProgNHop.tmp
#
# new Usage: <input file> <queue file> <CutList_ID> <ProgressFile> <VertexWeights> <Cutoff-Weight>
#
# old Usage: <input file> <queue file> <CutList_ID> <ProgressFile>

START=$(date +%s)

$NHOPBIN -g $GISNHOP -p $GPMETISIN.part.$NPARTS -o $OUTPART -M $CUTOFFW -H $NHOPS -P $NPARTS
#echo "\nBFS for nHop...\n"
#$NHOPBIN $GISNHOP $QFILE $CUTLISTID $METIS/ProgNHop.tmp $VERTEXW $CUTOFFW $NHOPS
# old args
#$NHOPBIN $GISNHOP $QFILE $CUTLISTID $METIS/ProgNHop.tmp

END=$(date +%s)
DIFF=$(( $END - $START ))
echo "nhop took $DIFF seconds"

START=$(date +%s)

echo "sorting nhop output..."

n=0
while [ $n -lt $NPARTS ]; do
	sort $OUTPART.$n | uniq > $OUTPART.$n.su
	n=`expr $n + 1`
	rm -f $OUTPART.$n
done

# will this work for large $NPARTS?
alloutput=`ls $OUTPART.*.su | sort -n`

echo "cat-ing nhop output..."

cat $alloutput >> $CUTLISTID

END=$(date +%s)
DIFF=$(( $END - $START ))
echo "sort/cat took $DIFF seconds"

# Name: conn_components
# Purpose: find disconnected graphs
#
# INPUT:
# - CutList_ID
# OUTPUT:
# - connected_ntriples
#
# Usage: ./connected_out <input file> <output filepath>

START=$(date +%s)

echo "\nConnected components...\n"
$CONNECTEDBIN $CUTLISTID $METIS/

END=$(date +%s)
DIFF=$(( $END - $START ))
echo "conn_components took $DIFF seconds"

# Name: PreProcess
# Purpose: convert IDs in CutList_ID & edgesID to original data
#
# INPUT:
# - connected_ntriples
# OUTPUT:
# - Output_ID
# - AddRdfType
# - EdgeCuts
#
# Usage:  <DestinationFilePath> <connectedcomps> <Vertices> <OutputFile> <PredicatesFile> <VerticesCount> <EdgeCuts>

START=$(date +%s)

cd $BIN
java -cp .:$LIB/jdbm-2.2.jar PreProcess $METIS/ $CONNECTEDNT $SORTEDV $OUTPUTID $PREDICATES $NODES $EDGECUTS

END=$(date +%s)
DIFF=$(( $END - $START ))
echo "PreProcess took $DIFF seconds"

# Name: nHop_2
# Purpose: no longer needed?
#
# INPUT:
# OUTPUT: 
#
# Usage:
# 1: Destination path
# 2: File with unique subjects and objects
# 3: Number to Partitions
# 4: File with list of EdgeCuts
# 5: List of predicates
# 6: number of vertices

#cd $BIN
#java -Xmx4g -cp .:$LIB/jdbm-2.2.jar nHop_2 $METIS/ $SORTEDV $NPARTS $METIS/EdgesCut $PREDICATES $NODES

echo "\nCongratulations! Partitions are ready.."
ENDALL=$(date +%s)
echo "EndTime: $(date)"
DIFF=$(( $ENDALL - $STARTALL ))
echo "It took $DIFF seconds"
