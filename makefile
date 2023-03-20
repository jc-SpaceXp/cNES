CC := gcc
CFLAGS := -Wall -Wextra -std=c99
CFLAGS += $(shell pkg-config --cflags sdl2)
LDFLAGS := $(shell pkg-config --libs sdl2)
LIBCHECK_FLAGS = $(shell pkg-config --cflags --libs check)
DEPFLAGS = -MMD -MP -MF $(@:$(OBJDIR)/%.o=$(DEPDIR)/%.d)

ASAN ?= 0
DEBUG ?= 0
ifeq ($(DEBUG), 1)
        CFLAGS += -g -D__DEBUG__
        CONFIG = debug
        ifeq ($(ASAN), 1)
                CFLAGS += -fsanitize=address,undefined
                LDFLAGS += -fsanitize=address,undefined
        endif
else
        CFLAGS += -DN__DEBUG__
        CONFIG = release
endif

SRCDIR := src
UTLDIR := $(SRCDIR)/util
BUILDDIR := build
BINDIR := $(BUILDDIR)/$(CONFIG)/bin
OBJDIR := $(BUILDDIR)/$(CONFIG)/obj
DEPDIR := $(BUILDDIR)/$(CONFIG)/dep

UTLS := $(UTLDIR)/bits_and_bytes.c
UTL_OBJS := $(UTLS:%.c=$(OBJDIR)/%.o)
UTL_DEPS := $(UTLS:%.c=$(DEPDIR)/%.d)

SRCS := $(SRCDIR)/cart.c \
        $(SRCDIR)/cpu.c \
        $(SRCDIR)/emu.c \
        $(SRCDIR)/gui.c \
        $(SRCDIR)/mappers.c \
        $(SRCDIR)/ppu.c \
        $(SRCDIR)/cpu_ppu_interface.c \
        $(SRCDIR)/cpu_mapper_interface.c

SRC_OBJS := $(SRCS:%.c=$(OBJDIR)/%.o)
SRC_DEPS := $(SRCS:%.c=$(DEPDIR)/%.d)

TSTDIR := tests
TSTS := $(wildcard $(TSTDIR)/*.c)
TST_OBJS := $(TSTS:%.c=$(OBJDIR)/%.o)
TST_DEPS := $(TSTS:%.c=$(DEPDIR)/%.d)
TST_TMP_OBJS := $(OBJDIR)/$(SRCDIR)/cpu.o \
                $(OBJDIR)/$(SRCDIR)/mappers.o \
                $(OBJDIR)/$(SRCDIR)/ppu.o \
                $(OBJDIR)/$(SRCDIR)/gui.o \
                $(OBJDIR)/$(SRCDIR)/cart.o \
                $(OBJDIR)/$(SRCDIR)/cpu_ppu_interface.o \
                $(OBJDIR)/$(SRCDIR)/cpu_mapper_interface.o \
                $(OBJDIR)/$(UTLDIR)/bits_and_bytes.o

.PHONY: all
all: $(BINDIR)/cnes $(BINDIR)/test_all

$(OBJDIR)/%.o : %.c
	@mkdir -p $(@D)
	@mkdir -p $(DEPDIR)/$(<D)
	$(CC) $(CFLAGS) $(DEPFLAGS) -I $(SRCDIR) -I $(UTLDIR) -c $< -o $@

$(BINDIR):
	mkdir -p $@

$(BINDIR)/cnes: $(SRC_OBJS) $(UTL_OBJS) | $(BINDIR)
	@echo "--- Linking target"
	$(CC) -o $@ $(SRC_OBJS) $(UTL_OBJS) $(LDFLAGS)
	@echo "--- Done: Linking target"

$(BINDIR)/test_all: $(TST_OBJS) $(TST_TMP_OBJS) | $(BINDIR)
	@echo "--- Linking tests"
	$(CC) -o $@ $^ $(LIBCHECK_FLAGS) $(LDFLAGS)
	@echo "--- Done: Linking tests"
	@echo "--- Running tests"
	@./$(BINDIR)/test_all

.PHONY: test
test:
	@echo "--- Running tests"
	@./$(BINDIR)/test_all

.PHONY: clean
clean:
	@echo "--- Cleaning build"
	rm -f $(BINDIR)/cnes
	rm -rf $(BUILDDIR)

-include $(SRC_DEPS)
-include $(UTL_DEPS)
-include $(TST_DEPS)
