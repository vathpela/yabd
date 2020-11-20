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
TARGETS = libxdiff $(BINTARGETS)

all: $(TARGETS)

% : | libxdiff.a
% : %.c $(wildcard iquote/*.h %.h)
	$(CC) \
		$(CFLAGS) -Ilibxdiff/xdiff/ \
		$(LDFLAGS) \
		-o $@ $(filter %.c,$^) \
		libxdiff/build/libxdiff.a

%.C : | $(wildcard *.h iquote/*.h)
%.C : %.c
	$(CC) $(CFLAGS) -Ilibxdiff/xdiff/ -E -o $@ $(filter %.c,$^)

$(wildcard *.c) : | $(wildcard *.h iquote/*.h)

bindiff : | libxdiff
bindiff : $(wildcard *.c *.h iquote/%.h) iquote/hexdump.h

.ONESHELL:
libxdiff :
	@ :;
	set -eu
	export PREFIX=/usr
	export CFLAGS="-Og -g3 -std=gnu11 -Wall -Wextra -Wno-missing-field-initializers -Wno-nonnull -Wno-unused-parameter -Wno-sign-compare -Wno-unused-but-set-variable -Wno-unsed-const-variable -Wno-unused-function -Wno-unused -Werror -I$${PWD}"
	export CC="$(CC)"
	if ! [[ -f libxdiff/build/libxdiff.a ]] ; then
		mkdir -p libxdiff/build
		cd libxdiff/build
		if ! [[ -f Makefile ]] ; then
			cmake ..
		fi
		$(MAKE) CMAKE_INSTALL_PREFIX=/usr PREFIX=/usr VERBOSE=1 clean all
		cd -
	fi

clean :
	@rm -vf $(BINTARGETS) $(wildcard *.C)
	rm -rfv libxdiff/build/

include iquote/scan-build.mk

.PHONY: clean all libxdiff

# vim:ft=make
