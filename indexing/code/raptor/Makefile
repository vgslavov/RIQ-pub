INCLUDES = `pkg-config raptor2 --cflags`
LIBS = `pkg-config raptor2 --libs`
CC = gcc
CFLAGS = -O3 -w -fpermissive

all: rdfprint rdfserialize rdfcat

rdfprint:
	$(CC) -o rdfprint rdfprint.c $(CFLAGS) $(INCLUDES) $(LIBS)

rdfserialize:
	$(CC) -o rdfserialize rdfserialize.c $(CFLAGS) $(INCLUDES) $(LIBS)

rdfcat:
	$(CC) -o rdfcat rdfcat.c $(CFLAGS) $(INCLUDES) $(LIBS)

clean:
	-rm -f *.o
	-rm -f rdfprint rdfserialize rdfcat
