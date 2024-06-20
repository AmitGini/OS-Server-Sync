CC = g++

CFLAGS = -g -Wall -std=c++11

all: SCC

SCC: main.o IOHandler.o Graph.o
	$(CC) $(CFLAGS) -o SCC main.o IOHandler.o Graph.o

main.o: main.cpp
	$(CC) $(CFLAGS) -c main.cpp

IOHandler.o: IOHandler.cpp IOHandler.hpp
	$(CC) $(CFLAGS) -c IOHandler.cpp

Graph.o: Graph.cpp Graph.hpp
	$(CC) $(CFLAGS) -c Graph.cpp

clean:
	rm -f *.o SCC

.PHONY: all clean