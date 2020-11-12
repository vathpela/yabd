// SPDX-License-Identifier: GPLv3-or-later
/*
 * math.h - math helpers
 * Copyright Peter Jones <pjones@redhat.com>
 */

#ifndef LIBUNC_MATH_H_
#define LIBUNC_MATH_H_

/* These appear in gcc 5.1 and clang 3.8. */
#if GNUC_PREREQ(5, 1) || CLANG_PREREQ(3, 8)

#define ADD(a, b, res)                                                       \
	({                                                                   \
		__auto_type a_ = (a);                                        \
		(((res) == NULL) ? __builtin_add_overflow((a), (b), &a_)     \
		                 : __builtin_add_overflow((a), (b), (res))); \
	})
#define SUB(a, b, res)                                                       \
	({                                                                   \
		__auto_type a_ = (a);                                        \
		(((res) == NULL) ? __builtin_sub_overflow((a), (b), &a_)     \
		                 : __builtin_sub_overflow((a), (b), (res))); \
	})
#define MUL(a, b, res)                                                       \
	({                                                                   \
		__auto_type a_ = (a);                                        \
		(((res) == NULL) ? __builtin_mul_overflow((a), (b), &a_)     \
		                 : __builtin_mul_overflow((a), (b), (res))); \
	})
#define DIV(a, b, res)                                          \
	({                                                      \
		bool ret_ = true;                               \
		if ((b) != 0) {                                 \
			if (!is_null(res))                      \
				(*(res)) = (a) / (b);           \
			ret_ = false;                           \
		} else {                                        \
			errno = EOVERFLOW; /* no EDIVBYZERO? */ \
		}                                               \
		ret_;                                           \
	})
/*
 * These really just exists for chaining results easily with || in an expr
 */
#define MOD(a, b, res)                        \
	({                                    \
		if (!is_null(res))            \
			(*(res)) = (a) % (b); \
		false;                        \
	})
#define DIVMOD(a, b, resd, resm) \
	(DIV((a), (b), (resd)) || MOD((a), (b), (resm)))
#define ASSIGN(a, res) ADD((a), (typeof(a))0, (res))
#define ABS(val, absval)                                           \
	({                                                         \
		bool res_;                                         \
		if (val < 0)                                       \
			res_ = SUB((typeof(val))0, (val), absval); \
		else                                               \
			res_ = ASSIGN((val), (absval));            \
		res_;                                              \
	})

#define INCREMENT(a) ADD((a), 1, &(a))
#define DECREMENT(a) SUB((a), 1, &(a))

#define _unc_generic_sint_builtin(op, x)                 \
	_Generic((x), int                                \
	         : CAT(__builtin_, op)(x), long          \
	         : CAT3(__builtin_, op, l)(x), long long \
	         : CAT3(__builtin_, op, ll)(x))
#define FFS(x) _unc_generic_sint_builtin(ffs, x)
#define CLRSB(x) _unc_generic_sint_builtin(clrsb, x)

#define _unc_generic_uint_builtin(op, x)                          \
	_Generic((x), unsigned int                                \
	         : CAT(__builtin_, op)(x), unsigned long          \
	         : CAT3(__builtin_, op, l)(x), unsigned long long \
	         : CAT3(__builtin_, op, ll)(x))
#define CLZ(x) _unc_generic_uint_builtin(clz, x)
#define CTZ(x) _unc_generic_uint_builtin(ctz, x)
#define POPCOUNT(x) _unc_generic_uint_builtin(popcount, x)
#define PARITY(x) _unc_generic_uint_builtin(parity, x)

#else
#error gcc 5.1 or newer or clang 3.8 or newer is required
#endif

typedef enum
{
	lt = -1,
	eq = 0,
	gt = 1
} cmp_result_t;

typedef bool(alg_set_t)(const int64_t val, void *const result);
typedef bool(alg_abs_t)(const void *const val, void *const absval);
typedef bool(alg_add_t)(const void *const addend0, const void *const addend1,
                        void *result);
typedef bool(alg_sub_t)(const void *const minuend, const void *const subtrahend,
                        void *result);
typedef bool(alg_mul_t)(const void *const factor0, const void *const factor1,
                        void *product);
typedef bool(alg_divmod_t)(const void *const dividend,
                           const void *const divisor, void *quotient,
                           void *remainder);
typedef bool(alg_cmp_t)(const void *const a, const void *const b,
                        cmp_result_t *const result);

typedef struct algebra {
	size_t size;
	alg_set_t *set;
	alg_abs_t *abs;
	alg_add_t *add;
	alg_sub_t *sub;
	alg_mul_t *mul;
	alg_divmod_t *divmod;
	alg_cmp_t *cmp;
} algebra_t;

static inline UNUSED bool
alg_ready(const algebra_t *const alg)
{
	return alg && alg->size && alg->set && alg->abs && alg->add &&
	       alg->sub && alg->mul && alg->divmod && alg->cmp;
}

static inline UNUSED bool
gcd(const algebra_t *const alg, const void *const ap, const void *const bp,
    void *const result)
{
	static void *tmp = NULL, *zero = NULL;
	static size_t size = 0;
	bool clear = false;
	bool ret = true;

	if (!alg_ready(alg)) {
		errno = EINVAL;
		return true;
	}
	if (size != alg->size || tmp == NULL || zero == NULL) {
		size = alg->size;
		tmp = alloca(size);
		zero = alloca(size);
		if (tmp == NULL || zero == NULL || alg->set(0, &zero))
			return true;
		clear = true;
	}

	cmp_result_t res;

	if (alg->set(0, zero) || alg->cmp(bp, zero, &res)) {
out:
		if (clear) {
			size = 0;
			tmp = zero = NULL;
		}
		return ret;
	}

	if (res == eq) {
		ret = alg->abs(ap, result);
		goto out;
	}

	if (alg->divmod(ap, bp, NULL, tmp))
		goto out;

	return gcd(alg, bp, tmp, result);
}

static inline UNUSED bool
lcm(const algebra_t *const alg, const void *const ap, const void *const bp,
    void *const result)
{
	if (!alg_ready(alg)) {
		errno = EINVAL;
		return true;
	}

	void *dividend = alloca(alg->size);
	void *divisor = alloca(alg->size);
	if (!dividend || !divisor)
		return true;

	if (alg->mul(ap, bp, dividend) || alg->abs(dividend, dividend) ||
	    gcd(alg, ap, bp, divisor) ||
	    alg->divmod(dividend, divisor, result, NULL))
		return true;

	return false;
}
#endif /* !LIBUNC_MATH_H_ */
// vim:fenc=utf-8:tw=75:noet
