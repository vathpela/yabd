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

#if !defined(XDIFFI_H)
#define XDIFFI_H

typedef struct s_diffdata {
	size_t nrec;
	const unsigned long *ha;
	size_t *rindex;
	char *rchg;
} diffdata_t;

typedef struct s_xdalgoenv {
	long mxcost;
	long snake_cnt;
	long heur_min;
} xdalgoenv_t;

typedef struct s_xdchange {
	struct s_xdchange *next;
	size_t i1, i2;
	size_t chg1, chg2;
} xdchange_t;

int xdl_recs_cmp(diffdata_t *dd1, size_t off1, size_t lim1, diffdata_t *dd2,
                 size_t off2, size_t lim2, long *kvdf, long *kvdb, int need_min,
                 xdalgoenv_t *xenv);
int xdl_do_diff(mmfile_t *mf1, mmfile_t *mf2, xpparam_t const *xpp,
                xdfenv_t *xe);
int xdl_build_script(xdfenv_t *xe, xdchange_t **xscr);
void xdl_free_script(xdchange_t *xscr);
int xdl_emit_diff(xdfenv_t *xe, xdchange_t *xscr, xdemitcb_t *ecb,
                  xdemitconf_t const *xecfg);

#endif /* #if !defined(XDIFFI_H) */
