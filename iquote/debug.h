// SPDX-License-Identifier: GPLv3-or-later
/*
 * debug.h
 * Copyright Peter Jones <pjones@redhat.com>
 */

#ifndef UNC_COMMON_DEBUG_H_
#define UNC_COMMON_DEBUG_H_

#include <err.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "compiler.h"

extern bool unc_debug_arg_, unc_debug_once_;
extern int unc_debug_pfx_len_;

#ifdef UNC_COMMON_NO_DEBUG
#define dprintc(x)
#define dprints(x, y)
#define dump_maps()
#define unc_debug_arg(...) 0
#define debugnonl_(f_, l_, fmt_...)
#define debug_(f_, l_, fmt_, args_...)
#define debugnonl(fmt, args...)
#define debug(fmt, args...)
#define dassert(x)
#define dassertp(p, x)
#else
ssize_t vdebug_(const char *file, const int line, const char *func,
                const char *fmt, va_list ap);
ssize_t dprintc(int c);
ssize_t dprints(const char *const s, ssize_t sz);
extern void dump_maps(void);

#define DBG_FLF_FMT "%s:%d:%s(): "
#define DBG_NFLF_FMT "%s:%s:%d:%s(): "

/*
 * XXX move this to unc_get_debug()
 */
#define unc_debug_arg(criterion)                                \
	({                                                      \
		if (unc_debug_once_) {                          \
			if (unc_get_debug(criterion) ||         \
			    getenv("UNC_COMMON_DEBUG")) {       \
				unc_set_debug(criterion, true); \
				fflush(stdout);                 \
				setlinebuf(stdout);             \
				setlinebuf(stderr);             \
			}                                       \
			unc_debug_once_ = false;                \
		}                                               \
		unc_get_debug(criterion);                       \
	})

