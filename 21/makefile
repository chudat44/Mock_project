# Makefile for SDL2 MP3 Player
# Works for both Windows (MSYS2/MinGW) and Linux

# Detect the operating system
ifeq ($(OS),Windows_NT)
    DETECTED_OS := Windows
    # Windows-specific settings
    CC := g++
    TARGET := mp3player.exe
    RM := del /Q
    INCLUDE_DIRS := -I/ucrt64/include
    LIBS := -L/ucrt64/lib -lmingw32 -lSDL2main -lSDL2 -lSDL2_mixer
else
    DETECTED_OS := $(shell uname -s)
    # Linux-specific settings
    CC := g++
    TARGET := mp3player
    RM := rm -f
    INCLUDE_DIRS := $(shell pkg-config --cflags sdl2)
    LIBS := $(shell pkg-config --libs sdl2) -lSDL2_mixer
endif

# Common settings
CFLAGS := -Wall -Wextra -O2 $(INCLUDE_DIRS)
SRC := mp3player.cpp

# Default target
all: $(TARGET)

# Build the executable
$(TARGET): $(SRC)
	@echo Building for $(DETECTED_OS)...
	$(CC) $(CFLAGS) $< -o $@ $(LIBS)

# Clean the build
clean:
	@echo Cleaning up...
	$(RM) $(TARGET)

# Run the program (requires a music file as parameter)
run: $(TARGET)
	@echo Running $(TARGET)...
	./$(TARGET) sample.mp3

# Help target
help:
	@echo "Available targets:"
	@echo "  all    - Build the MP3 player (default)"
	@echo "  clean  - Remove the executable"
	@echo "  run    - Run the program with a sample MP3 file"
	@echo "  help   - Show this help message"
	@echo ""
	@echo "Detected OS: $(DETECTED_OS)"

.PHONY: all clean run help