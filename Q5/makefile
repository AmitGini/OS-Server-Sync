# Compiler
CXX = g++
CXXFLAGS = -std=c++11 -Wall -pthread

# Source files
SRCS = demo.cpp Reactor.cpp

# Object files
OBJS = demo.o Reactor.o

# Targets
TARGET = demo

all: $(TARGET)

# Rule to build the target
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

demo.o: demo.cpp Reactor.hpp
	$(CXX) $(CXXFLAGS) -c demo.cpp

Reactor.o: Reactor.cpp Reactor.hpp
	$(CXX) $(CXXFLAGS) -c Reactor.cpp

# Clean rule
.PHONY: all clean
clean:
	rm -f $(OBJS) $(TARGET)