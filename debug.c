// SPDX-License-Identifier: GPLv3-or-later
/*
 * debug.c - debugging helper functions
 * Copyright Peter Jones <pjones@redhat.com>
 */

#include "bindiff.h"

PUBLIC bool unc_debug_arg_ = false;
PUBLIC bool unc_debug_once_ = true;
PUBLIC int unc_debug_pfx_len_ = 0;

/**
 * test a debug criterion
 *
 * XXX: currently ignores criterion
 */
PUBLIC bool
unc_get_debug(const char *criterion UNUSED)
{
	return unc_debug_arg_;
}

/**
 * set a debug criterion
 *
 * XXX: currently ignores criterion
 */
PUBLIC void
unc_set_debug(const char *criterion UNUSED, const bool enable)
{
	unc_debug_arg_ = enable;
}

PUBLIC ssize_t
dprintc(int c)
{
	ssize_t rc;
	if (isprint(c) || c == '\t' || c == '\n')
		rc = dprintf("%c", c);
	else
		rc = dprintf("\\%03hho", c);
	return rc;
}

PUBLIC ssize_t
dprints(const char *const s, ssize_t sz)
{
	ssize_t max = 0;
	if (!s) {
		return dprintf("(NULL)");
	}
	dprintf("\"");
	for (unsigned int i = 0; (sz < 0 && s && s[i] != 0); i++) {
		dprintc(s[i]);
		max = MAX(i, max);
	}
	for (unsigned int i = 0; (sz >= 0 && s && i < sz); i++) {
		dprintc(s[i]);
		max = MAX(i, max);
	}
	dprintf("\"");
	return max + 2;
}

PUBLIC ssize_t
vdebug_(const char *file, const int line, const char *func, const char *fmt,
        va_list args)
{
	va_list args2;
	ssize_t rc = 0;

	if (unc_get_debug(NULL)) {
		va_copy(args2, args);
		printf("%s:%d:%s() ", file, line, func);
		rc = vprintf(fmt, args2);
		va_end(args2);
	}
	return rc;
}

void
dump_maps(void)
{
	char *cmd = NULL;
	int rc;

	if (!unc_debug_arg(NULL))
		return;

	rc = asprintf(&cmd, "cat /proc/%d/maps >> /dev/stderr", getpid());
	if (rc > 0 && cmd != NULL)
		system(cmd);
	free(cmd);
}

// vim:fenc=utf-8:tw=75:noet
