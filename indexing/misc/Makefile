# Author: DBIS Lab
# UMKC, 2012
CFLAGS=-g
OBJS=compute_connectedcomponents_ntriples.o convert_to_gpmetis.o convert_to_graphviz.o
all: $(OBJS)
	g++ -Wall -o compute_connectedcomponents_ntriples compute_connectedcomponents_ntriples.o
	g++ -Wall -o convert_to_gpmetis convert_to_gpmetis.o
	g++ -Wall -o convert_to_graphviz convert_to_graphviz.o
%.o: %.cc
	g++ ${CFLAGS} -c $< -o $@
clean:
	rm -rf *.o
