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

/**
 * \brief Range that wraps two equal-sized ranges.
 *
 * Useful with std::sort.
 *
 * Based on https://artificial-mind.net/blog/2020/11/28/std-sort-multiple-ranges and
 * https://www.internalpointers.com/post/writing-custom-iterators-modern-cpp.
 *
 * Implements requirements (from cppreference):
 * - LegacyRandomAccessIterator (r += n, a + n, n + a, r -= n, i - n, b - a, i[n], a < b, a > b, a >= b, a <= b)
 * - LegacyBidirectionalIterator (--a, a--, *a--)
 * - LegacyForwardIterator (i++, *i++)
 * - LegacyIterator (++r, *r)
 */
template<typename L, typename R>
class zip {
public:
	class iterator {
		friend class zip;

		L l;
		R r;

		iterator(L l, R r) : l(l), r(r) {}

	public:
		using iterator_category = typename L::iterator_category;

		// FIXME difference_type may differ (hah) between L and R
		// Sadly glibc seems to require some integer for sort :(
		using difference_type = typename L::difference_type;

		struct value_type {
			typename L::value_type l;
			typename R::value_type r;

			friend void swap(value_type &lhs, value_type &rhs) {
				std::swap(lhs.l, rhs.l);
				std::swap(lhs.r, rhs.r);
			}
		};

		struct reference {
			typename L::value_type &l;
			typename R::value_type &r;
			
			reference &operator =(reference &&v) {
				l = std::move(v.l);
				r = std::move(v.r);
				return *this;
			}

			reference &operator =(value_type &&v) {
				l = std::move(v.l);
				r = std::move(v.r);
				return *this;
			}

			friend void swap(reference lhs, reference rhs) {
				std::swap(lhs.l, rhs.l);
				std::swap(lhs.r, rhs.r);
			}

			operator value_type() && {
				return { std::move(l), std::move(r) };
			}
		};

		using pointer = value_type *;

		iterator operator +(difference_type n) const {
			return { l + n, r + n };
		}

		iterator operator -(difference_type n) const {
			return { l - n, r - n };
		}

		difference_type operator -(const iterator &rhs) const {
			return l - rhs.l;
		}

		reference operator [](difference_type n) {
			return { l[n], r[n] };
		}

		iterator &operator ++() {
			++l, ++r;
			return *this;
		}

		iterator &operator --() {
			--l, --r;
			return *this;
		}
		
		reference operator *() {
			return { *l, *r };
		}

		bool operator ==(const iterator &with) const {
			return l == with.l;
		}

		bool operator !=(const iterator &with) const {
			return l != with.l;
		}

		bool operator <(const iterator &rhs) const {
			return l < rhs.l;
		}
	};

	zip(std::pair<L, R> begin, std::pair<L, R> end)
		: start(iterator { begin.first, begin.second })
		, stop(iterator { end.first, end.second })
	{}

	iterator begin() const {
		return start;
	}

	iterator end() const {
		return stop;
	}

private:
	iterator start, stop;
};
