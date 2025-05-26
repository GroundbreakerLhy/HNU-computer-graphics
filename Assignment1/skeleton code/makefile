CXX = g++
CXXFLAGS = -std=c++11 -Wall
LDFLAGS = -lGL -lGLEW -lglfw

IMGUI_DIR = ../../imgui
IMGUI_SRCS = $(IMGUI_DIR)/imgui.cpp $(IMGUI_DIR)/imgui_demo.cpp $(IMGUI_DIR)/imgui_draw.cpp \
			$(IMGUI_DIR)/imgui_tables.cpp $(IMGUI_DIR)/imgui_widgets.cpp \
			$(IMGUI_DIR)/backends/imgui_impl_glfw.cpp $(IMGUI_DIR)/backends/imgui_impl_opengl3.cpp

TARGET = Assignment1
SRCS = main.cpp $(IMGUI_SRCS)
INCLUDES = -I$(IMGUI_DIR)

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(TARGET)