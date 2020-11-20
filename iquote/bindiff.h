// SPDX-License-Identifier: GPLv3-or-later
/*
 * bindiff.h - our includes, sorted
 * Copyright Peter Jones <pjones@redhat.com>
 */

#ifndef BINDIFF_H_
#define BINDIFF_H_

#include <err.h>
#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/cdefs.h>
#include <sys/param.h>
#include <sys/uio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <termios.h>
#include <unistd.h>

extern int verbose;

#include "compiler.h"

#include "color.h"
#include "debug.h"
#include "hexdump.h"
#include "math.h"
#include "time.h"
#include "tty.h"

#endif /* !BINDIFF_H_ */
// vim:fenc=utf-8:tw=75:noet
