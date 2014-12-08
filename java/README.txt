1. COMPILE

	$ cd java
	$ javac  -Djava.ext.dirs=libs edu/umkc/sce/dbis/anask/Querier.java

(2. CREATE A MANIFEST FILE CALLED MANIFEST.MF)

WITH THE FOLLOWING CONTENT:

Manifest-Version: 1.0
Main-Class: edu.umkc.sce.dbis.anask.Querier
Class-Path: libs/commons-cli-1.2-javadoc.jar libs/commons-lang3-3.1.jar  libs/jackson-core-2.3.3.jar  libs/jena-iri-1.1.1.jar
 libs/log4j-1.2.17.jar libs/commons-cli-1.2-sources.jar libs/httpclient-4.2.6.jar
 libs/jackson-databind-2.3.3.jar libs/jena-sdb-1.5.1.jar libs/slf4j-api-1.7.6.jar
 libs/commons-cli-1.2.jar  libs/httpclient-cache-4.2.6.jar libs/jcl-over-slf4j-1.7.6.jar
 libs/jena-tdb-1.1.1.jar  libs/slf4j-log4j12-1.7.6.jarlibs/commons-codec-1.6.jar
 libs/httpcore-4.2.5.jar  libs/jena-arq-2.12.1.jar  libs/jsonld-java-0.5.0.jar
 libs/xercesImpl-2.11.0.jarlibs/commons-csv-1.0.jar
 libs/jackson-annotations-2.3.0.jar libs/jena-core-2.12.1.jar  libs/libthrift-0.9.1.jar  libs/xml-apis-1.4.01.jar
	
3. CREATE JAR FILE
	
	$ jar -cvfm querier.jar MANIFEST.MF edu/umkc/sce/dbis/anask/Querier.class

--------------------------------------------------------------------------------

FOR HELP:

	$ java -jar querier.jar -h

RUN EXAMPLE:

	$ java -Dlog4j.configuration=file:jena-log4j.properties -jar querier.jar -results xml -index /Users/anask/indexes/ -query /Users/anask/q1.q -candidates indsFile.txt
