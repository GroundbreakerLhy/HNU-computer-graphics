CC = g++
CFLAGS = -std=c++11 -O2
INCLUDE = -I../Dependencies
LIBDIR = -L../Dependencies/lib
LIBS = -lglfw3 -lglew32 -lopengl32 -lgdi32 -luser32 -lkernel32

# Linux 系统的库
ifeq ($(shell uname), Linux)
	LIBS = -lglfw -lGLEW -lGL -ldl
endif


TARGET = SpaceGoldCollector
SOURCES = main.cpp Shader.cpp Texture.cpp

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) $(INCLUDE) $(SOURCES) $(LIBDIR) $(LIBS) -o $(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all clean
