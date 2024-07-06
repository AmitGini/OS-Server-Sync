# Compiler
CC = g++
CFLAGS = -std=c++11 -g -Wall -pthread

# Object files
OBJ = server.o GraphMatrix.o

all: SCC

SCC: $(OBJ)
	$(CC) $(CFLAGS) -o SCC $(OBJ)

server.o: server.cpp
	$(CC) $(CFLAGS) -c server.cpp

GraphMatrix.o: GraphMatrix.cpp GraphMatrix.hpp
	$(CC) $(CFLAGS) -c GraphMatrix.cpp


# Clean rule
.PHONY: all clean

clean:
	rm -f $(OBJ) SCC