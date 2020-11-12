// SPDX-License-Identifier: GPLv3-or-later
/*
 * time.h - time utilities
 * Copyright Peter Jones <pjones@redhat.com>
 */

#ifndef TIME_H_
#define TIME_H_

#include <errno.h>
#include <stdbool.h>
#include <sys/time.h>
#include <time.h>

#include "compiler.h"
#include "math.h"

#define NSEC_PER_SEC 1000000000ll
#define NSEC_PER_USEC 1000ll
#define USEC_PER_SEC 1000000ll

static inline UNUSED bool
normalize_timespec(const struct timespec *const src, struct timespec *const dst)
{
	if (!src) {
		errno = EINVAL;
		return true;
	}

	struct timespec result = { src->tv_sec, src->tv_nsec };

	while (result.tv_nsec >= NSEC_PER_SEC) {
		if (INCREMENT(result.tv_sec) ||
		    SUB(result.tv_nsec, NSEC_PER_SEC, &result.tv_nsec)) {
eoverflow:
			COLD;
			errno = EOVERFLOW;
			return true;
		}
	}

	while (result.tv_nsec <= -NSEC_PER_SEC) {
		if (DECREMENT(result.tv_sec) ||
		    SUB(result.tv_nsec, NSEC_PER_SEC, &result.tv_nsec))
			goto eoverflow;
	}

	while (result.tv_sec < 0 && result.tv_nsec > 0) {
		if (SUB(result.tv_nsec, NSEC_PER_SEC, &result.tv_nsec) ||
		    INCREMENT(result.tv_sec))
			goto eoverflow;
	}

	while (result.tv_sec > 0 && result.tv_nsec < 0) {
		if (ADD(result.tv_nsec, NSEC_PER_SEC, &result.tv_nsec) ||
		    DECREMENT(result.tv_sec))
			goto eoverflow;
	}

	if (dst) {
		dst->tv_sec = result.tv_sec;
		dst->tv_nsec = result.tv_nsec;
	}
	return false;
}

static inline UNUSED bool
normalize_timeval(const struct timeval *const src, struct timeval *const dst)
{
	if (!src) {
		errno = EINVAL;
		return true;
	}

	struct timeval result = { src->tv_sec, src->tv_usec };

	while (result.tv_usec > USEC_PER_SEC) {
		if (INCREMENT(result.tv_sec) ||
		    SUB(result.tv_usec, USEC_PER_SEC, &result.tv_usec)) {
eoverflow:
			COLD;
			errno = EOVERFLOW;
			return true;
		}
	}

	while (result.tv_usec < -USEC_PER_SEC) {
		if (DECREMENT(result.tv_sec) ||
		    SUB(result.tv_usec, USEC_PER_SEC, &result.tv_usec))
			goto eoverflow;
	}

	while (result.tv_sec < 0 && result.tv_usec > 0) {
		if (SUB(result.tv_usec, USEC_PER_SEC, &result.tv_usec) ||
		    INCREMENT(result.tv_sec))
			goto eoverflow;
	}

	while (result.tv_sec > 0 && result.tv_usec < 0) {
		if (ADD(result.tv_usec, USEC_PER_SEC, &result.tv_usec) ||
		    DECREMENT(result.tv_sec))
			goto eoverflow;
	}

	if (dst) {
		dst->tv_sec = result.tv_sec;
		dst->tv_usec = result.tv_usec;
	}
	return false;
}

