TARGET = Rdf_Sig_Gen
#INCLUDES = `pkg-config raptor2 --cflags`
#LIBS = `pkg-config raptor2 --libs`
CC = g++
CFLAGS = -O3 -w
#CFLAGS = -O3 -w -fpermissive
#CFLAGS = -g -Wall

default: $(TARGET)
all: default

OBJECTS = $(patsubst %.cc, %.o, $(wildcard *.cc))
HEADERS = $(wildcard *.h)

%.o: %.cc $(HEADERS)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

.PRECIOUS: $(TARGET) $(OBJECTS)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -Wall $(LIBS) -o $@

clean:
	-rm -f *.o
	-rm -f $(TARGET)
