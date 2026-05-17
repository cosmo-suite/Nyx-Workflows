# Compiler
CXX = mpicxx

# Compiler flags
CXXFLAGS = -O2 -std=c++17 -Wall -Wextra

# Target executable
TARGET = growth_factor.ex

# Source files
SRCS = main.cpp Cosmology.cpp

# Object files
OBJS = $(SRCS:.cpp=.o)

# Header files
HEADERS = Cosmology.h TypesAndDefs.h MT_Random.h DataBase.h	InputParser.h

# Default target
all: $(TARGET)

# Link step
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Compile step
# Recompile if either the .cpp or any listed header changes
%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean build files
clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
