# Compiler
CXX = g++
CXXFLAGS = -g -std=c++11 -Wall -pthread

# Source files
SRCS = demo.cpp Reactor.cpp Proactor.cpp

# Object files
OBJS = demo.o Reactor.o Proactor.o

# Targets
TARGET = demo

all: $(TARGET)

# Rule to build the target
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

demo.o: demo.cpp Reactor.hpp Proactor.hpp
	$(CXX) $(CXXFLAGS) -c demo.cpp

Reactor.o: Reactor.cpp Reactor.hpp
	$(CXX) $(CXXFLAGS) -c Reactor.cpp

Proactor.o: Proactor.cpp Proactor.hpp 
	$(CXX) $(CXXFLAGS) -c Proactor.cpp


# Clean rule
.PHONY: all clean
clean:
	rm -f $(OBJS) $(TARGET)