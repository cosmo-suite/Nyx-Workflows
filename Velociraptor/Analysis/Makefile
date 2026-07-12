CXX = mpicxx
CXXFLAGS = -O2 -std=c++17 -Wall -Wextra

HDF5_LIB = /opt/cray/pe/hdf5-parallel/1.14.3.1/gnu/12.3/lib
HDF5_INC = /opt/cray/pe/hdf5-parallel/1.14.3.1/gnu/12.3/include

LDFLAGS = -L$(HDF5_LIB)
LDLIBS = -lhdf5

TARGET = halo_reader

SRC = \
	main.cpp \
	PropertyFileReader.cpp \
	GroupFileReader.cpp \
	ParticleFileReader.cpp \
	GadgetFileReader.cpp

OBJ = $(SRC:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(OBJ) -o $@ $(LDFLAGS) $(LDLIBS)

%.o: %.cpp VelociraptorOutputReader.H GadgetFileReader.H
	$(CXX) $(CXXFLAGS) -I$(HDF5_INC) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJ)

.PHONY: all clean
