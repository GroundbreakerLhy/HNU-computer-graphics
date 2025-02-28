CXX = g++
CXXFLAGS = -std=c++11 -Wall
LDFLAGS = -lGL -lGLEW -lglfw

TARGET = Demo
SRCS = main.cpp

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(TARGET)