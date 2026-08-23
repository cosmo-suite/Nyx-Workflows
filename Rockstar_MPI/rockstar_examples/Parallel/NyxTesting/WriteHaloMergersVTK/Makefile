CC       = mpicc
CXX      = mpicxx

CFLAGS   = -m64 -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -Wall -O3 -std=c99
CXXFLAGS = -m64 -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -Wall -O3 -std=c++17
LDFLAGS  = -lm

C_SRC    = read_tree.c stringparse.c check_syscalls.c
CXX_SRC  = main.cpp

C_OBJ    = $(C_SRC:.c=.o)
CXX_OBJ  = $(CXX_SRC:.cpp=.o)
OBJ      = $(C_OBJ) $(CXX_OBJ)

TARGET   = parse_trees.exe

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(OBJ) $(LDFLAGS) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJ) *~

.PHONY: all clean
