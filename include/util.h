#pragma once

#ifdef __GNUC__
# define ALWAYS_INLINE inline __attribute__((always_inline))
#else
# define ALWAYS_INLINE inline
#endif
#define UNREACHABLE assert(!"unreachable")

// Returns -1 if lower than 0, otherwise 1.
static inline int signum_or_one(int x) {
	return x < 0 ? -1 : 1;
}

// Round up to +inf
//
// This is faster than std::round, which accounts for sign and
// sets errno (without -ffast-math et al.).
static inline double round_up(double x) {
	return (long long)(x + 0.5);
}
