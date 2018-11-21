CC := gcc
CFLAGS := -Wall -Wextra -std=c99 -g
CFLAGS += $(shell pkg-config --cflags sdl2)
LDFLAGS := $(shell pkg-config --libs sdl2)
DEPFLAGS = -MMD -MP -MF $(@:$(OBJDIR)/%.o=$(DEPDIR)/%.d)

SRCDIR := src
BUILDDIR := build
OBJDIR := $(BUILDDIR)/obj
DEPDIR := $(BUILDDIR)/dep

SRCS := $(SRCDIR)/cart.c \
        $(SRCDIR)/cpu.c \
        $(SRCDIR)/emu.c \
        $(SRCDIR)/gui.c \
        $(SRCDIR)/helper_functions.c \
        $(SRCDIR)/mappers.c \
        $(SRCDIR)/opcode_functions.c \
        $(SRCDIR)/opcode_table.c \
        $(SRCDIR)/ppu.c

OBJS := $(SRCS:%.c=$(OBJDIR)/%.o)
DEPS := $(SRCS:%.c=$(DEPDIR)/%.d)

.PHONY: all
all: emu

$(OBJDIR)/%.o : %.c
	@mkdir -p $(@D)
	@mkdir -p $(DEPDIR)/$(<D)
	$(CC) $(CFLAGS) $(DEPFLAGS) -c $< -o $@

emu: $(OBJS)
	@echo "--- Linking target"
	$(CC) -o $@ $(OBJS) $(LDFLAGS)
	@echo "--- Done: Linking target"

.PHONY: clean
clean:
	@echo "--- Cleaning build"
	rm -f emu
	rm -rf $(BUILDDIR)

-include $(DEPS)
