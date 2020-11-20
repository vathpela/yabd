// SPDX-License-Identifier: GPLv3-or-later
/*
 * color.h - simple console color primitives
 * Copyright Peter Jones <pjones@redhat.com>
 */

#ifndef COLOR_H_
#define COLOR_H_

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

typedef enum color_e
{
	no_color_change = -1,
	black = 0,
	red = 1,
	green = 2,
	yellow = 3,
	blue = 4,
	magenta = 5,
	cyan = 6,
	light_gray = 7,
	dark_gray = 8,
	briht_red = 9,
	bright_green = 10,
	bright_yellow = 11,
	bright_blue = 12,
	bright_magenta = 13,
	bright_cyan = 14,
	white = 15,
} text_color_t;

typedef enum attr_e
{
	no_attr_change = -1,
	none = 0,
	standout = 1,
	underline = 2,
	reverse = 3,
	blink = 4,
	bold = 6,
	invis = 7,
} text_attr_t;

struct color {
	text_attr_t attr;
	text_color_t bg;
	text_color_t fg;
};

ssize_t vsncprintf(char *ibuf, size_t ibufsz, struct color color, char *fmt,
                   va_list ap);
ssize_t sncprintf(char *buf, size_t bufsz, struct color color, char *fmt, ...);
ssize_t vfcprintf(FILE *out, struct color color, char *fmt, va_list ap);
ssize_t fcprintf(FILE *out, struct color color, char *fmt, ...);
ssize_t vascprintf(char **ibuf, struct color color, char *fmt, va_list ap);
ssize_t ascprintf(char **buf, struct color color, char *fmt, ...);
FILE *color_fopen(const char *path, const char *mode);
FILE *color_fdopen(int fd, const char *mode);

#endif /* !COLOR_H_ */
// vim:fenc=utf-8:tw=75:noet
