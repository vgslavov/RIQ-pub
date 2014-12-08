CC=gcc
OFLAG=-O3
#OFLAG=-O3 -pg
#OFLAG=-O3
OBJS = dablooms.o

all: ${OBJS}
	ar r libdablooms.a dablooms.o murmur.o

dablooms.o: dablooms.c
	${CC} ${OFLAG} -c dablooms.c murmur.c -lm

murmur.o: murmur.c
	${CC} ${OFLAG} -c murmur.c -lm

clean: 
	rm -rf *.o
