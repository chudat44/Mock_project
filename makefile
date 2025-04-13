# Makefile for Media Player Project

# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra
DEBUGFLAGS = -g -O0 -DDEBUG
RELEASEFLAGS = -O2 -DNDEBUG

# Directories
SRC_DIR = src
BUILD_DIR = build
RELEASE_BUILD_DIR = $(BUILD_DIR)/Build
DEBUG_BUILD_DIR = $(BUILD_DIR)/Debug
OBJ_DIR_RELEASE = $(RELEASE_BUILD_DIR)/src
OBJ_DIR_DEBUG = $(DEBUG_BUILD_DIR)/src

LIBS        := -ltag -lSDL2 -lSDL2_test -lSDL2_ttf -lSDL2_image -lSDL2_mixer
LIBDIRS     := -L/ucrt64/lib

# Source files
SRCS = $(wildcard $(SRC_DIR)/*.cpp) \
       $(wildcard $(SRC_DIR)/Controller/*.cpp) \
       $(wildcard $(SRC_DIR)/Model/*.cpp) \
       $(wildcard $(SRC_DIR)/View/*.cpp)

# Object files
OBJS_RELEASE = $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR_RELEASE)/%.o,$(SRCS))
OBJS_DEBUG = $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR_DEBUG)/%.o,$(SRCS))

# Target executable
TARGET_RELEASE = $(RELEASE_BUILD_DIR)/MediaPlayer.exe
TARGET_DEBUG = $(DEBUG_BUILD_DIR)/MediaPlayer.exe

# Include directories
INCLUDES = -I$(SRC_DIR) -I$(SRC_DIR)/Core -I$(SRC_DIR)/View/Interface $(LIBDIRS) $(LIBS)

# Default target
all: release

# Create build directories
$(OBJ_DIR_RELEASE) $(OBJ_DIR_DEBUG):
	mkdir -p $(OBJ_DIR_RELEASE)/Controller
	mkdir -p $(OBJ_DIR_RELEASE)/Model
	mkdir -p $(OBJ_DIR_RELEASE)/View
	mkdir -p $(DEBUG_BUILD_DIR)
	mkdir -p $(OBJ_DIR_DEBUG)/Controller
	mkdir -p $(OBJ_DIR_DEBUG)/Model
	mkdir -p $(OBJ_DIR_DEBUG)/View

# Compile source files (release build)
$(OBJ_DIR_RELEASE)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR_RELEASE)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(RELEASEFLAGS) $(INCLUDES) -c $< -o $@

# Compile source files (debug build)
$(OBJ_DIR_DEBUG)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR_DEBUG)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(DEBUGFLAGS) $(INCLUDES) -c $< -o $@

# Link release executable
$(TARGET_RELEASE): $(OBJS_RELEASE)
	$(CXX) $(CXXFLAGS) $(RELEASEFLAGS) $^ $(INCLUDES) -o $@

# Link debug executable
$(TARGET_DEBUG): $(OBJS_DEBUG)
	$(CXX) $(CXXFLAGS) $(DEBUGFLAGS) $^ $(INCLUDES) -o $@

# Build targets
release: $(TARGET_RELEASE)

debug: $(TARGET_DEBUG)

# Run targets
run: release
	$(TARGET_RELEASE)

run-debug: debug
	$(TARGET_DEBUG)

# Clean build files
clean:
	rm -rf $(RELEASE_BUILD_DIR)/* $(DEBUG_BUILD_DIR)/*

# Clean all generated files
clean-all:
	rm -rf $(BUILD_DIR)/*

# Phony targets
.PHONY: all release debug run run-debug clean clean-all