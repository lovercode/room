# Compiler to use
CXX = g++

# Compiler flags
CXXFLAGS = -Wall -std=c++11 -pthread

# Name of the executable
TARGET = main

# Source files
SRCS = cache.cpp main.cpp cost.cpp

# Object files
OBJS = $(SRCS:.cpp=.o)

# Rule to link the executable
$(TARGET) : $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

# Rule to compile source files
.cpp.o:
	$(CXX) $(CXXFLAGS) -c $<

# Clean rule
clean:
	$(RM) $(TARGET) $(OBJS)