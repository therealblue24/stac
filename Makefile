#
# Copyright (C) 2025 therealblue24.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# Makefile for stac
#

# set this to 'yes' for release mode
RELEASE = no
# set this to your favorite compiler
CC = clang
# binary directory
BINDIR = bin
# app name
APP = stac
# lib name
# LIBNAM = libstac.a

# TODO: Switch to C11/C99
CFLAGS = -std=c23 -Wall -Wextra -Isrc -Iinclude -g3
CFLAGS += -MMD -MP
LDFLAGS = 

RELEASE ?= no
CODE_REVIEW ?= no
SANITIZERS ?= no

ifeq ($(RELEASE), yes)
	CFLAGS += -O2
endif

ifeq ($(CODE_REVIEW), yes)
	# Enable EVERYTHING.
    CFLAGS += -Weverything
    # We are using C
    CFLAGS += -Wno-unsafe-buffer-usage
    # We don't care about trailing ;
    CFLAGS += -Wno-extra-semi-stmt
    # We are using C23
    CFLAGS += -Wno-declaration-after-statement
    # I honestly don't care right know
    CFLAGS += -Wno-padded
    # I don't care
    CFLAGS += -Wno-date-time
    # I don't care
    CFLAGS += -Wno-float-equal
endif

ifeq ($(SANTIZERS), yes)
	CFLAGS += -fsanitize=undefined,address,leak
	LDFLAGS += -fsanitize=undefined,address,leak
endif

SRC = $(wildcard src/*.c)
LIBSRC = $(filter-out src/main.c, $(SRC))
OBJ = $(SRC:src/%.c=$(BINDIR)/%.o)
LIBOBJ = $(LIBSRC:src/%.c=$(BINDIR)/%.o)
DEP = $(OBJ:.o=.d)
LIBDEP = $(LIBOBJ:.o=.d)

TESTARGS = test/test.stac

FMTFILES = $(wildcard include/*.h) $(wildcard src/*.c) $(wildcard src/*.h)

.PHONY: dirs build test link fmt test

all: dirs build link

dirs:
	@# Create bin dir
	@mkdir -p $(BINDIR)

fmt:
	echo "WARNING: Not meant to be used on git repo clones!"
	make -C tools
	clang-format -style="file:.clang-format" -i -- $(FMTFILES)
	@echo "going to detab, used for git"
	./tools/detab v $(FMTFILES)

$(BINDIR)/%.o: src/%.c
	@echo "compiling $<"
	@$(CC) -o $@ -c $< $(CFLAGS)

-include $(DEP)

build: dirs $(OBJ)

$(BINDIR)/$(APP): link

link: build
	@# @echo "linking library"
	@# @ar rcs $(BINDIR)/$(LIBNAM)	$(LIBOBJ)
	@# @echo "made lib $(LIBNAM)"
	@echo "linking $(APP)"
	@$(CC) -o $(BINDIR)/$(APP) $(LDFLAGS) $(OBJ)
	@echo "made $(APP)"

clean:
	@echo "cleaning"
	rm -rf $(BINDIR)
	@echo "cleaned"

test: link
	@$(BINDIR)/$(APP) $(TESTARGS)
