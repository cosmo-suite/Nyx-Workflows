CXX = mpicxx
CXXFLAGS = -O2 -std=c++17 -Wall -Wextra

TARGET = write_halos_vtk

SRC = \
	main.cpp \
    Halo.cpp \
	GadgetFileReader.cpp \
    IO.cpp \

OBJ = $(SRC:.cpp=.o)

all: $(TARGET)

LDFLAGS =
LDLIBS =

$(TARGET): $(OBJ)
	$(CXX) $(OBJ) -o $@ $(LDFLAGS) $(LDLIBS)

%.o: %.cpp GadgetFileReader.H Halo.H IO.H
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJ)

.PHONY: all clean
