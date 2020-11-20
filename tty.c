// SPDX-License-Identifier: GPLv3-or-later
/*
 * tty.c - tty handling junk
 * Copyright Peter Jones <pjones@redhat.com>
 */

#include "bindiff.h"

// struct term {
//	size_t
// };

HIDDEN ssize_t
get_esc_sz(const char *buf, const size_t size)
{
	errno = ENOSYS;
	return -1;
}

// vim:fenc=utf-8:tw=75:noet
