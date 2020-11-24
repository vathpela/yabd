// SPDX-License-Identifier: GPLv3-or-later
/*
 * hexdump.c - hex dumper
 * Copyright Peter Jones <pjones@redhat.com>
 */

#include "bindiff.h"

bool hexdebug = false;

#define HDBUF ((buf && bufsz > 0) ? (buf + off) : NULL)
#define HDLIM ((buf && bufsz > 0) ? (bufsz - off) : 0)

static inline ssize_t UNUSED
prepare_hex(void *data, size_t size, size_t *consumed,
	    char *buf, size_t bufsz,
	    size_t position, int64_t skew)
{
	char hexchars[] = "0123456789abcdef";
	size_t i;
	size_t j;
	size_t ret;
	size_t off = 0;
	ssize_t sz;

	size_t before = ((position - skew) % 16);
	size_t after = (before + size >= 16) ? 0 : 16 - (before + size);

	if (!consumed) {
		errno = EINVAL;
		return -1;
	}

	*consumed = 0;
	if (hexdebug) {
		dprintf("=dbg %s\t", __func__);
		dprintf("before:%zu after:%zu data:%p size:%zu position:0x%lx skew:0x%lx ",
			before, after, data, size, position, skew);
	}

	for (i = 0; i < before; i++) {
		sz = snprintf(HDBUF, HDLIM, "   %s", i == 7 ? " " : "");
		if (sz < 0)
			return sz;
		off += sz;
	}
	debug("off:%zd", off);
	for (j = 0; j < 16 - after - before; j++) {
		uint8_t d = ((uint8_t *)data)[j];
		sz = snprintf(HDBUF, HDLIM, "%c%c%s%s",
			      hexchars[(d & 0xf0) >> 4],
			      hexchars[(d & 0x0f)],
			      i + j != 15 ? " " : "",
			      i + j == 7 ? " " : "");
		if (sz < 0)
			return sz;
		off += sz;
	}
	debug("off:%zd", off);
	*consumed = 16 - after - before;
	debug("*consumed:%zd = 16 - after:%zd - before:%zd", *consumed, after, before);
	j += i;
	for (i = 0; i < after; i++) {
		sz = snprintf(HDBUF, HDLIM, "  %s%s",
			      i + j != 15 ? " " : "",
			      i + j == 7 ? " " : "");
		if (sz < 0)
			return sz;
		off += sz;
	}
	debug("off:%zd", off);
	if (hexdebug) {
		debug("consumed:0x%zx, ret:0x%zx\n", *consumed, off);
		fflush(stdout);
	}
	return off;
}

static inline ssize_t UNUSED
prepare_text(void *data, size_t size, char *buf, size_t bufsz,
	     uint64_t position, uint64_t skew, int highlight, int regular)
{
	size_t off = 0;
	uint64_t i;
	uint64_t j;
	ssize_t sz;

	size_t before = (position - skew) % 16;
	size_t after = (before + size > 16) ? 0 : 16 - (before + size);

	if (hexdebug) {
		dprintf("=dbg %s\t", __func__);
		dprintf("before:%zu after:%zu data:%p\n", before, after, data);
	}

	if (size == 0) {
		if (HDBUF)
			buf[0] = '\0';
		return 0;
	}
	for (i = 0; i < before; i++) {
		sz = snprintf(HDBUF, HDLIM, " ");
		if (sz < 0)
			return sz;
		off += sz;
	}

	if (highlight == regular) {
		sz = snprintf(HDBUF, HDLIM, "|");
	} else {
		sz = snprintf(HDBUF, HDLIM, "\033[38:5:%dm|\033[38:5:%dm",
			      regular, highlight);
	}
	if (sz < 0)
		return sz;
	off += sz;
	for (j = 0; j < 16 - after - before; j++) {
		sz = snprintf(HDBUF, HDLIM, "%c",
			      isprint(((uint8_t *)data)[j]) ? ((uint8_t *)data)[j] : '.');
		if (sz < 0)
			return sz;
		off += sz;
	}

	if (highlight != regular) {
		sz = snprintf(HDBUF, HDLIM, "\033[38:5:%dm", regular);
		if (sz < 0)
			return sz;
		off += sz;
	}

	sz = snprintf(HDBUF, HDLIM, "%c", size > 0 ? '|' : ' ');
	if (sz < 0)
		return sz;
	off += sz;

	return off;
}

