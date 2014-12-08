# !/bin/sh
#
# USAGE: splodge.sh <config>
#################################################

# set main class
mainclass=de.uniko.west.splodge.QueryGen

# set binaries path
classpath=./bin

# set libary path
lib_path=./lib

# add all jars in library path to classpath
for jar in $lib_path/*.jar; do classpath=$classpath:$jar; done

# run main class (with arguments)
java -cp $classpath $mainclass $*

