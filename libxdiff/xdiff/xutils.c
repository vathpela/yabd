/*
 *  LibXDiff by Davide Libenzi ( File Differential Library )
 *  Copyright (C) 2003	Davide Libenzi
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

#include "xinclude.h"

#define XDL_GUESS_NLINES 256

uint32_t
xdl_bogosqrt(uint32_t n)
{
	uint32_t i;

	/*
	 * Classical integer square root approximation using shifts.
	 */
	for (i = 1; n > 0; n >>= 2)
		i <<= 1;

	return i;
}

int
xdl_emit_diffrec(const char *rec, size_t size, const char *pre, size_t psize,
                 xdemitcb_t *ecb)
{
	int i = 2;
	mmbuffer_t mb[3];

	mb[0].ptr = (char *)pre;
	mb[0].size = psize;
	mb[1].ptr = (char *)rec;
	mb[1].size = size;
	if (size > 0 && rec[size - 1] != '\n') {
		mb[2].ptr = (char *)"\n\\ No newline at end of file\n";
		mb[2].size = strlen(mb[2].ptr);
		i++;
	}
	if (ecb->outf(ecb->priv, mb, i) < 0) {
		return -1;
	}

	return 0;
}

int
xdl_init_mmfile(mmfile_t *mmf, size_t bsize, uint32_t flags)
{
	mmf->flags = flags;
	mmf->head = mmf->tail = NULL;
	mmf->bsize = bsize;
	mmf->fsize = 0;
	mmf->rcur = mmf->wcur = NULL;
	mmf->rpos = 0;

	return 0;
}

void
xdl_free_mmfile(mmfile_t *mmf)
{
	mmblock_t *cur, *tmp;

	for (cur = mmf->head; (tmp = cur) != NULL;) {
		cur = cur->next;
		xdl_free(tmp);
	}
}

int
xdl_mmfile_iscompact(mmfile_t *mmf)
{
	return mmf->head == mmf->tail;
}

int
xdl_seek_mmfile(mmfile_t *mmf, size_t pos)
{
	size_t bsize;

	if (xdl_mmfile_first(mmf, &bsize)) {
		do {
			if (pos < bsize) {
				mmf->rpos = pos;
				return 0;
			}
			pos -= bsize;
		} while (xdl_mmfile_next(mmf, &bsize));
	}

	return -1;
}

ssize_t
xdl_read_mmfile(mmfile_t *mmf, void *data, size_t size)
{
	size_t rsize, csize;
	char *ptr = data;
	mmblock_t *rcur;

	for (rsize = 0, rcur = mmf->rcur; rcur && rsize < size;) {
		if (mmf->rpos >= rcur->size) {
			if (!(mmf->rcur = rcur = rcur->next))
				break;
			mmf->rpos = 0;
		}
		csize = XDL_MIN(size - rsize, rcur->size - mmf->rpos);
		memcpy(ptr, rcur->ptr + mmf->rpos, csize);
		rsize += csize;
		ptr += csize;
		mmf->rpos += csize;
	}

	return rsize;
}

ssize_t
xdl_write_mmfile(mmfile_t *mmf, void const *data, size_t size)
{
	size_t wsize, bsize, csize;
	mmblock_t *wcur;

	for (wsize = 0; wsize < size;) {
		wcur = mmf->wcur;
		if (wcur && (wcur->flags & XDL_MMB_READONLY))
			return wsize;
		if (!wcur || wcur->size == wcur->bsize ||
		    (mmf->flags & XDL_MMF_ATOMIC &&
		     wcur->size + size > wcur->bsize)) {
			bsize = XDL_MAX(mmf->bsize, size);
			if (!(wcur = (mmblock_t *)xdl_malloc(sizeof(mmblock_t) +
			                                     bsize))) {
				return wsize;
			}
			wcur->flags = 0;
			wcur->ptr = (char *)wcur + sizeof(mmblock_t);
			wcur->size = 0;
			wcur->bsize = bsize;
			wcur->next = NULL;
			if (!mmf->head)
				mmf->head = wcur;
			if (mmf->tail)
				mmf->tail->next = wcur;
			mmf->tail = wcur;
			mmf->wcur = wcur;
		}
		csize = XDL_MIN(size - wsize, wcur->bsize - wcur->size);
		memcpy(wcur->ptr + wcur->size, (const char *)data + wsize,
		       csize);
		wsize += csize;
		wcur->size += csize;
		mmf->fsize += csize;
	}

	return size;
}

