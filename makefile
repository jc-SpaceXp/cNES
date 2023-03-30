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
COREDIR := $(SRCDIR)/core
UTILDIR := $(SRCDIR)/util
BUILDDIR := build
BINDIR := $(BUILDDIR)/$(CONFIG)/bin
OBJDIR := $(BUILDDIR)/$(CONFIG)/obj
DEPDIR := $(BUILDDIR)/$(CONFIG)/dep

INCDIR := include
INCS_CORE := $(INCDIR)/core
INCS_UTIL := $(INCDIR)/util

UTILS := $(UTILDIR)/bits_and_bytes.c
UTIL_OBJS := $(UTILS:%.c=$(OBJDIR)/%.o)
UTIL_DEPS := $(UTILS:%.c=$(DEPDIR)/%.d)

SRCS_CORE := $(COREDIR)/cart.c \
             $(COREDIR)/cpu.c \
             $(COREDIR)/emu.c \
             $(COREDIR)/gui.c \
             $(COREDIR)/mappers.c \
             $(COREDIR)/ppu.c \
             $(COREDIR)/cpu_ppu_interface.c \
             $(COREDIR)/cpu_mapper_interface.c

CORE_OBJS := $(SRCS_CORE:%.c=$(OBJDIR)/%.o)
CORE_DEPS := $(SRCS_CORE:%.c=$(DEPDIR)/%.d)

TESTDIR := tests
TESTS := $(wildcard $(TESTDIR)/*.c)
TEST_OBJS := $(TESTS:%.c=$(OBJDIR)/%.o)
TEST_DEPS := $(TESTS:%.c=$(DEPDIR)/%.d)
TEST_DEP_OBJS := $(OBJDIR)/$(COREDIR)/cpu.o \
                 $(OBJDIR)/$(COREDIR)/mappers.o \
                 $(OBJDIR)/$(COREDIR)/ppu.o \
                 $(OBJDIR)/$(COREDIR)/gui.o \
                 $(OBJDIR)/$(COREDIR)/cart.o \
                 $(OBJDIR)/$(COREDIR)/cpu_ppu_interface.o \
                 $(OBJDIR)/$(COREDIR)/cpu_mapper_interface.o \
                 $(OBJDIR)/$(UTILDIR)/bits_and_bytes.o

.PHONY: all
all: $(BINDIR)/cnes $(BINDIR)/test_all

$(OBJDIR)/%.o : %.c
	@mkdir -p $(@D)
	@mkdir -p $(DEPDIR)/$(<D)
	$(CC) $(CFLAGS) $(DEPFLAGS) -I $(INCS_CORE) -I $(INCS_UTIL) -c $< -o $@

$(BINDIR):
	mkdir -p $@

$(BINDIR)/cnes: $(CORE_OBJS) $(UTIL_OBJS) | $(BINDIR)
	@echo "--- Linking target"
	$(CC) -o $@ $(CORE_OBJS) $(UTIL_OBJS) $(LDFLAGS)
	@echo "--- Done: Linking target"

$(BINDIR)/test_all: $(TEST_OBJS) $(TEST_DEP_OBJS) | $(BINDIR)
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

-include $(CORE_DEPS)
-include $(UTIL_DEPS)
-include $(TEST_DEPS)
