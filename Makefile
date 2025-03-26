CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -D_XOPEN_SOURCE=700 `sdl2-config --cflags`
LIBS = `sdl2-config --libs` -lSDL2_ttf -lSDL2_gfx -lm -lcrypto

SRCDIR = src
OBJDIR = obj
SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SOURCES))
TARGET = Q-Striker

# Dependency checks
CHECK_SDL2_CONFIG := $(shell command -v sdl2-config 2> /dev/null)
ifeq ($(CHECK_SDL2_CONFIG),)
$(error "sdl2-config not found. Please install SDL2 development packages for your OS.")
endif

CHECK_SDL2_TTF := $(shell pkg-config --exists SDL2_ttf && echo yes || echo no)
ifeq ($(CHECK_SDL2_TTF),no)
$(error "SDL2_ttf not found. Please install the SDL2_ttf development package for your OS.")
endif

CHECK_SDL2_GFX := $(shell pkg-config --exists SDL2_gfx && echo yes || echo no)
ifeq ($(CHECK_SDL2_GFX),no)
$(error "SDL2_gfx not found. Please install the SDL2_gfx development package for your OS.")
endif

.PHONY: all debug clean

all: $(TARGET)
	@echo "Build complete."
	@rm -rf $(OBJDIR)

debug: $(TARGET)
	@echo "Debug build complete. Object files retained."

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR):
	mkdir -p $(OBJDIR)

clean:
	rm -rf $(OBJDIR) $(TARGET)

