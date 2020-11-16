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

#if !defined(XTYPES_H)
#define XTYPES_H

typedef struct s_chanode {
	struct s_chanode *next;
	size_t icurr;
} chanode_t;

typedef struct s_chastore {
	chanode_t *head, *tail;
	size_t isize, nsize;
	chanode_t *ancur;
	chanode_t *sncur;
	size_t scurr;
} chastore_t;

typedef struct s_xrecord {
	struct s_xrecord *next;
	const char *ptr;
	size_t size;
	unsigned long ha;
} xrecord_t;

typedef struct s_xdfile {
	chastore_t rcha;
	size_t nrec;
	size_t hbits;
	xrecord_t **rhash;
	size_t dstart, dend;
	xrecord_t **recs;
	char *rchg;
	size_t *rindex;
	size_t nreff;
	unsigned long *ha;
} xdfile_t;

typedef struct s_xdfenv {
	xdfile_t xdf1, xdf2;
} xdfenv_t;

#endif /* #if !defined(XTYPES_H) */
