#!/bin/sh
#-------------------------------------------------------------------------
##########################################################################

DATADIR=test-sources/
CONSOLE=/usr/lib/openrdf-sesame-2.6.6/bin/console.sh
#REPODIR=db/sesame/openrdf-sesame

#echo "show repositories." | $CONSOLE --dataDir $SESDIR | grep myrepo || \
#  echo "create native.\nmyrepo\n\n\nexit.\n" | $CONSOLE --dataDir $SESDIR

#export JAVA_OPTS="-Dinfo.aduna.platform.appdata.basedir=db/sesame -Xmx256M"
#export JRE_HOME=/usr/lib/jvm/java-6-openjdk/jre
#export CATALINA_BASE=conf/tomcat
#exec /tomcat/bin/catalina.sh run

# handle all files from argument list
echo "processing $# files..." >&2;
for i in $@; do

  # load rdf file
  name=${i%*.nt.gz};
  name=${name##*/};
  echo `date +%X` "$i" >&2;

  echo "create native.\n$name\n\n\nopen $name.\nload $i.\nclose.\nexit.\n" | $CONSOLE >>log.txt
done

echo `date +%X` "loaded all repositories." >&2;
echo "show repositories." | $CONSOLE >>log.txt
