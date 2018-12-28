CC := gcc
CFLAGS := -Wall -Wextra -std=c99
CFLAGS += $(shell pkg-config --cflags sdl2)
LDFLAGS := $(shell pkg-config --libs sdl2)
DEPFLAGS = -MMD -MP -MF $(@:$(OBJDIR)/%.o=$(DEPDIR)/%.d)

CONFIG ?= debug

ifneq ($(CONFIG),debug)
ifneq ($(CONFIG),release)
$(error "CONFIG must be set to release or debug")
endif
endif

ifeq ($(CONFIG),debug)
CFLAGS += -g
endif
ifeq ($(CONFIG),release)
CFLAGS += -O2
endif

SRCDIR := src
BUILDDIR := build
BINDIR := $(BUILDDIR)/$(CONFIG)/bin
OBJDIR := $(BUILDDIR)/$(CONFIG)/obj
DEPDIR := $(BUILDDIR)/$(CONFIG)/dep

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
all: $(BINDIR)/emu

$(OBJDIR)/%.o : %.c
	@mkdir -p $(@D)
	@mkdir -p $(DEPDIR)/$(<D)
	$(CC) $(CFLAGS) $(DEPFLAGS) -c $< -o $@

$(BINDIR):
	mkdir -p $@

$(BINDIR)/emu: $(OBJS) | $(BINDIR)
	@echo "--- Linking target"
	$(CC) -o $@ $(OBJS) $(LDFLAGS)
	@echo "--- Done: Linking target"

.PHONY: clean
clean:
	@echo "--- Cleaning build"
	rm -f $(BINDIR)/emu
	rm -rf $(BUILDDIR)

-include $(DEPS)
