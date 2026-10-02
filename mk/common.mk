# Variables that all sub-Makefiles share
PLATFORM  ?= linux
MODE      ?= debug

CC        ?= cc
AR        ?= ar
BUILD     := bin/$(MODE)/$(PLATFORM)
OBJ_DIR   := $(BUILD)/obj
LIB_DIR   := $(BUILD)/lib
PKGS = 

CPPFLAGS  := -Isys/include/sys -Isys/include -I$(PLATFORM)
CFLAGS := -std=c11
CFLAGS += -Wall -Wextra -Wpedantic
CFLAGS += -Wshadow -Wcast-align -Wunused
CFLAGS += -Wmissing-prototypes -Wno-unused-parameter -Werror
CFLAGS += -Wstrict-prototypes -Wpointer-arith -Wwrite-strings
CFLAGS += -Wconversion -Wformat=2 -Wformat-security
CFLAGS += -Wunreachable-code -Wundef -Wbad-function-cast
CFLAGS += -Wdouble-promotion -Wmissing-include-dirs
CFLAGS += -fno-common -fstack-protector-strong

CFLAGS    += -MMD -MP

LDFLAGS := -L$(LIB_DIR)
LDLIBS   = -Wl,--start-group $(LINK_LIBS:%=-l%) -Wl,--end-group $(PKG_LIBS) -lm

ifeq ($(MODE),debug)
	CFLAGS += -g3 -O0 -fsanitize=address,undefined
else
	CFLAGS += -O2 -DNDEBUG
endif

# pkg-config bits (only computed once)
PKG_CFLAGS := $(shell pkg-config --cflags $(PKGS))
PKG_LIBS   := $(shell pkg-config --libs $(PKGS))

CPPFLAGS += $(PKG_CFLAGS)
# need to be include 1 time 

.SUFFIXES: .c .o


.c.o:
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@