ssize_t
xdl_writem_mmfile(mmfile_t *mmf, mmbuffer_t *mb, size_t nbuf)
{
	size_t i;
	size_t size;
	char *data;

	for (i = 0, size = 0; i < nbuf; i++)
		size += mb[i].size;
	if (!(data = (char *)xdl_mmfile_writeallocate(mmf, size)))
		return -1;
	for (i = 0; i < nbuf; i++) {
		memcpy(data, mb[i].ptr, mb[i].size);
		data += mb[i].size;
	}

	return size;
}

void *
xdl_mmfile_writeallocate(mmfile_t *mmf, size_t size)
{
	long bsize;
	mmblock_t *wcur;
	char *blk;

	if (!(wcur = mmf->wcur) || wcur->size + size > wcur->bsize) {
		bsize = XDL_MAX(mmf->bsize, size);
		if (!(wcur = (mmblock_t *)xdl_malloc(sizeof(mmblock_t) +
		                                     bsize))) {
			return NULL;
		}
		wcur->flags = 0;
		wcur->ptr = (char *)wcur + sizeof(mmblock_t);
		wcur->size = 0;
		wcur->bsize = bsize;
		wcur->next = NULL;
		if (!mmf->head)
			mmf->head = wcur;
		if (mmf->tail)
			mmf->tail->next = wcur;
		mmf->tail = wcur;
		mmf->wcur = wcur;
	}

	blk = wcur->ptr + wcur->size;
	wcur->size += size;
	mmf->fsize += size;

	return blk;
}

long
xdl_mmfile_ptradd(mmfile_t *mmf, char *ptr, size_t size, uint32_t flags)
{
	mmblock_t *wcur;

	if (!(wcur = (mmblock_t *)xdl_malloc(sizeof(mmblock_t)))) {
		return -1;
	}
	wcur->flags = flags;
	wcur->ptr = ptr;
	wcur->size = wcur->bsize = size;
	wcur->next = NULL;
	if (!mmf->head)
		mmf->head = wcur;
	if (mmf->tail)
		mmf->tail->next = wcur;
	mmf->tail = wcur;
	mmf->wcur = wcur;

	mmf->fsize += size;

	return size;
}

size_t
xdl_copy_mmfile(mmfile_t *mmf, size_t size, xdemitcb_t *ecb)
{
	size_t rsize, csize;
	mmblock_t *rcur;
	mmbuffer_t mb;

	for (rsize = 0, rcur = mmf->rcur; rcur && rsize < size;) {
		if (mmf->rpos >= rcur->size) {
			if (!(mmf->rcur = rcur = rcur->next))
				break;
			mmf->rpos = 0;
		}
		csize = XDL_MIN(size - rsize, rcur->size - mmf->rpos);
		mb.ptr = rcur->ptr + mmf->rpos;
		mb.size = csize;
		if (ecb->outf(ecb->priv, &mb, 1) < 0) {
			return rsize;
		}
		rsize += csize;
		mmf->rpos += csize;
	}

	return rsize;
}

void *
xdl_mmfile_first(mmfile_t *mmf, size_t *size)
{
	if (!(mmf->rcur = mmf->head))
		return NULL;

	*size = mmf->rcur->size;

	return mmf->rcur->ptr;
}

void *
xdl_mmfile_next(mmfile_t *mmf, size_t *size)
{
	if (!mmf->rcur || !(mmf->rcur = mmf->rcur->next))
		return NULL;

	*size = mmf->rcur->size;

	return mmf->rcur->ptr;
}

long
xdl_mmfile_size(mmfile_t *mmf)
{
	return mmf->fsize;
}

