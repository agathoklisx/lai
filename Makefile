
ROOTDIR     := $(shell pwd)
SYSDIR      := $(shell realpath $(ROOTDIR)/../sys)
SRCDIR       = $(ROOTDIR)/src
BUILDDIR     = $(shell realpath $(ROOTDIR)/build)

BINDIR       = $(SYSDIR)/bin
INCDIR       = $(SYSDIR)/include
LIBDIR       = $(SYSDIR)/lib

DICTU        = Dictu
DICTU_REPO   = https://github.com/Jason2605/Dictu.git
DICTU_DIR    = $(SRCDIR)/$(DICTU)
DICTU_CDIR   = $(DICTU_DIR)/c
DATATYPE_DIR = $(DICTU_CDIR)/datatypes
OPTIONAL_DIR = $(DICTU_CDIR)/optionals
DICTU_BIN    = $(DICTU_DIR)/dictu

CC          := gcc
CC_STD      := -std=c11

BASE_FLAGS  := -g -O2 -march=native
# -fvisibility=hidden
DEBUG_FLAGS := -Wextra -Wshadow -Wall -Wunused-function -Wunused-macros

FLAGS       := $(BASE_FLAGS) $(CFLAGS)

DEBUG := 1
ifneq ($(DEBUG), 0)
  FLAGS += $(DEBUG_FLAGS)
endif

DEFINES   = -DSYSDIR='"$(SYSDIR)"'
DEFINES  += -DBUILDDIR='"$(BUILDDIR)"'
DEFINES  += -DSRCDIR='"$(SRCDIR)"'
DEFINES  += -DLANGCDIR='"$(DICTU_CDIR)"'

lmake: makeenv
	$(CC) $(CC_STD) $(DEFINES) $(BASE_FLAGS) $(DEBUG_FLAGS) lmake.c -o lmake

clone_upstream: makeenv
	@$(TEST) -d $(DICTU_DIR) || (cd $(SRCDIR) && $(GIT_CLONE) $(DICTU_REPO) $(DICTU))

update_upstream: clone_upstream
	@cd $(DICTU_DIR) && $(GIT_PULL)
	
makeenv:
	@$(TEST_DIR) $(SRCDIR)    || $(MAKDIR) $(SRCDIR)
	@$(TEST_DIR) $(SYSDIR)    || $(MAKDIR) $(SYSDIR)
	@$(TEST_DIR) $(BINDIR)    || $(MAKDIR) $(BINDIR)
	@$(TEST_DIR) $(LIBDIR)    || $(MAKDIR) $(LIBDIR)
	@$(TEST_DIR) $(INCDIR)    || $(MAKDIR) $(INCDIR)
	@$(TEST_DIR) $(BUILDDIR) || $(MAKDIR) $(BUILDDIR)

GIT          = git
GIT_CLONE    = $(GIT) clone
GIT_PULL     = $(GIT) pull
MAKDIR       = mkdir
TEST         = test
TEST_DIR     = $(TEST) -d
