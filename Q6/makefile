# Compiler
CXX = g++
CXXFLAGS = -std=c++11 -Wall -pthread

# Source files
SRCS = GraphMatrix.cpp server.cpp Reactor.cpp

# Object files
OBJS = $(SRCS:.cpp=.o)

# Target executable
TARGET = SCC

# Default target
all: $(TARGET)

# Rule to build the target
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

# Pattern rule to compile .cpp files to .o files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean rule
.PHONY: all clean
clean:
	rm -f $(OBJS) $(TARGET)