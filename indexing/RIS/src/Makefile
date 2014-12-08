# paths
HOME=/home/${USER}
LIBS=${HOME}/RIS/lib

# libraries
#CBF=${LIBS}/bloom_counter
#BF=${LIBS}/bloom_filter
HASHLIB=${LIBS}/hashlibpp/trunk/src
RAPTORLIB=-lraptor2
#RAPTORLIB=`pkg-config raptor2 --libs`
RASQALLIB=-lrasqal
#RASQALLIB=`pkg-config rasqal --libs`

# headers
# static only
BDB=${LIBS}/bdb
# don't use: static and shared
#BDB=${HOME}/bdb
DABF=${LIBS}/dablooms
RAPTORI=/usr/include/raptor2
#RAPTORI=`pkg-config raptor2 --cflags`
# don't use: latest rasqal from Git
#RASQALI=/usr/local/include/rasqal
# ubuntu rasqal package
RASQALI=/usr/include/rasqal
#RASQALI=`pkg-config rasqal --cflags`

# flags
#OFLAG=-O3 -pg -std=c++0x
OFLAG=-O3 -std=c++0x -Wno-write-strings
#OFLAG=-g -std=c++0x -Wno-write-strings
#OFLAG=-O3 -std=c++0x -D_NOOPT_
#OFLAG=-g

all: riq.o utils.o lsh.o cbf.o
# make -C ${CBF}
# make -C ${BF}
# make -C ${DABF}
	g++ ${OFLAG} -Wall -o riq riq.o utils.o lsh.o ${DABF}/libdablooms.a -L${HASHLIB} -lhl++ ${RAPTORLIB} ${RASQALLIB} -L${BDB} -ldb -ldb_cxx -lpthread
	g++ ${OFLAG} -Wall -o cbf cbf.o utils.o ${DABF}/libdablooms.a
# g++ ${OFLAG} -Wall -o riq riq.o utils.o lsh.o ${CBF}/libbloom_counter.a ${BF}/libbloom_filter.a ${DABF}/libdablooms.a -L${HASHLIB} -lhl++ -L${BDB} -ldb -ldb_cxx -lpthread
# g++ ${OFLAG} -Wall -o cbf cbf.o utils.o ${CBF}/libbloom_counter.a ${BF}/libbloom_filter.a ${DABF}/libdablooms.a
	cp riq ../bin/
	rm riq
	cp cbf ../bin/
	rm cbf

riq.o: riq.C
	g++ ${OFLAG} -c riq.C -I../h/ -I${BDB} -I${DABF} -I${HASHLIB} -I${RAPTORI} -I${RASQALI}

utils.o: utils.C
	g++ ${OFLAG} -c utils.C -I../h

cbf.o: cbf.C
	g++ ${OFLAG} -c cbf.C -I../h -I${DABF}

lsh.o: lsh.C
	g++ ${OFLAG} -c lsh.C -I../h -I${HASHLIB}

clean:
	rm -rf *.o *.*~
