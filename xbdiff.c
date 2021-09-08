// SPDX-License-Identifier: GPLv3-or-later
/*
 * xdiff.c - bindiff interface for libxdiff differs
 * Copyright Peter Jones <pjones@redhat.com>
 */

#include "bindiff.h"

struct xbdiff_priv {
};

static void
xbdiff_collect(void *priv)
{

}

struct differ xbdiff = {
	.collect = xbdiff_collect,
};


// vim:fenc=utf-8:tw=75:noet