/*
 * variadic fhexdump formatted
 * think of it as: fprintf(f, %s%s\n", vformat(fmt, ap), hexdump(data,size));
 */
void
vfhexdumpf(FILE *f, const char *const fmt, uint8_t *data, size_t size,
           uint64_t at, int highlight, int regular, va_list ap)
{
	size_t display_offset = at;
	size_t offset = 0;
	//debug("data:%p size:%zd at:%zd\n", data, size, display_offset);

	while (offset < size) {
		char hexbuf[69];
		char txtbuf[39];
		ssize_t sz;
		size_t consumed;

		sz = prepare_hex(data + offset, size - offset, &consumed,
				 hexbuf, sizeof(hexbuf),
		                 (size_t)data + offset, at % 16);
		debug("prepare_hex(%p, %zd, %zd, %p, %zd, %zd, %zd) = %zd",
		      data + offset, size - offset, consumed,
		      hexbuf, sizeof(hexbuf),
		      (size_t)data + offset, at % 16, sz);
		if (consumed == 0 || sz < 0)
			return;

		prepare_text(data + offset, size - offset,
			     txtbuf, sizeof(txtbuf),
		             (size_t)data + offset, at % 16,
			     highlight, regular);
		vfprintf(f, fmt, ap);
		fprintf(f, "%08lx  %s  %s\n", display_offset, hexbuf, txtbuf);

		display_offset += consumed;
		offset += consumed;
	}
	fflush(f);
}

/*
 * fhexdump formatted
 * think of it as: fprintf(f, %s%s\n", format(fmt, ...), hexdump(data,size));
 */
void
fhexdumpf(FILE *f, const char *const fmt, uint8_t *data, size_t size,
          uint64_t at, int highlight, int regular, ...)
{
	va_list ap;

	va_start(ap, regular);
	vfhexdumpf(f, fmt, data, size, at, highlight, regular, ap);
	va_end(ap);
}

void
hexdump(void *data, size_t size)
{
	fhexdumpf(stdout, "", data, size, (intptr_t)data, black, black);
}

void
hexdumpat(void *data, size_t size, size_t at)
{
	fhexdumpf(stdout, "", data, size, at, black, black);
}

void
dhexdump(void *data, size_t size)
{
	if (unc_get_debug(NULL))
		hexdump(data, size);
}

void
dhexdumpf(const char *fmt, void *data, size_t size, ...)
{
	if (!unc_get_debug(NULL))
		return;

	va_list ap;

	va_start(ap, size);
	vfhexdumpf(stdout, fmt, data, size, (uint64_t)data, black, black, ap);
	va_end(ap);
}

void
dhexdumpat(void *data, size_t size, size_t at)
{
	if (unc_get_debug(NULL))
		hexdumpat(data, size, at);
}

/*
 * variadic hexdiff-to-file, formatted
 * emits one diff op
 * think of it as: fprintf(f, %s%s\n", vformat(fmt, ap), hexdiff(op, opos, apos, data, size));
 */
