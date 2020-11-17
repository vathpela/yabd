# SPDX-License-Identifier: GPLv3-or-later
#
# Makefile
# Copyright Peter Jones <pjones@redhat.com>
#

CFLAGS ?=
OPTIMIZE ?= -Og -flto
CC=gcc
CFLAGS += $(OPTIMIZE) \
	  -iquote iquote \
	  -g3 \
	  -D_GNU_SOURCE \
	  -std=gnu11 \
	  -fno-strict-aliasing \
	  -Wall -Wextra \
	  -Wno-missing-field-initializers \
	  -Wno-nonnull \
	  -Wno-unused-parameter \
	  -Werror \
	  -Wno-error=sign-compare \
	  -Wno-error=unused-but-set-variable \
	  -Wno-error=unused-const-variable \
	  -Wno-error=unused-variable \
	  -Wno-error=unused-function \
	  -Wno-unused-variable \
	  -Wno-unused-const-variable \
	  -Wno-unused-function

LDFLAGS := -Wl,--add-needed \
	   -Wl,--build-id \
	   -Wl,--no-allow-shlib-undefined \
	   -Wl,--no-undefined-version \
	   -Wl,-z,muldefs \
	   -Wl,-z,now \
	   -Wl,-z,relro

BINTARGETS = bindiff
TARGETS = $(BINTARGETS)

all: $(TARGETS)

% : %.c $(wildcard iquote/*.h %.h)
	$(CC) \
		$(CFLAGS) \
		$(LDFLAGS) \
		-o $@ $(filter %.c,$^)

%.C : | $(wildcard *.h iquote/*.h)
%.C : %.c
	$(CC) $(CFLAGS) -E -o $@ $(filter %.c,$^)

$(wildcard *.c) : | $(wildcard *.h iquote/*.h)

bindiff : $(wildcard *.c *.h iquote/%.h)


clean :
	@rm -vf $(BINTARGETS) $(wildcard *.C)

include iquote/scan-build.mk

.PHONY: clean all

# vim:ft=make
