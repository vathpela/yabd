/*
 *  LibXDiff by Davide Libenzi ( File Differential Library )
 *  Copyright (C) 2003  Davide Libenzi
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  Davide Libenzi <davidel@xmailserver.org>
 *
 */

#if !defined(XUTILS_H)
#define XUTILS_H

uint32_t xdl_bogosqrt(uint32_t n);
int xdl_emit_diffrec(const char *rec, size_t size, const char *pre,
                     size_t psize, xdemitcb_t *ecb);
int xdl_mmfile_outf(void *priv, mmbuffer_t *mb, size_t nbuf);
int xdl_cha_init(chastore_t *cha, size_t isize, size_t icount);
void xdl_cha_free(chastore_t *cha);
void *xdl_cha_alloc(chastore_t *cha);
void *xdl_cha_first(chastore_t *cha);
void *xdl_cha_next(chastore_t *cha);
size_t xdl_guess_lines(mmfile_t *mf);
unsigned long xdl_hash_record(const char **data, const char *top);
unsigned int xdl_hashbits(size_t size);
int xdl_num_out(char *out, long val);
long xdl_atol(const char *str, const char **next);
int xdl_emit_hunk_hdr(size_t s1, size_t c1, size_t s2, size_t c2,
                      xdemitcb_t *ecb);

#endif /* #if !defined(XUTILS_H) */
