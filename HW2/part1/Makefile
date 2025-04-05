CXX = g++
CXXFLAGS = -Wall -Wextra -O3 -pg -pthread -march=native -mavx2
TARGET = pi.out
SRC = pi.cpp
CPPFLAGS += -I./pcg-cpp-0.98/include
all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)
