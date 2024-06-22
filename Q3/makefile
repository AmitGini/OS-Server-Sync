CC = g++

CFLAGS = -g -Wall -std=c++11

OBJ = main.o IOHandler.o GraphMatrix.o

all: SCC

SCC: $(OBJ)
	$(CC) $(CFLAGS) -o SCC $(OBJ)

main.o: main.cpp
	$(CC) $(CFLAGS) -c main.cpp

IOHandler.o: IOHandler.cpp IOHandler.hpp
	$(CC) $(CFLAGS) -c IOHandler.cpp

GraphMatrix.o: GraphMatrix.cpp GraphMatrix.hpp
	$(CC) $(CFLAGS) -c GraphMatrix.cpp

clean:
	rm -f *.o SCC gmon.out

.PHONY: all clean