#define dprintf(fmt, ...)                                          \
	({                                                         \
		ssize_t rc_ = 0;                                   \
		if (unc_debug_arg(NULL))                           \
			rc_ = fprintf(stderr, fmt, ##__VA_ARGS__); \
		rc_;                                               \
	})

#define dprintflf_(F_, L_, f_)                                  \
	({                                                      \
		ssize_t rc_ = 0;                                \
		if (unc_debug_arg(NULL))                        \
			rc_ = dprintf(DBG_FLF_FMT, F_, L_, f_); \
		rc_;                                            \
	})
#define dprintflf() dprintflf_(__FILE__, __LINE__, __func__)

#define dprintnflf_(F_, L_, f_) \
	({ dprintf(DBG_NFLF_FMT, program_invocation_short_name, F_, L_, f_); })
#define dprintnflf() dprintflf_(__FILE__, __LINE__, __func__)

#define debug_(F_, L_, f_, fmt_, args_...)                        \
	({                                                        \
		ssize_t ret_ = 0;                                 \
		if (unc_get_debug(NULL)) {                        \
			size_t len_ = strlen(fmt_);               \
			dprintflf_(F_, L_, f_);                   \
			ret_ = dprintf(fmt_, ##args_);            \
			if (len_ > 0 && fmt_[len_ - 1] != '\n') { \
				dprintc('\n');                    \
				if (ret_ >= 0)                    \
					ret_ += 1;                \
			}                                         \
		}                                                 \
		ret_;                                             \
	})

extern bool unc_get_debug(const char *criteria);
extern void unc_set_debug(const char *criteria, bool enable);

#define debug(fmt, args...) debug_(__FILE__, __LINE__, __func__, fmt, ##args)

#define dassert(x)                                                \
	({                                                        \
		if (!(x))                                         \
			debug("Assertion failed: %s", STRING(x)); \
	})
#define dassertp(p, x)                                                   \
	({                                                               \
		if (!(x))                                                \
			debug("Assertion failed: %s (%s=%p)", STRING(x), \
			      STRING(p), (void *)(p));                   \
	})

#define errx_(status, fmt, file, line, func, ...)                            \
	({                                                                   \
		if (unc_get_debug(NULL)) {                                   \
			fprintf(stderr, "%s:%d:%s(): " fmt "\n", file, line, \
			        func, ##__VA_ARGS__);                        \
			exit(status);                                        \
		} else {                                                     \
			errx(status, fmt, ##__VA_ARGS__);                    \
		}                                                            \
	})
#define errx(status, fmt, ...) errx_(status, fmt, __FILE__, __LINE__, __func__, ##__VA_ARGS__)

#define err_(status, fmt, file, line, func, ...)                           \
	({                                                                 \
		if (unc_get_debug(NULL)) {                                 \
			fprintf(stderr, "%s:%d:%s(): " fmt ": %m\n", file, \
			        line, func, ##__VA_ARGS__);                \
			exit(status);                                      \
		} else {                                                   \
			err(status, fmt, ##__VA_ARGS__);                   \
		}                                                          \
	})
#define err(status, fmt, ...) err_(status, fmt, __FILE__, __LINE__, __func__, ##__VA_ARGS__)

#define warnx_(fmt, file, line, func, ...)                                   \
	({                                                                   \
		if (unc_get_debug(NULL)) {                                   \
			fprintf(stderr, "%s:%d:%s(): " fmt "\n", file, line, \
			        func, ##__VA_ARGS__);                        \
		} else {                                                     \
			warnx(fmt, ##__VA_ARGS__);                           \
		}                                                            \
	})
#define warnx(fmt, ...) warnx_(fmt, __FILE__, __LINE__, __func__, ##__VA_ARGS__)

#define warn_(fmt, file, line, func, ...)                                  \
	({                                                                 \
		if (unc_get_debug(NULL)) {                                 \
			fprintf(stderr, "%s:%d:%s(): " fmt ": %m\n", file, \
			        line, func, ##__VA_ARGS__);                \
		} else {                                                   \
			warnx(fmt, ##__VA_ARGS__);                         \
		}                                                          \
	})
#define warn(fmt, ...) warn_(fmt, __FILE__, __LINE__, __func__, ##__VA_ARGS__)

static inline UNUSED void
verrx_(int status, const char *fmt, const char *file, const int line,
       const char *func, va_list args)
{
	if (unc_get_debug(NULL)) {
		fprintf(stderr, "%s:%d:%s(): ", file, line, func);
		vfprintf(stderr, fmt, args);
		fprintf(stderr, "\n");
		exit(status);
	} else {
		verrx(status, fmt, args);
	}
}
#define verrx(status, fmt, args) verrx_(status, fmt, __FILE__, __LINE__, __func__, args)

static inline UNUSED void
verr_(int status, const char *fmt, const char *file, const int line,
      const char *func, va_list args)
{
	if (unc_get_debug(NULL)) {
		fprintf(stderr, "%s:%d:%s(): ", file, line, func);
		vfprintf(stderr, fmt, args);
		fprintf(stderr, ": %m\n");
		exit(status);
	} else {
		verr(status, fmt, args);
	}
}
#define verr(status, fmt, args) verr_(status, fmt, __FILE__, __LINE__, __func__, args)

static inline void UNUSED
vwarnx_(const char *fmt, const char *file, const int line, const char *func,
        va_list args)
{
	if (unc_get_debug(NULL)) {
		fprintf(stderr, "%s:%d:%s(): ", file, line, func);
		vfprintf(stderr, fmt, args);
		fprintf(stderr, "\n");
	} else {
		vwarnx(fmt, args);
	}
}
#define vwarnx(status, fmt, args) vwarnx_(status, fmt, __FILE__, __LINE__, __func__, args)

static inline UNUSED void
vwarn_(const char *fmt, const char *file, const int line, const char *func,
       va_list args)
{
	if (unc_get_debug(NULL)) {
		fprintf(stderr, "%s:%d:%s(): ", file, line, func);
		vfprintf(stderr, fmt, args);
		fprintf(stderr, ": %m\n");
	} else {
		vwarn(fmt, args);
	}
}
#define vwarn(status, fmt, args) vwarn_(status, fmt, __FILE__, __LINE__, __func__, args)

#endif /* !UNC_COMMON_NO_DEBUG */
#endif /* !UNC_COMMON_DEBUG_H_ */
// vim:fenc=utf-8:tw=75:noet
