CC       = mpicc
CXX      = mpicxx

CFLAGS   = -m64 -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -Wall -O3 -std=c99
CXXFLAGS = -m64 -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -Wall -O3 -std=c++17
LDFLAGS  = -lm

# Explicitly list only the target source files
C_SRC    = read_tree.c stringparse.c check_syscalls.c
CXX_SRC  = main.cpp Halo.cpp IO.cpp 

# Explicitly list header files for dependency tracking
HEADERS  = read_tree.h stringparse.h check_syscalls.h Halo.H IO.H

# Object files derived strictly from the defined sources above
C_OBJ    = $(C_SRC:.c=.o)
CXX_OBJ  = $(CXX_SRC:.cpp=.o)
OBJ      = $(C_OBJ) $(CXX_OBJ)

TARGET   = parse_trees.exe

all: $(TARGET)

# Link step
$(TARGET): $(OBJ)
	$(CXX) $(OBJ) $(LDFLAGS) -o $@

# C compilation rule (rebuilds if any C source or header changes)
%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

# C++ compilation rule (rebuilds if any C++ source or header changes)
%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJ) *~

.PHONY: all clean
