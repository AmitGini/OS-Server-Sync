CC = g++

CFLAGS = -g -Wall -std=c++11

OBJ = pollserver.o GraphMatrix.o

all: SCC

SCC: $(OBJ)
	$(CC) $(CFLAGS) -o SCC $(OBJ)

main.o: pollserver.cpp
	$(CC) $(CFLAGS) -c pollserver.cpp

GraphMatrix.o: GraphMatrix.cpp GraphMatrix.hpp
	$(CC) $(CFLAGS) -c GraphMatrix.cpp

clean:
	rm -f *.o SCC

.PHONY: all clean
