// SPDX-License-Identifier: GPLv3-or-later
/*
 * compiler.h - helpers for dumb compiler stuff
 * Copyright Peter Jones <pjones@redhat.com>
 */

#ifndef LIBUNC_COMPILER_H_
#define LIBUNC_COMPILER_H_

/* GCC version checking borrowed from glibc. */
#if defined(__GNUC__) && defined(__GNUC_MINOR__)
#define GNUC_PREREQ(maj, min) \
	((__GNUC__ << 16) + __GNUC_MINOR__ >= ((maj) << 16) + (min))
#else
#define GNUC_PREREQ(maj, min) 0
#endif

/* Does this compiler support compile-time error attributes? */
#if GNUC_PREREQ(4, 3)
#define ATTRIBUTE_ERROR(msg) __attribute__((__error__(msg)))
#else
#define ATTRIBUTE_ERROR(msg) __attribute__((noreturn))
#endif

#if GNUC_PREREQ(4, 4)
#define GNU_PRINTF gnu_printf
#else
#define GNU_PRINTF printf
#endif

#if GNUC_PREREQ(3, 4)
#define WARN_UNUSED_RESULT __attribute__((warn_unused_result))
#else
#define WARN_UNUSED_RESULT
#endif

#ifndef uint128_t
typedef unsigned __int128 uint128_t;
#define uint128_t uint128_t
#endif
#ifndef int128_t
typedef __int128 int128_t;
#define int128_t int128_t
#endif

#if defined(__clang__) && defined(__clang_major__) && defined(__clang_minor__)
#define CLANG_PREREQ(maj, min)        \
	((__clang_major__ > (maj)) || \
	 (__clang_major__ == (maj) && __clang_minor__ >= (min)))
#else
#define CLANG_PREREQ(maj, min) 0
#endif

#ifndef LIBUNC_CAT_
#define LIBUNC_CAT_(a, b) a##b
#endif
#ifndef LIBUNC_CAT
#define LIBUNC_CAT(a, b) LIBUNC_CAT_(a, b)
#endif
#ifndef LIBUNC_CAT3_
#define LIBUNC_CAT3_(a, b, c) a##b##c
#endif
#ifndef LIBUNC_CAT3
#define LIBUNC_CAT3(a, b, c) LIBUNC_CAT3_(a, b, c)
#endif

/* Indirect macros required for expanded argument pasting, eg. __LINE__. */
#define ___PASTE(a, b) a##b
#define __PASTE(a, b) ___PASTE(a, b)

#define __UNIQUE_ID(prefix) __PASTE(__PASTE(__UNIQUE_ID_, prefix), __COUNTER__)

#ifndef typeof
#define typeof(x) __typeof__(x)
#endif
#ifndef aligned_alloca
#define aligned_alloca(size, alignment)                                    \
	({                                                                 \
		void *ptr_ = __builtin_alloca_with_align(size, alignment); \
		if (!ptr_)                                                 \
			errno = ENOMEM;                                    \
		ptr_;                                                      \
	})
#endif
#ifndef aligned_alloca_max
#define aligned_alloca_max(size, alignment, max)                  \
	({                                                        \
		void *ptr_ = __builtin_alloca_with_align_and_max( \
			size, alignment, max);                    \
		if (!ptr_)                                        \
			errno = ENOMEM;                           \
		ptr_;                                             \
	})