int
xdl_mmfile_cmp(mmfile_t *mmf1, mmfile_t *mmf2)
{
	int cres;
	size_t size, bsize1, bsize2, size1, size2;
	const char *blk1, *cur1, *top1;
	const char *blk2, *cur2, *top2;

	if ((cur1 = blk1 = xdl_mmfile_first(mmf1, &bsize1)) != NULL)
		top1 = blk1 + bsize1;
	if ((cur2 = blk2 = xdl_mmfile_first(mmf2, &bsize2)) != NULL)
		top2 = blk2 + bsize2;
	if (!cur1) {
		if (!cur2 || xdl_mmfile_size(mmf2) == 0)
			return 0;
		return -*cur2;
	} else if (!cur2)
		return xdl_mmfile_size(mmf1) ? *cur1 : 0;
	for (;;) {
		if (cur1 >= top1) {
			if ((cur1 = blk1 = xdl_mmfile_next(mmf1, &bsize1)) !=
			    NULL)
				top1 = blk1 + bsize1;
		}
		if (cur2 >= top2) {
			if ((cur2 = blk2 = xdl_mmfile_next(mmf2, &bsize2)) !=
			    NULL)
				top2 = blk2 + bsize2;
		}
		if (!cur1) {
			if (!cur2)
				break;
			return -*cur2;
		} else if (!cur2)
			return *cur1;
		size1 = top1 - cur1;
		size2 = top2 - cur2;
		size = XDL_MIN(size1, size2);
		if ((cres = memcmp(cur1, cur2, size)) != 0)
			return cres;
		cur1 += size;
		cur2 += size;
	}

	return 0;
}

int
xdl_mmfile_compact(mmfile_t *mmfo, mmfile_t *mmfc, size_t bsize, uint32_t flags)
{
	size_t fsize = xdl_mmfile_size(mmfo), size;
	char *data;
	const char *blk;

	if (xdl_init_mmfile(mmfc, bsize, flags) < 0) {
		return -1;
	}
	if (!(data = (char *)xdl_mmfile_writeallocate(mmfc, fsize))) {
		xdl_free_mmfile(mmfc);
		return -1;
	}
	if ((blk = (const char *)xdl_mmfile_first(mmfo, &size)) != NULL) {
		do {
			memcpy(data, blk, size);
			data += size;
		} while ((blk = (const char *)xdl_mmfile_next(mmfo, &size)) !=
		         NULL);
	}

	return 0;
}

int
xdl_mmfile_outf(void *priv, mmbuffer_t *mb, size_t nbuf)
{
	mmfile_t *mmf = priv;

	if (xdl_writem_mmfile(mmf, mb, nbuf) < 0) {
		return -1;
	}

	return 0;
}

int
xdl_cha_init(chastore_t *cha, size_t isize, size_t icount)
{
	cha->head = cha->tail = NULL;
	cha->isize = isize;
	cha->nsize = icount * isize;
	cha->ancur = cha->sncur = NULL;
	cha->scurr = 0;

	return 0;
}

void
xdl_cha_free(chastore_t *cha)
{
	chanode_t *cur, *tmp;

	for (cur = cha->head; (tmp = cur) != NULL;) {
		cur = cur->next;
		xdl_free(tmp);
	}
}

void *
xdl_cha_alloc(chastore_t *cha)
{
	chanode_t *ancur;
	void *data;

	if (!(ancur = cha->ancur) || ancur->icurr == cha->nsize) {
		if (!(ancur = (chanode_t *)xdl_malloc(sizeof(chanode_t) +
		                                      cha->nsize))) {
			return NULL;
		}
		ancur->icurr = 0;
		ancur->next = NULL;
		if (cha->tail)
			cha->tail->next = ancur;
		if (!cha->head)
			cha->head = ancur;
		cha->tail = ancur;
		cha->ancur = ancur;
	}

	data = (char *)ancur + sizeof(chanode_t) + ancur->icurr;
	ancur->icurr += cha->isize;

	return data;
}

