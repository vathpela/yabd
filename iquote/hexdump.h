// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * libefivar - library for the manipulation of EFI variables
 * Copyright 2018-2020 Peter M. Jones <pjones@redhat.com>
 */
#ifndef STATIC_HEXDUMP_H
#define STATIC_HEXDUMP_H

#include <ctype.h>

extern bool hexdebug;

typedef enum
{
	DELETE,
	COPY,
	INSERT,
	IGNORE
} hexdiff_op_t;

void vfhexdumpf(FILE *f, const char *const fmt, uint8_t *data, size_t size, uint64_t at, int highlight, int regular, va_list ap);
void fhexdumpf(FILE *f, const char *const fmt, uint8_t *data, size_t size, uint64_t at, int highlight, int regular, ...);
void hexdump(void *data, size_t size);
void hexdumpat(void *data, size_t size, size_t at);
void dhexdump(void *data, size_t size);
void dhexdumpf(const char *fmt, void *data, size_t size, ...);
void dhexdumpat(void *data, size_t size, size_t at);
void vfhexdifff(FILE *f, const char *const fmt, va_list ap, hexdiff_op_t op, uint64_t *opos, uint64_t *npos, uint8_t *data, size_t size, text_color_t fg);
void fhexdifff(FILE *f, const char *const fmt, hexdiff_op_t op, uint64_t *oposp, uint64_t *nposp, uint8_t *data, size_t size, text_color_t fg, ...);
void hexdiff(hexdiff_op_t op, uint64_t *opos, uint64_t *npos, void *data, size_t sz, text_color_t fg);

#endif /* STATIC_HEXDUMP_H */

// vim:fenc=utf-8:tw=75:noet
