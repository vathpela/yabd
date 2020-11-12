// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * libefivar - library for the manipulation of EFI variables
 * Copyright 2018-2020 Peter M. Jones <pjones@redhat.com>
 */
#ifndef STATIC_HEXDUMP_H
#define STATIC_HEXDUMP_H

#include <ctype.h>

static bool hexdebug = false;

static inline size_t UNUSED
prepare_hex(void *data, size_t size, char *buf, uint64_t position, int64_t skew)
{
	char hexchars[] = "0123456789abcdef";
	uint64_t offset = 0;
	uint64_t i;
	uint64_t j;
	uint64_t ret;

	size_t before = ((position - skew) % 16);
	size_t after = (before + size >= 16) ? 0 : 16 - (before + size);

	if (hexdebug) {
		printf("=dbg %s\t", __func__);
		printf("before:%zu after:%zu data:%p size:%zu position:0x%lx skew:0x%lx ",
		       before, after, data, size, position, skew);
	}

	for (i = 0; i < before; i++) {
		buf[offset++] = ' ';
		buf[offset++] = ' ';
		buf[offset++] = ' ';
		if (i == 7)
			buf[offset++] = ' ';
	}
	for (j = 0; j < 16 - after - before; j++) {
		uint8_t d = ((uint8_t *)data)[j];
		buf[offset++] = hexchars[(d & 0xf0) >> 4];
		buf[offset++] = hexchars[(d & 0x0f)];
		if (i + j != 15)
			buf[offset++] = ' ';
		if (i + j == 7)
			buf[offset++] = ' ';
	}
	ret = 16 - after - before;
	j += i;
	for (i = 0; i < after; i++) {
		buf[offset++] = ' ';
		buf[offset++] = ' ';
		if (i + j != 15)
			buf[offset++] = ' ';
		if (i + j == 7)
			buf[offset++] = ' ';
	}
	buf[offset] = '\0';
	if (hexdebug) {
		printf("ret:0x%lx\n", ret);
		fflush(stdout);
	}
	return ret;
}

static inline void UNUSED
prepare_text(void *data, size_t size, char *buf, uint64_t position,
             uint64_t skew)
{
	uint64_t offset = 0;
	uint64_t i;
	uint64_t j;

	size_t before = (position - skew) % 16;
	size_t after = (before + size > 16) ? 0 : 16 - (before + size);

	if (hexdebug) {
		printf("=dbg %s\t", __func__);
		printf("before:%zu after:%zu data:%p\n", before, after, data);
	}

	if (size == 0) {
		buf[0] = '\0';
		return;
	}
	for (i = 0; i < before; i++)
		buf[offset++] = ' ';
	buf[offset++] = '|';
	for (j = 0; j < 16 - after - before; j++) {
		if (isprint(((uint8_t *)data)[j]))
			buf[offset++] = ((uint8_t *)data)[j];
		else
			buf[offset++] = '.';
	}
	buf[offset++] = size > 0 ? '|' : ' ';
	buf[offset] = '\0';
}

/*
 * variadic fhexdump formatted
 * think of it as: fprintf(f, %s%s\n", vformat(fmt, ap), hexdump(data,size));
 */
static inline void UNUSED
vfhexdumpf(FILE *f, const char *const fmt, uint8_t *data, size_t size,
           uint64_t at, va_list ap)
{
	size_t display_offset = at;
	size_t offset = 0;
	//debug("data:%p size:%zd at:%zd\n", data, size, display_offset);

	while (offset < size) {
		char hexbuf[49];
		char txtbuf[19];
		size_t sz;

		sz = prepare_hex(data + offset, size - offset, hexbuf,
		                 (size_t)data + offset, at % 16);
		if (sz == 0)
			return;

		prepare_text(data + offset, size - offset, txtbuf,
		             (size_t)data + offset, at % 16);
		vfprintf(f, fmt, ap);
		fprintf(f, "%08lx  %s  %s\n", display_offset, hexbuf, txtbuf);

		display_offset += sz;
		offset += sz;
	}
	fflush(f);
}

/*
 * fhexdump formatted
 * think of it as: fprintf(f, %s%s\n", format(fmt, ...), hexdump(data,size));
 */