void
vfhexdifff(FILE *f, const char *const fmt, va_list ap, hexdiff_op_t op,
           uint64_t *opos, uint64_t *npos, uint8_t *data, size_t size,
	   text_color_t fg)
{
	const char opc[] = "- +";
	size_t offset = 0;

	if (hexdebug)
		debug("data:%p size:%zd opos:0x%zx npos:0x%zx color:%d\n", data, size,
		      *opos, *npos, fg);
	while (offset < size) {
		char linebuf[4096];
		ssize_t sz = 0;
		size_t tmpsz = 0;
		size_t consumed = 0;

		switch (op) {
		case DELETE:
			sz = prepare_hex(data + offset, size - offset, &consumed,
					 linebuf + tmpsz, sizeof(linebuf) - tmpsz,
					 *opos, 0);
			debug("prepare_hex(%p, %zd, %zd, %p, %zd, %zd, %zd) = %zd",
			      data + offset, size - offset, consumed,
			      linebuf + tmpsz, sizeof(linebuf) - tmpsz,
			      *opos, (size_t)0, sz);
			if (consumed == 0)
				break;
			if (sz < 0)
				break;
			tmpsz += sz;
			sz = snprintf(linebuf + tmpsz, sizeof(linebuf) - tmpsz,
				      "\033[38:5:%dm", black);

			tmpsz += snprintf(linebuf + tmpsz, sizeof(linebuf) - tmpsz, "  ");
			tmpsz += prepare_text(data + offset, size - offset,
					      linebuf + tmpsz, sizeof(linebuf) - tmpsz,
					      *opos, 0, fg, black);

			vfprintf(f, fmt, ap);
			fprintf(f, "%c\033[38:5:%dm%08lx  %s\n",
				opc[op], fg, *opos, linebuf);
			break;
		case COPY:
			sz = prepare_hex(data + offset, size - offset, &consumed,
					 linebuf + tmpsz, sizeof(linebuf) - tmpsz,
					 *npos, 0);
			debug("prepare_hex(%p, %zd, %zd, %p, %zd, %zd, %zd) = %zd",
			      data + offset, size - offset, consumed,
			      linebuf + tmpsz, sizeof(linebuf) - tmpsz,
			      *npos, (size_t)0, sz);
			if (sz < 0)
				break;
			tmpsz += sz;
			tmpsz += snprintf(linebuf + tmpsz, sizeof(linebuf) - tmpsz, "  ");
			prepare_text(data + offset, size - offset,
				     linebuf + tmpsz, sizeof(linebuf) - tmpsz, *npos, 0,
				     *opos == *npos ? black : fg, black);
			vfprintf(f, fmt, ap);
			fprintf(f, "%c\033[38:5:%dm%08lx  %s\n",
				opc[op], *opos == *npos ? black : fg, *npos, linebuf);
			break;
		case INSERT:
			sz = prepare_hex(data + offset, size - offset, &consumed,
					 linebuf + tmpsz, sizeof(linebuf) - tmpsz,
			                 *npos, 0);
			debug("prepare_hex(%p, %zd, %zd, %p, %zd, %zd, %zd) = %zd",
			      data + offset, size - offset, consumed,
			      linebuf + tmpsz, sizeof(linebuf) - tmpsz,
			      *npos, (size_t)0, sz);
			if (consumed == 0)
				break;
			if (sz < 0)
				break;
			tmpsz += sz;
			tmpsz += snprintf(linebuf + tmpsz, sizeof(linebuf) - tmpsz,
					  "  \033[38:5:%dm", black);
			tmpsz += prepare_text(data + offset, size - offset,
					      linebuf + tmpsz, sizeof(linebuf) - tmpsz,
					      *npos, 0, fg, black);
			vfprintf(f, fmt, ap);
			fprintf(f, "%c\033[38:5:%dm%08lx  %s\n",
				opc[op], fg, *npos, linebuf);
			break;
		case IGNORE:
			continue;
		default:
			break;
		}

		offset += consumed;

		switch (op) {
		case DELETE:
			*opos += consumed;
			break;
		case COPY:
			*opos += consumed;
			*npos += consumed;
			break;
		case INSERT:
			*npos += consumed;
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
void
fhexdifff(FILE *f, const char *const fmt, hexdiff_op_t op, uint64_t *oposp,
          uint64_t *nposp, uint8_t *data, size_t size, text_color_t fg,
	  ...)
{
	va_list ap;

	va_start(ap, fg);
	vfhexdifff(f, fmt, ap, op, oposp, nposp, data, size, fg);
	va_end(ap);
}

void
hexdiff(hexdiff_op_t op, uint64_t *opos, uint64_t *npos, void *data, size_t sz,
	text_color_t fg)
{
	hexdebug = true;
	switch (op) {
	case DELETE:
	case COPY:
	case INSERT:
		fhexdifff(stdout, "", op, opos, npos, data, sz, fg);
		break;
	case IGNORE:
		break;
	}
	hexdebug = false;
}

// vim:fenc=utf-8:tw=75:noet
