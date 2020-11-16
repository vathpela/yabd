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

#if !defined(XDIFF_H)
#define XDIFF_H

#ifdef __cplusplus
extern "C" {
#endif /* #ifdef __cplusplus */

#ifdef _WIN32
#ifdef LIBXDIFF_DLL_EXPORT
#define LIBXDIFF_EXPORT __declspec(dllexport)
#else
#define LIBXDIFF_EXPORT __declspec(dllimport)
#endif
#else
#define LIBXDIFF_EXPORT
#endif

#define XDF_NEED_MINIMAL (1 << 1)

#define XDL_PATCH_NORMAL '-'
#define XDL_PATCH_REVERSE '+'
#define XDL_PATCH_MODEMASK ((1 << 8) - 1)
#define XDL_PATCH_IGNOREBSPACE (1 << 8)

#define XDL_MMB_READONLY (1 << 0)

#define XDL_MMF_ATOMIC (1 << 0)

#define XDL_BDOP_INS 1
#define XDL_BDOP_CPY 2
#define XDL_BDOP_INSB 3

LIBXDIFF_EXPORT typedef struct s_memallocator {
	void *priv;
	void *(*malloc)(void *, size_t);
	void (*free)(void *, void *);
	void *(*realloc)(void *, void *, size_t);
} memallocator_t;

LIBXDIFF_EXPORT typedef struct s_mmblock {
	struct s_mmblock *next;
	uint32_t flags;
	size_t size, bsize;
	char *ptr;
} mmblock_t;

LIBXDIFF_EXPORT typedef struct s_mmfile {
	uint32_t flags;
	mmblock_t *head, *tail;
	size_t bsize, fsize, rpos;
	mmblock_t *rcur, *wcur;
} mmfile_t;

LIBXDIFF_EXPORT typedef struct s_mmbuffer {
	char *ptr;
	size_t size;
} mmbuffer_t;

LIBXDIFF_EXPORT typedef struct s_xpparam {
	uint32_t flags;
} xpparam_t;

LIBXDIFF_EXPORT typedef struct s_xdemitcb {
	void *priv;
	int (*outf)(void *priv, mmbuffer_t *bufs, size_t n_bufs);
} xdemitcb_t;

LIBXDIFF_EXPORT typedef struct s_xdemitconf {
	size_t ctxlen;
} xdemitconf_t;

LIBXDIFF_EXPORT typedef struct s_bdiffparam {
	size_t bsize;
} bdiffparam_t;

LIBXDIFF_EXPORT int xdl_set_allocator(memallocator_t const *malt);
LIBXDIFF_EXPORT void *xdl_malloc(size_t size);
LIBXDIFF_EXPORT void xdl_free(void *ptr);
LIBXDIFF_EXPORT void *xdl_realloc(void *ptr, size_t size);

LIBXDIFF_EXPORT int xdl_init_mmfile(mmfile_t *mmf, size_t bsize,
                                    uint32_t flags);
LIBXDIFF_EXPORT void xdl_free_mmfile(mmfile_t *mmf);
LIBXDIFF_EXPORT int xdl_mmfile_iscompact(mmfile_t *mmf);
LIBXDIFF_EXPORT int xdl_seek_mmfile(mmfile_t *mmf, size_t pos);
LIBXDIFF_EXPORT ssize_t xdl_read_mmfile(mmfile_t *mmf, void *data, size_t size);
LIBXDIFF_EXPORT ssize_t xdl_write_mmfile(mmfile_t *mmf, const void *data,
                                         size_t size);
LIBXDIFF_EXPORT ssize_t xdl_writem_mmfile(mmfile_t *mmf, mmbuffer_t *mb,
                                          size_t nbuf);
LIBXDIFF_EXPORT void *xdl_mmfile_writeallocate(mmfile_t *mmf, size_t size);
LIBXDIFF_EXPORT long xdl_mmfile_ptradd(mmfile_t *mmf, char *ptr, size_t size,
                                       uint32_t flags);
LIBXDIFF_EXPORT size_t xdl_copy_mmfile(mmfile_t *mmf, size_t size,
                                       xdemitcb_t *ecb);
LIBXDIFF_EXPORT void *xdl_mmfile_first(mmfile_t *mmf, size_t *size);
LIBXDIFF_EXPORT void *xdl_mmfile_next(mmfile_t *mmf, size_t *size);
LIBXDIFF_EXPORT long xdl_mmfile_size(mmfile_t *mmf);
LIBXDIFF_EXPORT int xdl_mmfile_cmp(mmfile_t *mmf1, mmfile_t *mmf2);
LIBXDIFF_EXPORT int xdl_mmfile_compact(mmfile_t *mmfo, mmfile_t *mmfc,
                                       size_t bsize, uint32_t flags);

LIBXDIFF_EXPORT int xdl_diff(mmfile_t *mf1, mmfile_t *mf2, const xpparam_t *xpp,
                             const xdemitconf_t *xecfg, xdemitcb_t *ecb);
LIBXDIFF_EXPORT int xdl_patch(mmfile_t *mf, mmfile_t *mfp, int mode,
                              xdemitcb_t *ecb, xdemitcb_t *rjecb);

LIBXDIFF_EXPORT int xdl_merge3(mmfile_t *mmfo, mmfile_t *mmf1, mmfile_t *mmf2,
                               xdemitcb_t *ecb, xdemitcb_t *rjecb);

LIBXDIFF_EXPORT int xdl_bdiff_mb(mmbuffer_t *mmb1, mmbuffer_t *mmb2,
                                 const bdiffparam_t *bdp, xdemitcb_t *ecb);
LIBXDIFF_EXPORT int xdl_bdiff(mmfile_t *mmf1, mmfile_t *mmf2,
                              const bdiffparam_t *bdp, xdemitcb_t *ecb);
LIBXDIFF_EXPORT int xdl_rabdiff_mb(mmbuffer_t *mmb1, mmbuffer_t *mmb2,
                                   xdemitcb_t *ecb);
LIBXDIFF_EXPORT int xdl_rabdiff(mmfile_t *mmf1, mmfile_t *mmf2,
                                xdemitcb_t *ecb);
LIBXDIFF_EXPORT size_t xdl_bdiff_tgsize(mmfile_t *mmfp);
LIBXDIFF_EXPORT int xdl_bpatch(mmfile_t *mmf, mmfile_t *mmfp, xdemitcb_t *ecb);
LIBXDIFF_EXPORT int xdl_bpatch_multi(mmbuffer_t *base, mmbuffer_t *mbpch, int n,
                                     xdemitcb_t *ecb);

#ifdef __cplusplus
}
#endif /* #ifdef __cplusplus */
#endif /* #if !defined(XDIFF_H) */
