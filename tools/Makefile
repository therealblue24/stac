#
# Copyright (C) 2025 therealblue24.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# Makefile for stac tools
#
 
CC ?= clang

all: lhdr escapegen detab

%: %.c
	$(CC) -o $@ $<
