# Define the C++ compiler
CXX = g++

# Compiler flags
CXXFLAGS = -O3 -Wall -I./deps/raylib-cpp/include -I./deps/raudio/src

# Linker flags
LDFLAGS = -lraylib

# Define the executable name
TARGET = downbat

# Define source files
SRCS = src/backend/components.cpp src/game/game.cpp src/game/scene_intro.cpp src/game/scene_interlude.cpp src/game/scene_game.cpp src/main.cpp

# Define object files
OBJS = $(SRCS:.cpp=.o)

# Default target: build the executable
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) $(LDFLAGS) -o $(TARGET)

# Rule for compiling .cpp files into .o files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean target: remove generated files
clean:
	rm -f $(OBJS) $(TARGET)