// SPDX-License-Identifier: GPLv3-or-later
/*
 * diffapi.h - our internal API for a diffing engine.
 * Copyright Peter Jones <pjones@redhat.com>
 */

#ifndef DIFFAPI_H_
#define DIFFAPI_H_

#include <sys/uio.h>

struct differ_data {
	char *file;
	struct iovec mem;
	size_t pos;
};

struct differ_hunk {
        struct color *color;
        hexdiff_op_t op;
        size_t apos, bpos;
        char *buf;
        size_t sz;
};

struct differ_priv {
	struct differ_data a, b;
	bool first;
	size_t opos;
	struct differ_hunk *hunks;
	size_t n_hunks;
	size_t n_hunk_bufs;
};

typedef void collect_t(void *priv);

struct differ {
	collect_t *collect;
};

extern struct differ xbdiff;
extern struct differ xrabdiff;

#endif /* !DIFFAPI_H_ */
// vim:fenc=utf-8:tw=75:noet