#ifndef tvneg
#define tvneg(tv)                                                \
	({                                                       \
		struct timeval tv_;                              \
		normalize_timeval(&(tv), &tv_);                  \
	(tv_.tv_sec < 0 || (tv_.tv_sec == 0 && tv_.tv_usec < 0); \
	})
#endif
#ifndef tsneg
#define tsneg(ts)                                                \
	({                                                       \
		struct timespec ts_;                             \
		normalize_timespec(&(ts), &ts_);                 \
	(ts_.tv_sec < 0 || (ts_.tv_sec == 0 && ts_.tv_nsec < 0); \
	})
#endif
#ifndef tvzero
#define tvzero(tv)                                       \
	({                                               \
		struct timeval tv_;                      \
		normalize_timeval(&(tv), &tv_);          \
		((tv).tv_sec == 0 && (tv).tv_usec == 0); \
	})
#endif
#ifndef tszero
#define tszero(ts)                                     \
	({                                             \
		struct timespec ts_;                   \
		normalize_timespec(&(ts), &ts_);       \
		(ts_.tv_sec == 0 && ts_.tv_nsec == 0); \
	})
#endif
#ifndef tvpos
#define tvpos(tv)                                                         \
	({                                                                \
		struct timeval tv_;                                       \
		normalize_timeval(&(tv), &tv_);                           \
		(tv_.tv_sec > 0 || (tv_.tv_sec == 0 && tv_.tv_usec > 0)); \
	})
#endif
#ifndef tspos
#define tspos(ts)                                                         \
	({                                                                \
		struct timespec ts_;                                      \
		normalize_timespec(&(ts), &ts_);                          \
		(ts_.tv_sec > 0 || (ts_.tv_sec == 0 && ts_.tv_nsec > 0)); \
	})
#endif

static inline bool UNUSED
timespec_from_timeval(struct timespec *const dst,
                      const struct timeval *const src)
{
	if (!src) {
		errno = EINVAL;
		return true;
	}

	struct timeval tmp;
	if (normalize_timeval(src, &tmp) ||
	    MUL(tmp.tv_usec, NSEC_PER_USEC, &tmp.tv_usec) ||
	    normalize_timespec((struct timespec *)&tmp, dst)) {
		errno = EOVERFLOW;
		return true;
	}

	return false;
}

static inline UNUSED bool
timeval_from_timespec(struct timeval *const dst,
                      const struct timespec *const src)
{
	if (!src) {
		errno = EINVAL;
		return true;
	}

	struct timespec tmp;

	if (normalize_timespec(src, &tmp) ||
	    DIV(tmp.tv_nsec, NSEC_PER_USEC, &tmp.tv_nsec) ||
	    normalize_timeval((struct timeval *)&tmp, dst)) {
		errno = EOVERFLOW;
		return true;
	}

	return false;
}

#define timespec_to_timeval(a, b) timeval_from_timespec((b), (a))
#define timeval_to_timespec(a, b) timespec_from_timeval((b), (a))

static inline UNUSED bool
tsset(int64_t val, struct timespec *const result)
{
	if (DIVMOD(val, NSEC_PER_SEC, &result->tv_sec, &result->tv_nsec) ||
	    normalize_timespec(result, result))
		return true;
	return false;
}

static inline UNUSED bool
tsadd(const struct timespec *const addend0,
      const struct timespec *const addend1, struct timespec *sum)
{
	if (!addend0 || !addend1) {
		errno = EINVAL;
		return true;
	}

	struct timespec addend0p;
	struct timespec addend1p;
	struct timespec sump = {
		0,
	};

	if (normalize_timespec(addend0, &addend0p) ||
	    normalize_timespec(addend1, &addend1p) ||
	    ADD(addend0p.tv_sec, addend1p.tv_sec, &sump.tv_sec) ||
	    ADD(addend0p.tv_nsec, addend1p.tv_nsec, &sump.tv_nsec) ||
	    normalize_timespec(&sump, sum)) {
		errno = EOVERFLOW;
		return true;
	}

	return false;
}

static inline UNUSED bool
tssub(const struct timespec *const minuend,
      const struct timespec *const subtrahend, struct timespec *difference)
{
	if (!minuend || !subtrahend) {
		errno = EINVAL;
		return true;
	}

	struct timespec minuendp;
	struct timespec subtrahendp;
	struct timespec differencep = {
		0,
	};

	if (normalize_timespec(minuend, &minuendp) ||
	    normalize_timespec(subtrahend, &subtrahendp) ||
	    SUB(minuendp.tv_sec, subtrahendp.tv_sec, &differencep.tv_sec) ||
	    SUB(minuendp.tv_nsec, subtrahendp.tv_nsec, &differencep.tv_nsec) ||
	    normalize_timespec(&differencep, difference)) {
		errno = EOVERFLOW;
		return true;
	}

	return false;
}

static inline UNUSED bool
tsmul(const struct timespec *const factora,
      const struct timespec *const factorb, struct timespec *product)
{
	if (!factora || !factorb) {
		errno = EINVAL;
		return true;
	}

	struct timespec factorap;
	struct timespec factorbp;
	struct timespec productp = {
		0,
	};

	if (normalize_timespec(factora, &factorap) ||
	    normalize_timespec(factorb, &factorbp) ||
	    MUL(factorap.tv_sec, factorbp.tv_sec, &productp.tv_sec) ||
	    MUL(factorap.tv_nsec, factorbp.tv_nsec, &productp.tv_nsec) ||
	    normalize_timespec(&productp, product)) {
		errno = EOVERFLOW;
		return true;
	}

	return false;
}

static inline UNUSED bool
tsdivmod(const struct timespec *const dividend,
         const struct timespec *const divisor, struct timespec *quotient,
         struct timespec *remainder)
{
	if (!dividend || !divisor) {
		errno = EINVAL;
		return true;
	}

	struct timespec num, den;
	struct timespec divp, modp;
	struct timespec tmp;

	/*
	 * nominally we're looking for:
	 *
	 * dividendns = dividend.tv_sec * NSEC_PER_SEC + dividend.tv_nsec;
	 * divisorns = divisor.tv_sec * NSEC_PER_SEC + divisor.tv_nsec;
	 * divns = dividendns / divisorns;
	 * modns = dividendns % divisorns;
	 *
	 * and then normalize those.
	 */
	int64_t dividendns, divisorns, quotientns, remainderns;
	if (normalize_timespec(dividend, &num) ||
	    normalize_timespec(divisor, &den) ||
	    MUL(num.tv_sec, NSEC_PER_SEC, &dividendns) ||
	    ADD(dividendns, num.tv_nsec, &dividendns) ||
	    MUL(den.tv_sec, NSEC_PER_SEC, &divisorns) ||
	    ADD(divisorns, den.tv_nsec, &divisorns) ||
	    DIVMOD(dividendns, divisorns, &quotientns, &remainderns) ||
	    DIVMOD(quotientns, NSEC_PER_SEC, &divp.tv_sec, &divp.tv_nsec) ||
	    DIVMOD(remainderns, NSEC_PER_SEC, &modp.tv_sec, &modp.tv_nsec) ||
	    normalize_timespec(&divp, quotient) ||
	    normalize_timespec(&modp, remainder)) {
		errno = EOVERFLOW;
		return true;
	}
	return false;
}

static inline UNUSED bool
tscmp(const struct timespec *const a, const struct timespec *const b,
      cmp_result_t *const result)
{
	struct timespec difference;

	if (tssub(a, b, &difference))
		return true;

	if (tspos(difference))
		*result = lt;
	else if (tszero(difference))
		*result = eq;
	else
		*result = gt;

	return false;
}

static inline UNUSED bool
tsabs(const struct timespec *const val, struct timespec *const absval)
{
	struct timespec tmp;
	if (normalize_timespec(val, &tmp) || ABS(tmp.tv_sec, &tmp.tv_sec) ||
	    ASSIGN(tmp.tv_nsec, &tmp.tv_nsec))
		return true;

	memcpy(absval, &tmp, sizeof(tmp));
	return false;
}

extern const algebra_t tsalg;

static inline UNUSED bool
tsgcd(const struct timespec *const divisor0,
      const struct timespec *const divisor1, struct timespec *gcd)
{
	struct timespec mod;
	if (tsdivmod(divisor0, divisor1, NULL, &mod)) {
		warn("could not divide time");
		return true;
	}

	return false;
}

static inline UNUSED bool
tslcm(const struct timespec *const factor0,
      const struct timespec *const factor1, struct timespec *result)
{
	return lcm(&tsalg, factor0, factor1, result);
}

static inline UNUSED bool
tvset(int64_t val, struct timeval *const result)
{
	if (DIVMOD(val, USEC_PER_SEC, &result->tv_sec, &result->tv_usec) ||
	    normalize_timeval(result, result))
		return true;
	return false;
}

static inline UNUSED bool
tvadd(const struct timeval *const addend0, const struct timeval *const addend1,
      struct timeval *sum)
{
	if (!addend0 || !addend1 || !sum) {
		errno = EINVAL;
		return true;
	}

	struct timeval addend0p;
	struct timeval addend1p;
	struct timeval sump = {
		0,
	};

	if (normalize_timeval(addend0, &addend0p) ||
	    normalize_timeval(addend1, &addend1p) ||
	    ADD(addend0p.tv_sec, addend1p.tv_sec, &sump.tv_sec) ||
	    ADD(addend0p.tv_usec, addend1p.tv_usec, &sump.tv_usec) ||
	    normalize_timeval(&sump, sum)) {
		errno = EOVERFLOW;
		return true;
	}

	return false;
}

static inline UNUSED bool
tvsub(const struct timeval *const minuend,
      const struct timeval *const subtrahend, struct timeval *difference)
{
	if (!minuend || !subtrahend || !difference) {
		errno = EINVAL;
		return true;
	}

	struct timeval minuendp;
	struct timeval subtrahendp;
	struct timeval differencep = {
		0,
	};

	if (normalize_timeval(minuend, &minuendp) ||
	    normalize_timeval(subtrahend, &subtrahendp) ||
	    SUB(minuendp.tv_sec, subtrahendp.tv_sec, &differencep.tv_sec) ||
	    SUB(minuendp.tv_usec, subtrahendp.tv_usec, &differencep.tv_usec) ||
	    normalize_timeval(&differencep, difference)) {
		errno = EOVERFLOW;
		return true;
	}

	return false;
}

static inline UNUSED bool
tvmul(const struct timeval *const factora, const struct timeval *const factorb,
      struct timeval *product)
{
	if (!factora || !factorb || !product) {
		errno = EINVAL;
		return true;
	}

	struct timeval factorap;
	struct timeval factorbp;
	struct timeval productp = {
		0,
	};

	if (normalize_timeval(factora, &factorap) ||
	    normalize_timeval(factorb, &factorbp) ||
	    MUL(factorap.tv_sec, factorbp.tv_sec, &productp.tv_sec) ||
	    MUL(factorap.tv_usec, factorbp.tv_usec, &productp.tv_usec) ||
	    normalize_timeval(&productp, product)) {
		errno = EOVERFLOW;
		return true;
	}

	return false;
}

static inline UNUSED bool
tvdivmod(const struct timeval *const dividend,
         const struct timeval *const divisor, struct timeval *quotient,
         struct timeval *remainder)
{
	if (!dividend || !divisor) {
		errno = EINVAL;
		return true;
	}

	struct timeval num, den;
	struct timeval divp, modp;
	struct timeval tmp;

	/*
	 * nominally we're looking for:
	 *
	 * dividendus = dividend.tv_sec * USEC_PER_SEC + dividend.tv_usec;
	 * divisorus = divisor.tv_sec * USEC_PER_SEC + divisor.tv_usec;
	 * divus = dividendus / divisorus;
	 * modus = dividendus % divisorus;
	 *
	 * and then normalize those.
	 */
	int64_t dividendus, divisorus, quotientus, remainderus;
	if (normalize_timeval(dividend, &num) ||
	    normalize_timeval(divisor, &den) ||
	    MUL(num.tv_sec, USEC_PER_SEC, &dividendus) ||
	    ADD(dividendus, num.tv_usec, &dividendus) ||
	    MUL(den.tv_sec, USEC_PER_SEC, &divisorus) ||
	    ADD(divisorus, den.tv_usec, &divisorus) ||
	    DIVMOD(dividendus, divisorus, &quotientus, &remainderus) ||
	    DIVMOD(quotientus, USEC_PER_SEC, &divp.tv_sec, &divp.tv_usec) ||
	    DIVMOD(remainderus, USEC_PER_SEC, &modp.tv_sec, &modp.tv_usec) ||
	    normalize_timeval(&divp, quotient) ||
	    normalize_timeval(&modp, remainder)) {
		errno = EOVERFLOW;
		return true;
	}
	return false;
}

static inline UNUSED bool
tvcmp(const struct timeval *const a, const struct timeval *const b,
      cmp_result_t *const result)
{
	struct timeval difference;

	if (tvsub(a, b, &difference))
		return true;

	if (tvpos(difference))
		*result = lt;
	else if (tvzero(difference))
		*result = eq;
	else
		*result = gt;

	return false;
}

static inline UNUSED bool
tvabs(const struct timeval *const val, struct timeval *const absval)
{
	struct timeval tmp;
	if (normalize_timeval(val, &tmp) || ABS(tmp.tv_sec, &tmp.tv_sec) ||
	    ASSIGN(tmp.tv_usec, &tmp.tv_usec))
		return true;

	memcpy(absval, &tmp, sizeof(tmp));
	return false;
}

extern const algebra_t tvalg;

static inline UNUSED bool
tvgcd(const struct timeval *const divisor0,
      const struct timeval *const divisor1, struct timeval *gcd)
{
	struct timeval mod;
	if (tvdivmod(divisor0, divisor1, NULL, &mod)) {
		warn("could not divide time");
		return true;
	}

	return false;
}

static inline UNUSED bool
tvlcm(const struct timeval *const factor0, const struct timeval *const factor1,
      struct timeval *result)
{
	return lcm(&tvalg, factor0, factor1, result);
}

static inline UNUSED bool
gettimespecofday(struct timespec *const ts)
{
	int rc;
	bool of;

	rc = gettimeofday(((struct timeval *)ts), NULL);
	if (rc < 0)
		return rc;

	of = timespec_from_timeval(ts, (struct timeval *)ts);
	if (of) {
		errno = EOVERFLOW;
		rc = -1;
	}
	return rc;
}

#endif /* !TIME_H_ */
// vim:fenc=utf-8:tw=75:noet
