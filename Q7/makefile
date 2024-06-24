CC = g++

CFLAGS = -g -Wall -std=c++11

OBJ = server.o GraphMatrix.o

all: SCC

SCC: $(OBJ)
	$(CC) $(CFLAGS) -o SCC $(OBJ)

server.o: server.cpp
	$(CC) $(CFLAGS) -c server.cpp

GraphMatrix.o: GraphMatrix.cpp GraphMatrix.hpp
	$(CC) $(CFLAGS) -c GraphMatrix.cpp

clean:
	rm -f *.o SCC

.PHONY: all clean
