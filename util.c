// SPDX-License-Identifier: GPLv3-or-later
/*
 * util.c
 * Copyright Peter Jones <pjones@redhat.com>
 */

#include "bindiff.h"

int
envtolong(const char *const env, unsigned long *valp)
{
	char *tmp = NULL;
	unsigned long val;

	if (env)
		tmp = getenv(env);
	if (!tmp)
		return -1;

	errno = 0;
	val = strtol(tmp, NULL, 0);
	if (errno != 0)
		return -1;

	*valp = val;
	return 0;
}

// vim:fenc=utf-8:tw=75:noet