static inline void UNUSED
fhexdumpf(FILE *f, const char *const fmt, uint8_t *data, size_t size,
          uint64_t at, ...)
{
	va_list ap;

	va_start(ap, at);
	vfhexdumpf(f, fmt, data, size, at, ap);
	va_end(ap);
}

static inline void UNUSED
hexdump(void *data, size_t size)
{
	fhexdumpf(stdout, "", data, size, (intptr_t)data);
}

static inline void UNUSED
hexdumpat(void *data, size_t size, size_t at)
{
	fhexdumpf(stdout, "", data, size, at);
}

static inline void UNUSED
dhexdump(void *data, size_t size)
{
	if (unc_get_debug(NULL))
		hexdump(data, size);
}

static inline void UNUSED
dhexdumpf(const char *fmt, void *data, size_t size, ...)
{
	if (!unc_get_debug(NULL))
		return;

	va_list ap;

	va_start(ap, size);
	vfhexdumpf(stdout, fmt, data, size, (uint64_t)data, ap);
	va_end(ap);
}

static inline void UNUSED
dhexdumpat(void *data, size_t size, size_t at)
{
	if (unc_get_debug(NULL))
		hexdumpat(data, size, at);
}

typedef enum
{
	DELETE,
	COPY,
	INSERT,
	IGNORE
} hexdiff_op_t;

/*
 * variadic hexdiff-to-file, formatted
 * think of it as: fprintf(f, %s%s\n", vformat(fmt, ap), hexdiff(op, opos, apos, data, size));
 */
static inline void UNUSED
vfhexdifff(FILE *f, const char *const fmt, va_list ap, hexdiff_op_t op,
           uint64_t *opos, uint64_t *npos, uint8_t *data, size_t size)
{
	const char opc[] = "- +";
	size_t offset = 0;

	if (hexdebug)
		debug("data:%p size:%zd opos:0x%zx npos:0x%zx\n", data, size,
		      *opos, *npos);
	while (offset < size) {
		char hexbuf[49];
		char txtbuf[19];
		size_t sz = 0;

		switch (op) {
		case DELETE:
			sz = prepare_hex(data + offset, size - offset, hexbuf,
			                 *opos, 0);
			if (sz == 0)
				break;

			prepare_text(data + offset, size - offset, txtbuf,
			             *opos, 0);
			break;
		case COPY:
		case INSERT:
			sz = prepare_hex(data + offset, size - offset, hexbuf,
			                 *npos, 0);
			if (sz == 0)
				break;

			prepare_text(data + offset, size - offset, txtbuf,
			             *npos, 0);
			break;
		case IGNORE:
			continue;
		default:
			break;
		}

		vfprintf(f, fmt, ap);
		fprintf(f, "%c%08lx/%08lx  %s  %s\n", opc[op], *opos, *npos,
		        hexbuf, txtbuf);

		offset += sz;
		switch (op) {
		case DELETE:
			*opos += sz;
			break;
		case COPY:
			*opos += sz;
			*npos += sz;
			break;
		case INSERT:
			*npos += sz;
			break;
		case IGNORE:
			break;
		}
	}
	fflush(f);
}

/*
 * fhexdiff formatted
 * think of it as: fprintf(f, %s%s\n", format(fmt, ...), hexdiff(opos, npos, data, size));
 */
static inline void UNUSED
fhexdifff(FILE *f, const char *const fmt, hexdiff_op_t op, uint64_t *oposp,
          uint64_t *nposp, uint8_t *data, size_t size, ...)
{
	va_list ap;

	va_start(ap, size);
	vfhexdifff(f, fmt, ap, op, oposp, nposp, data, size);
	va_end(ap);
}

static inline void UNUSED
hexdiff(hexdiff_op_t op, uint64_t *opos, uint64_t *npos, void *data, size_t sz)
{
	hexdebug = false;
	switch (op) {
	case DELETE:
		fhexdifff(stdout, "", DELETE, opos, npos, data, sz);
		break;
	case COPY:
		fhexdifff(stdout, "", COPY, opos, npos, data, sz);
		break;
	case INSERT:
		fhexdifff(stdout, "", INSERT, opos, npos, data, sz);
		break;
	case IGNORE:
		break;
	}
	hexdebug = false;
}

#endif /* STATIC_HEXDUMP_H */

// vim:fenc=utf-8:tw=75:noet
