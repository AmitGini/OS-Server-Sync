# Compiler
CXX = g++
CXXFLAGS = -g -std=c++11 -Wall -pthread

# Object files
OBJS = main.o Reactor.o Proactor.o GraphMatrix.o

# Targets
TARGET = main

all: $(TARGET)

# Rule to build the target
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

main.o: main.cpp Reactor.hpp Proactor.hpp GraphMatrix.hpp
	$(CXX) $(CXXFLAGS) -c main.cpp

GraphMatrix.o: GraphMatrix.cpp GraphMatrix.hpp 
	$(CXX) $(CXXFLAGS) -c GraphMatrix.cpp

Reactor.o: Reactor.cpp Reactor.hpp 
	$(CXX) $(CXXFLAGS) -c Reactor.cpp

Proactor.o: Proactor.cpp Proactor.hpp 
	$(CXX) $(CXXFLAGS) -c Proactor.cpp

# Clean rule
.PHONY: all clean
clean:
	rm -f $(OBJS) $(TARGET)