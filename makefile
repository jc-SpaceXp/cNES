CC := gcc
CFLAGS := -Wall -Wextra -std=c99
CFLAGS += $(shell pkg-config --cflags sdl2)
LDFLAGS := $(shell pkg-config --libs sdl2)
DEPFLAGS = -MMD -MP -MF $(@:$(OBJDIR)/%.o=$(DEPDIR)/%.d)

DEBUG ?= 0
ifeq ($(DEBUG), 1)
        CFLAGS += -g -D__DEBUG__
        CONFIG = debug
else
        CFLAGS += -DN__DEBUG__ 
        CONFIG = release
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
