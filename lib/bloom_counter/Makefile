OFLAG=-O3
OBJS = utils.o bloom_counter.o test_2.o
all: ${OBJS}
	#g++ ${OFLAG} -o bloom_counter bloom_counter.o utils.o
	ar r libbloom_counter.a bloom_counter.o utils.o
test_2.o:
	g++ ${OFLAG} -c test_2.cc

utils.o: utils.cc
	g++ ${OFLAG} -c utils.cc

bloom_counter.o: bloom_counter.cc
	g++ ${OFLAG} -c bloom_counter.cc

clean: 
	rm -rf *.o