void *
xdl_cha_first(chastore_t *cha)
{
	chanode_t *sncur;

	if (!(cha->sncur = sncur = cha->head))
		return NULL;

	cha->scurr = 0;

	return (char *)sncur + sizeof(chanode_t) + cha->scurr;
}

void *
xdl_cha_next(chastore_t *cha)
{
	chanode_t *sncur;

	if (!(sncur = cha->sncur))
		return NULL;
	cha->scurr += cha->isize;
	if (cha->scurr == sncur->icurr) {
		if (!(sncur = cha->sncur = sncur->next))
			return NULL;
		cha->scurr = 0;
	}

	return (char *)sncur + sizeof(chanode_t) + cha->scurr;
}

size_t
xdl_guess_lines(mmfile_t *mf)
{
	size_t nl = 0, size, tsize = 0;
	const char *data, *cur, *top;

	if ((cur = data = xdl_mmfile_first(mf, &size)) != NULL) {
		for (top = data + size; nl < XDL_GUESS_NLINES;) {
			if (cur >= top) {
				tsize += (long)(cur - data);
				if (!(cur = data = xdl_mmfile_next(mf, &size)))
					break;
				top = data + size;
			}
			nl++;
			if (!(cur = memchr(cur, '\n', top - cur)))
				cur = top;
			else
				cur++;
		}
		tsize += (long)(cur - data);
	}

	if (nl && tsize)
		nl = xdl_mmfile_size(mf) / (tsize / nl);

	return nl + 1;
}

unsigned long
xdl_hash_record(const char **data, const char *top)
{
	unsigned long ha = 5381;
	const char *ptr = *data;

	for (; ptr < top && *ptr != '\n'; ptr++) {
		ha += (ha << 5);
		ha ^= (unsigned long)*ptr;
	}
	*data = ptr < top ? ptr + 1 : ptr;

	return ha;
}

unsigned int
xdl_hashbits(size_t size)
{
	unsigned int val = 1, bits = 0;

	for (; val < size && bits < CHAR_BIT * sizeof(unsigned int);
	     val <<= 1, bits++)
		;
	return bits ? bits : 1;
}

int
xdl_num_out(char *out, long val)
{
	char *ptr, *str = out;
	char buf[32];

	ptr = buf + sizeof(buf) - 1;
	*ptr = '\0';
	if (val < 0) {
		*--ptr = '-';
		val = -val;
	}
	for (; val && ptr > buf; val /= 10)
		*--ptr = "0123456789"[val % 10];
	if (*ptr)
		for (; *ptr; ptr++, str++)
			*str = *ptr;
	else
		*str++ = '0';
	*str = '\0';

	return str - out;
}

long
xdl_atol(const char *str, const char **next)
{
	long val, base;
	const char *top;

	for (top = str; XDL_ISDIGIT(*top); top++)
		;
	if (next)
		*next = top;
	for (val = 0, base = 1, top--; top >= str; top--, base *= 10)
		val += base * (long)(*top - '0');
	return val;
}

int
xdl_emit_hunk_hdr(size_t s1, size_t c1, size_t s2, size_t c2, xdemitcb_t *ecb)
{
	int nb = 0;
	mmbuffer_t mb;
	char buf[128];

	memcpy(buf, "@@ -", 4);
	nb += 4;

	nb += xdl_num_out(buf + nb, c1 ? s1 : s1 - 1);

	memcpy(buf + nb, ",", 1);
	nb += 1;

	nb += xdl_num_out(buf + nb, c1);

	memcpy(buf + nb, " +", 2);
	nb += 2;

	nb += xdl_num_out(buf + nb, c2 ? s2 : s2 - 1);

	memcpy(buf + nb, ",", 1);
	nb += 1;

	nb += xdl_num_out(buf + nb, c2);

	memcpy(buf + nb, " @@\n", 4);
	nb += 4;

	mb.ptr = buf;
	mb.size = nb;
	if (ecb->outf(ecb->priv, &mb, 1) < 0)
		return -1;

	return 0;
}
