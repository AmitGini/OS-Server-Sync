CC = g++

CFLAGS = -g -Wall -std=c++11 -pg

OBJ = main.o GraphList.o GraphMatrix.o

all: SCC

SCC: $(OBJ)
	$(CC) $(CFLAGS) -o SCC $(OBJ)

main.o: main.cpp
	$(CC) $(CFLAGS) -c main.cpp

GraphList.o: GraphList.cpp GraphList.hpp
	$(CC) $(CFLAGS) -c GraphList.cpp

GraphMatrix.o: GraphMatrix.cpp GraphMatrix.hpp
	$(CC) $(CFLAGS) -c GraphMatrix.cpp

clean:
	rm -f *.o SCC gmon.out *.txt

.PHONY: all clean