#endif
#ifndef ALIAS
#define ALIAS(x) __attribute__((weak, alias(#x)))
#endif
#ifndef ALIGN
#define __ALIGN_MASK(x, mask) (((x) + (mask)) & ~(mask))
#define __ALIGN(x, a) __ALIGN_MASK(x, (typeof(x))(a)-1)
#define ALIGN(x, a) __ALIGN((x), (a))
#endif
#ifndef ALIGN_DOWN
#define ALIGN_DOWN(x, a) __ALIGN((x) - ((a)-1), (a))
#endif
#ifndef ALIGNMENT_PADDING
#define ALIGNMENT_PADDING(value, align) ((align - (value % align)) % align)
#endif
#ifndef ALIGN_UP
#define ALIGN_UP(value, align) ((value) + ALIGNMENT_PADDING(value, align))
#endif
#ifndef ALIGNED
#define ALIGNED(n) __attribute__((__aligned__(n)))
#endif
#ifndef CLEANUP_FUNC
#define CLEANUP_FUNC(x) __attribute__((__cleanup__(x)))
#endif
#ifndef COLD
#define COLD __attribute__((__cold__))
#endif
#ifndef CONSTRUCTOR
#define CONSTRUCTOR(priority) __attribute__((constructor(priority)))
#endif
#ifndef DESTRUCTOR
#define DESTRUCTOR(priority) __attribute__((destructor(priority)))
#endif
#ifndef FALLS_THROUGH
#define FALLS_THROUGH __attribute__((__fallthrough__))
#endif
#ifndef FLATTEN
#define FLATTEN __attribute__((__flatten__))
#endif
#ifndef HIDDEN
#define HIDDEN __attribute__((__visibility__("hidden")))
#endif
#ifndef HOT
#define HOT __attribute__((__hot__))
#endif
#ifndef likely
#define likely(x) __builtin_expect(!!(x), 1)
#endif
#ifndef NONNULL
#define NONNULL(first, args...) __attribute__((__nonnull__(first, ##args)))
#endif
#ifndef NORETURN
#define NORETURN __attribute__((__noreturn__))
#endif
#ifndef offsetof
#define offsetof(type, member) __builtin_offsetof(type, member)
#endif
#ifndef OPTIMIZE
#define OPTIMIZE(x) __attribute__((__optimize__(x)))
#endif
#ifndef PACKED
#define PACKED __attribute__((__packed__))
#endif
#ifndef PRINTF
#define PRINTF(first, args...) \
	__attribute__((__format__(printf, first, ##args)))
#endif
#ifndef PUBLIC
#define PUBLIC __attribute__((__visibility__("default")))
#endif
#ifndef READ_ONCE
#define READ_ONCE(var) (*((volatile typeof(var) *)(&(var))))
#endif
#ifndef SECTION
#define SECTION(x) __attribute__((__section__(x)))
#endif
#ifndef STRING
#define STRING(x) __STRING(x)
#endif
#ifndef TRAP
#define TRAP __builtin_trap()
#endif
#ifndef unlikely
#define unlikely(x) __builtin_expect(!!(x), 0)
#endif
#ifndef UNREACHABLE
#define UNREACHABLE __builtin_unreachable()
#endif
#ifndef UNUSED
#define UNUSED __attribute__((__unused__))
#endif
#ifndef USED
#define USED __attribute__((__used__))
#endif
#ifndef VERSION
#define VERSION(sym, ver) __asm__(".symver " #sym "," #ver)
#endif
#ifndef WARN_UNCHECKED
#define WARN_UNCHECKED __attribute__((__warn_unused_result__))
#endif
#ifndef WRITE_ONCE
#define WRITE_ONCE(var, val) (*((volatile typeof(val) *)(&(var))) = (val))
#endif

/*
 * min()/max()/clamp() macros must accomplish three things:
 *
 * - avoid multiple evaluations of the arguments (so side-effects like
 *   "x++" happen only once) when non-constant.
 * - perform strict type-checking (to generate warnings instead of
 *   nasty runtime surprises). See the "unnecessary" pointer comparison
 *   in __typecheck().
 * - retain result as a constant expressions when called with only
 *   constant expressions (to avoid tripping VLA warnings in stack
 *   allocation usage).
 */
#define __typecheck(x, y) (!!(sizeof((typeof(x) *)1 == (typeof(y) *)1)))

/*
 * This returns a constant expression while determining if an argument is
 * a constant expression, most importantly without evaluating the argument.
 * Glory to Martin Uecker <Martin.Uecker@med.uni-goettingen.de>
 */
#define __is_constexpr(x) \
	(sizeof(int) == sizeof(*(8 ? ((void *)((long)(x)*0l)) : (int *)8)))

#define __no_side_effects(x, y) (__is_constexpr(x) && __is_constexpr(y))

#define __safe_cmp(x, y) (__typecheck(x, y) && __no_side_effects(x, y))

#define __cmp(x, y, op) ((x)op(y) ? (x) : (y))

#define __cmp_once(x, y, unique_x, unique_y, op) \
	({                                       \
		typeof(x) unique_x = (x);        \
		typeof(y) unique_y = (y);        \
		__cmp(unique_x, unique_y, op);   \
	})

#define __careful_cmp(x, y, op)                                  \
	__builtin_choose_expr(__safe_cmp(x, y), __cmp(x, y, op), \
	                      __cmp_once(x, y, __UNIQUE_ID(__x), \
	                                 __UNIQUE_ID(__y), op))

/**
 * min - return minimum of two values of the same or compatible types
 * @x: first value
 * @y: second value
 */
#define min(x, y) __careful_cmp(x, y, <)

/**
 * max - return maximum of two values of the same or compatible types
 * @x: first value
 * @y: second value
 */
#define max(x, y) __careful_cmp(x, y, >)

#define is_null(x) __careful_cmp((void *)(x), NULL, ==)

/* Are two types/vars the same type (ignoring qualifiers)? */
#ifndef same_type
#define same_type(a, b) __builtin_types_compatible_p(typeof(a), typeof(b))
#endif
#ifndef choose
#define choose(cnd, a, b) __builtin_choose_expr(cnd, a, b)
#endif
#ifndef is_ptr
#define is_ptr(type, x) same_type(type, typeof((x)))
#endif
#ifndef is_unsigned
#define is_unsigned(a) \
	__builtin_types_compatible_p(unsigned typeof(a), typeof(a))
#endif
#ifndef is_signed
#define is_signed(a) (!is_unsigned(a))
#endif

/* Compile time object size, -1 for unknown */
#ifndef libunc_compiletime_object_size_
#define libunc_compiletime_object_size_(obj) -1
#endif
#ifndef libunc_compiletime_warning_
#define libunc_compiletime_warning_(message)
#endif
#ifndef libunc_compiletime_error_
#define libunc_compiletime_error_(message)
#endif

#ifndef libunc_compiletime_assert__
#define libunc_compiletime_assert__(condition, msg, prefix, suffix) \
	do {                                                        \
		extern void prefix##suffix(void)                    \
			libunc_compiletime_error_(msg);             \
		if (!(condition))                                   \
			prefix##suffix();                           \
	} while (0)
#endif

#ifndef libunc_compiletime_assert_
#define libunc_compiletime_assert_(condition, msg, prefix, suffix) \
	libunc_compiletime_assert_(condition, msg, prefix, suffix)
#endif

/**
 * compiletime_assert - break build and emit msg if condition is false
 * @condition: a compile-time constant condition to check
 * @msg:       a message to emit if condition is false
 *
 * In tradition of POSIX assert, this macro will break the build if the
 * supplied condition is *false*, emitting the supplied error message if the
 * compiler has support to do so.
 */
#ifndef libunc_compiletime_assert
#define libunc_compiletime_assert(condition, msg) libunc_compiletime_assert_(condition, msg, libunc_compiletime_assert__, __LINE__)
#endif

/**
 * BUILD_BUG_ON_MSG - break compile if a condition is true & emit supplied
 *		      error message.
 * @condition: the condition which the compiler should know is false.
 *
 * See BUILD_BUG_ON for description.
 */
#ifndef BUILD_BUG_ON_MSG
#define BUILD_BUG_ON_MSG(cond, msg) compiletime_assert(!(cond), msg)
#endif

#if __BYTE_ORDER == __LITTLE_ENDIAN
#define cpu_to_le16(x) (x)
#define cpu_to_le32(x) (x)
#define cpu_to_le64(x) (x)
#define le16_to_cpu(x) (x)
#define le32_to_cpu(x) (x)
#define le64_to_cpu(x) (x)
#define cpu_to_be16(x) __builtin_bswap16(x)
#define cpu_to_be32(x) __builtin_bswap32(x)
#define cpu_to_be64(x) __builtin_bswap64(x)
#define be16_to_cpu(x) __builtin_bswap16(x)
#define be32_to_cpu(x) __builtin_bswap32(x)
#define be64_to_cpu(x) __builtin_bswap64(x)
#else
#define cpu_to_be16(x) (x)
#define cpu_to_be32(x) (x)
#define cpu_to_be64(x) (x)
#define be16_to_cpu(x) (x)
#define be32_to_cpu(x) (x)
#define be64_to_cpu(x) (x)
#define cpu_to_le16(x) __builtin_bswap16(x)
#define cpu_to_le32(x) __builtin_bswap32(x)
#define cpu_to_le64(x) __builtin_bswap64(x)
#define le16_to_cpu(x) __builtin_bswap16(x)
#define le32_to_cpu(x) __builtin_bswap32(x)
#define le64_to_cpu(x) __builtin_bswap64(x)
#endif

#endif /* !LIBUNC_COMPILER_H_ */
// vim:fenc=utf-8:tw=75:noet
//
