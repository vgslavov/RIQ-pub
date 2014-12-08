OFLAG=-O3
OBJS= utils.o bloom_filter.o test.o

all: ${OBJS}
	ar r libbloom_filter.a bloom_filter.o utils.o

#all: utils.o bloom_filter.o test.o
#	g++ -o bloom_test test.o bloom_filter.o utils.o

test.o:
	g++ ${OFLAG} -c test.cc

utils.o: utils.cc
	g++ ${OFLAG} -c utils.cc

bloom_filter.o: bloom_filter.cc
	g++ ${OFLAG} -c bloom_filter.cc

clean:
	rm -rf *.o

