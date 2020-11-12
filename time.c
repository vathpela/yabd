// SPDX-License-Identifier: GPLv3-or-later
/*
 * time.c - time stuff that can't be inlined
 * Copyright Peter Jones <pjones@redhat.com>
 */

#include "bindiff.h"

const algebra_t tsalg = {
	.size = sizeof(struct timespec),
	.set = (alg_set_t *)tsset,
	.cmp = (alg_cmp_t *)tscmp,
	.abs = (alg_abs_t *)tsabs,
	.add = (alg_add_t *)tsadd,
	.sub = (alg_sub_t *)tssub,
	.mul = (alg_mul_t *)tsmul,
	.divmod = (alg_divmod_t *)tsdivmod,
};

const algebra_t tvalg = {
	.size = sizeof(struct timeval),
	.set = (alg_set_t *)tvset,
	.cmp = (alg_cmp_t *)tvcmp,
	.abs = (alg_abs_t *)tvabs,
	.add = (alg_add_t *)tvadd,
	.sub = (alg_sub_t *)tvsub,
	.mul = (alg_mul_t *)tvmul,
	.divmod = (alg_divmod_t *)tvdivmod,
};

// vim:fenc=utf-8:tw=75:noet
