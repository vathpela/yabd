// SPDX-License-Identifier: GPLv3-or-later
/*
 * color.c - format and print colored text
 * Copyright Peter Jones <pjones@redhat.com>
 */

#include "bindiff.h"

static inline int UNUSED
get_cursor_position(int fd, unsigned int *row, unsigned int *col)
{
	return 0;
}

static inline ssize_t UNUSED
save_cursor(char *buf, size_t bufsz)
{
	return snprintf(buf, bufsz, "\x1b\x37");
}

static inline ssize_t UNUSED
restore_cursor(char *buf, size_t bufsz)
{
	return snprintf(buf, bufsz, "\x1b\x38");
}

static inline ssize_t UNUSED
set_bg(char *buf, size_t bufsz, text_color_t color)
{
	return snprintf(buf, bufsz, "\033[48:5:%dm", color);
}

static inline ssize_t UNUSED
set_fg(char *buf, size_t bufsz, text_color_t color)
{
	return snprintf(buf, bufsz, "\033[38:5:%dm", color);
}

#define snpf(fn, off_, b_, bs_, args_...)                          \
	({                                                         \
		ssize_t rc_;                                       \
		debug("buf:%p bufsz:%zd off:%zd", b_, bs_, off_);  \
		rc_ = fn(((b_) ? ((b_) + (off_)) : NULL),          \
		         ((bs_) ? ((bs_) - (off_)) : 0), ##args_); \
		if (rc_ >= 0)                                      \
			(off) += rc_;                              \
		rc_;                                               \
	})

ssize_t
vsncprintf(char *ibuf, size_t ibufsz, struct color color, char *fmt, va_list ap)
{
	va_list aq;
	ssize_t rc = 0, sz;
	off_t off = 0;
	char *buf = (ibuf && ibufsz) ? ibuf : NULL;
	size_t bufsz = buf ? ibufsz : 0;
	int tty;
	size_t deltav = 0, deltah = 0;

	tty = isatty(STDOUT_FILENO);
	debug("isatty(%d):%d", STDOUT_FILENO, tty);
	if (tty < 0)
		tty = 0;
	tty = 1;

	if (tty) {
		sz = snpf(save_cursor, off, buf, bufsz);
		if (sz < 0)
			return sz;
		rc += sz;

		if (color.bg != no_color_change) {
			sz = snpf(set_bg, off, buf, bufsz, color.bg);
			if (sz < 0)
				return sz;
			rc += sz;
		}

		if (color.fg != no_color_change) {
			sz = snpf(set_fg, off, buf, bufsz, color.fg);
			if (sz < 0)
				return sz;
			rc += sz;
		}
	}

	sz = snpf(vsnprintf, off, buf, bufsz, fmt, ap);
	if (sz < 0)
		return sz;
	rc += sz;

	if (tty) {
		sz = snpf(restore_cursor, off, buf, bufsz);
		if (sz < 0)
			return sz;
		rc += sz;

#if 0
		sz = snpf(snprintf, off, buf, bufsz, "\x1b[%zuC ", vis);
		if (sz < 0)
			return sz;
		rc += sz;
#endif
	}

	return rc;
}
#undef snpf

ssize_t
sncprintf(char *buf, size_t bufsz, struct color color, char *fmt, ...)
{
	va_list ap;
	ssize_t rc;

	va_start(ap, fmt);
	rc = vsncprintf(buf, bufsz, color, fmt, ap);
	va_end(ap);
	return rc;
}

ssize_t
vfcprintf(FILE *out, struct color color, char *fmt, va_list ap)
{
	ssize_t sz;
	char *buf = NULL;
	size_t bufsz = 0;
	va_list aq;

	va_copy(aq, ap);
	sz = vsncprintf(buf, bufsz, color, fmt, ap);
	va_end(ap);
	if (sz <= 0) {
		return sz;
	}

	bufsz = sz + 1;
	buf = alloca(sz);
	if (!buf) {
		errno = ENOMEM;
		return -1;
	}

	memset(buf, 0, bufsz);
	sz = vsncprintf(buf, bufsz, color, fmt, aq);
	if (sz <= 0)
		return sz;

	dhexdumpf("", buf, bufsz);
	sz = fwrite(buf, 1, bufsz, out);
	return sz;
}

ssize_t
fcprintf(FILE *out, struct color color, char *fmt, ...)
{
	va_list ap;
	ssize_t sz;

	va_start(ap, fmt);
	sz = vfcprintf(out, color, fmt, ap);
	va_end(ap);
	return sz;
}

ssize_t
vascprintf(char **ibuf, struct color color, char *fmt, va_list ap)
{
	ssize_t sz;
	char *buf = NULL;
	size_t bufsz = 0;
	va_list aq;

	if (!ibuf) {
		errno = EINVAL;
		return -1;
	}

	va_copy(aq, ap);
	sz = vsncprintf(buf, bufsz, color, fmt, ap);
	va_end(ap);
	if (sz <= 0) {
		if (sz == 0) {
			*ibuf = NULL;
		}
		return sz;
	}

	bufsz = sz;
	buf = calloc(1, sz);
	if (!buf) {
		errno = ENOMEM;
		return -1;
	}

	sz = vsncprintf(buf, bufsz, color, fmt, aq);
	return sz;
}

ssize_t
ascprintf(char **buf, struct color color, char *fmt, ...)
{
	va_list ap;
	ssize_t sz;

	va_start(ap, fmt);
	sz = vascprintf(buf, color, fmt, ap);
	va_end(ap);
	return sz;
}

struct color_cookie {
	FILE *filep;
	int fd;
};

static ssize_t
color_read_priv(void *cookiep, char *buf, size_t size)
{
	struct color_cookie *cookie = cookiep;
	fpos_t pos = {
		0,
	};
	bool setpos = false;
	ssize_t sz;
	int rc;

	rc = fgetpos(cookie->filep, &pos);
	if (rc >= 0)
		setpos = true;

	sz = fread(buf, 1, size, cookie->filep);

	if (setpos)
		fsetpos(cookie->filep, &pos);

	return sz;
}

static ssize_t
color_read(void *cookiep, char *buf, size_t size)
{
	struct color_cookie *cookie = cookiep;
	size_t sz;

	return fread(buf, 1, size, cookie->filep);
}

static inline const char *
strnchrnul(const char *const buf, const size_t limit, const int c)
{
	size_t pos;
	for (pos = 0; pos < limit && buf[pos] && buf[pos] != c; pos++)
		;

	return &buf[pos];
}

#define CWBUF_ (buf + off)
#define CWLEN_ (size - off)

static ssize_t
color_write_(const void *const cookiep, const char *const buf,
             const size_t size)
{
	const struct color_cookie *const cookie = cookiep;
	ssize_t sz = 0;
	off_t off = 0;
	const char *nextesc;
	char pagebuf[4096];

	if (size == 0)
		return 0;

	if (!buf) {
		errno = EINVAL;
		return -1;
	}

	nextesc = strnchrnul(CWBUF_, CWLEN_, '\033');
	do {
		const char *esc = nextesc;
		size_t limit = MIN(size - (esc - buf), 4096);
		ssize_t escsz;
		ssize_t osz, isz;

		nextesc = strnchrnul(CWBUF_, CWLEN_, '\033');

		if (esc > CWBUF_) {
			isz = esc - CWBUF_;

			osz = fwrite(CWBUF_, 1, isz, cookie->filep);
			sz += osz;
			off += osz;
			if (osz < isz)
				return sz;
			if (sz >= 0 && (size_t)sz == size)
				break;
		}

		// escsz = get_esc_sz(CWBUF_, MIN(CWLEN_, nextesc - CWBUF_));
		isz = nextesc - CWBUF_;
		osz = fwrite(CWBUF_, 1, isz, cookie->filep);
		sz += osz;
		off += osz;
		if (osz < isz)
			return sz;
		if (sz >= 0 && (size_t)sz == size)
			break;
	} while (nextesc && *nextesc && sz >= 0 && (size_t)sz < size);

	return sz;
}

static ssize_t
color_write(void *cookiep, const char *buf, size_t size)
{
	return color_write_(cookiep, buf, size);
}

static int
color_seek(void *cookiep, off64_t *offset, int whence)
{
	struct color_cookie *cookie = cookiep;
	long pos;
	int rc;

	rc = fseek(cookie->filep, *offset, whence);
	if (rc >= 0) {
		pos = ftell(cookie->filep);
		if (pos < 0) {
			rc = -1;
		} else {
			*offset = pos;
		}
	}

	return rc;
}

static int
color_close(void *cookiep)
{
	struct color_cookie *cookie = cookiep;

	return fclose(cookie->filep);
}

cookie_io_functions_t color_io_funcs = {
	.read = color_read,
	.write = color_write,
	.seek = color_seek,
	.close = color_close,
};

FILE *
color_fdopen(int fd, const char *mode)
{
	FILE *ret;
	int errnum;
	struct color_cookie *cookie;

	cookie = calloc(1, sizeof(*cookie));
	if (!cookie)
		return NULL;

	cookie->fd = fd;
	cookie->filep = fdopen(fd, mode);
	if (!cookie->filep)
		goto err_free;

	setvbuf(cookie->filep, NULL, _IONBF, 0);

	ret = fopencookie(cookie, mode, color_io_funcs);
	if (!ret)
		goto err_fclose;

	return ret;

err_fclose:
	errnum = errno;
	fclose(cookie->filep);
	errno = errnum;
err_free:
	errnum = errno;
	free(cookie);
	errno = errnum;

	return NULL;
}

FILE *
color_fopen(const char *path, const char *mode)
{
	FILE *ret;
	int errnum;
	struct color_cookie *cookie;

	cookie = calloc(1, sizeof(*cookie));
	if (!cookie)
		return NULL;

	cookie->filep = fopen(path, mode);
	if (!cookie->filep)
		goto err_free;

	setvbuf(cookie->filep, NULL, _IONBF, 0);

	cookie->fd = fileno(cookie->filep);
	if (cookie->fd < 0)
		goto err_fclose;

	ret = fopencookie(cookie, mode, color_io_funcs);
	if (!ret)
		goto err_fclose;

	return ret;

err_fclose:
	errnum = errno;
	fclose(cookie->filep);
	errno = errnum;
err_free:
	errnum = errno;
	free(cookie);
	errno = errnum;

	return NULL;
}

// vim:fenc=utf-8:tw=75:noet
