# Returns all c files nested or not in $(1)
define collect_sources
	$(shell find $(1) -name '*.c')
endef

SOURCES = $(call collect_sources, src)
OBJECTS = $(patsubst %.c, objects/%.o, $(SOURCES))

C_FLAGS = `pkg-config --cflags sdl2 SDL2_image SDL2_mixer SDL2_ttf`
LD_FLAGS = `pkg-config --libs sdl2 SDL2_image SDL2_mixer SDL2_ttf` -lm

all: hextinction

hextinction: $(OBJECTS)
	@echo "[Makefile] Creating the executable"
	@$(CC) $^ -o $@ $(LD_FLAGS)

	@# Running the executable once the building is done with 2 players
	@./$@ 2

objects/%.o: %.c
	@# Making sure that the directory already exists before creating the object
	@mkdir -p $(dir $@)

	@echo "[Makefile] Building $@"
	@$(CC) $(C_FLAGS) -c $< -o $@

