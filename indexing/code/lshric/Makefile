TARGET = lshric
INDEXING = ${HOME}/RIS/indexing/
# make sure protobuf is configured with "configure --prefix=/usr"
INCLUDES = -I${INDEXING}/hashlibpp/trunk/src
# need -pthread and -lpthread?
# check output of 'pkg-config --cflags --libs protobuf'
#LIBS = -L${HOME}/HashLib2Plus/trunk/src -lhl++ -lprotobuf -lz
LIBS = -L${INDEXING}/hashlibpp/trunk/src -lhl++
CC = g++
CFLAGS = -w
#CFLAGS = -g -Wall

default: $(TARGET)
all: default

OBJECTS = $(patsubst %.cc, %.o, $(wildcard *.cc))
HEADERS = $(wildcard *.h)

%.o: %.cc $(HEADERS)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

.PRECIOUS: $(TARGET) $(OBJECTS)

# run manually!
#protoc_middleman: ris.proto
#	protoc --cpp_out=. ris.proto
#	@touch protoc_middleman

#pkg-config --cflags protobuf	# fails if protobuf is not installed
$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -Wall $(LIBS) -o $@

clean:
	-rm -f *.o
	-rm -f $(TARGET)
