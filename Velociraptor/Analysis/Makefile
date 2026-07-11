CXX = mpicxx
CXXFLAGS = -O2 -std=c++17 -Wall -Wextra

HDF5_LIB = /opt/cray/pe/hdf5-parallel/1.14.3.1/gnu/12.3/lib
HDF5_INC = /opt/cray/pe/hdf5-parallel/1.14.3.1/gnu/12.3/include

LDFLAGS = -L$(HDF5_LIB)
LDLIBS = -lhdf5

TARGET = halo_reader
SRC = main.cpp

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -I$(HDF5_INC) $^ -o $@ $(LDFLAGS) $(LDLIBS)

clean:
	rm -f $(TARGET)
