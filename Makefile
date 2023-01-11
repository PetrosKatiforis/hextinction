# Variables set by the programmer
SOURCE_DIRS = src src/engine src/libs/noise
PKG_CONFIG_FLAGS = $(shell pkg-config --cflags --libs sdl2 SDL2_image SDL2_mixer SDL2_ttf) -lm
EXE_NAME = hextinction 

SOURCES = $(foreach sdir, $(SOURCE_DIRS), $(wildcard $(sdir)/*.c))
OBJECTS = $(patsubst src/%.c, objects/%.o, $(SOURCES))

all: prepare $(EXE_NAME) run

$(EXE_NAME): $(OBJECTS)
	@gcc $^ -o $@ $(PKG_CONFIG_FLAGS)
	@echo "The project was build successfully!"

objects/%.o: src/%.c
	@echo "Building $@"
	@gcc -c $^ -o $@ $(PKG_CONFIG_FLAGS)
	
prepare:
	@# Creating the necessary directories inside the objects folder
	@mkdir -p $(patsubst src/%, objects/%, $(SOURCE_DIRS))

run:
	@# Run the project once it's done building
	@./$(EXE_NAME) 2
