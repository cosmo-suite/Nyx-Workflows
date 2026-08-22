CXX = mpicxx
CXXFLAGS = -O2 -std=c++17 -Wall -Wextra

TARGET = write_halos_vtk

SRC = \
	GadgetFileReader.cpp \
	main.cpp \

OBJ = $(SRC:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(OBJ) -o $@ $(LDFLAGS) $(LDLIBS)

%.o: %.cpp GadgetFileReader.H
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJ)

.PHONY: all clean
