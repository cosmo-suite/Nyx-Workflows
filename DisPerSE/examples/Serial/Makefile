# Compiler and flags
CXX      := mpicxx
CXXFLAGS := -Wall -Wextra -std=c++17 -O2

# Target executable name
TARGET   := out

# Source files and header dependencies
SRCS     := main.cpp NDfield.cpp
OBJS     := $(SRCS:.cpp=.o)
HEADERS  := NDfield.h

# Default rule to build the target
all: $(TARGET)

# Link object files into executable
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Compile source files into object files (tracks header changes)
%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up build artifacts
clean:
	rm -f $(OBJS) $(TARGET)

# Phony targets to prevent conflicts with files named 'all' or 'clean'
.PHONY: all clean
