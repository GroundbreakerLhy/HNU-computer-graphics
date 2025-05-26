CXX = g++
CXXFLAGS = -std=c++11 -Wall
LDFLAGS = -lGL -lGLEW -lglfw -ldl

TARGET = Assignment2
SRCS = main.cpp Shader.cpp Texture.cpp

INCLUDES = -I. -I./Dependencies -I../Dependencies

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(TARGET)

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run
