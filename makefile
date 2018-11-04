CC = gcc
CFLAGS = -Wall -std=c99
LDFLAGS = $(shell pkg-config --cflags --libs sdl2)

SRC= src
OBJ= obj

ALL_SRCS = $(wildcard $(SRC)/*.c)
ALL_OBJS = $(notdir $(ALL_SRCS:.c=.o))

NO_SDL_SRC := $(filter-out $(SRC)/gui.c, $(ALL_SRCS))
NO_SDL_SRC := $(filter-out $(SRC)/emu.c, $(ALL_SRCS))
NO_SDL_OBJ = $(notdir $(NO_SDL_SRC:.c=.o))

all: emu

$(NO_SDL_OBJ): %.o : $(SRC)/%.h

$(SRC)/cpu.c: $(SRC)/ppu.h
$(SRC)/cart.c: $(SRC)/ppu.h $(SRC)/mappers.h
$(SRC)/mappers.c: $(SRC)/cart.h
$(SRC)/opcode_functions.c: $(SRC)/helper_functions.h
$(SRC)/opcode_table.c: $(SRC)/opcode_functions.h

%.o : $(SRC)/%.c
	@echo "--- Compiling $@"
	$(CC) $(CFLAGS) -c $< -o $(OBJ)/$@

gui.o: $(SRC)/gui.*
	@echo "--- Compiling SDL2 files (gui.c)"
	$(CC) $(CFLAGS) -c $(SRC)/gui.c $(LDFLAGS) -o $(OBJ)/$@
	@echo "--- Done: Compiling SDL2 files (gui.c)"

emu.o: $(SRC)/emu.c $(SRC)/ppu.h $(SRC)/cpu.h $(SRC)/opcode_table.h
	@echo "--- Generating $@"
	$(CC) $(CFLAGS) -c $< -o $(OBJ)/$@

emu: $(ALL_OBJS)
	@echo "--- Linking target"
	$(CC) $(LDFLAGS) -o emu $(addprefix obj/,$(ALL_OBJS))
	@echo "--- Done: Linking target"

clean:
	@echo "--- Cleaning build"
	rm $(OBJ)/*.o emu