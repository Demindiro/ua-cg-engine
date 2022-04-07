#pragma once

#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <initializer_list>

namespace engine {
namespace util {

/**
 * \brief A vector with limited inline capacity.
 *
 * This is useful to avoid heap allocations for small vectors.
 */
template<typename T, size_t inline_capacity>
class InlineVector {
	static_assert(inline_capacity > 0);

	size_t _size, _capacity;
	union {
		T _inline_elem[inline_capacity];
		T *_heap_elem;
	};

	bool is_inline() const {
		return _capacity <= inline_capacity;
	}

public:
	InlineVector<T, inline_capacity>() : _size(0), _capacity(inline_capacity) {}

	InlineVector<T, inline_capacity>(std::initializer_list<T> l)
		: _size(0), _capacity(inline_capacity)
	{
		if (l.size() > capacity()) {
			grow(l.size());
		}
		for (auto &&e : l) {
			push_back(l);
		}
	}

	InlineVector<T, inline_capacity>(InlineVector<T, inline_capacity> &&v) {
		auto s = std::max(sizeof(T) * inline_capacity, sizeof(T *));
		std::memcpy(_inline_elem, v._inline_elem, s);
		_capacity =  v.capacity();
		_size = v.size();
		v._capacity = inline_capacity;
		v._size = 0;
	}

	InlineVector<T, inline_capacity>(const InlineVector<T, inline_capacity> &v)
		: _size(0), _capacity(inline_capacity)
	{
		reserve(v.size());
		for (size_t i = 0; i < v.size(); i++) {
			push_back(v[i]);
		}
	}

	void clear() {
		if (capacity() <= inline_capacity) {
			// Decrement size while iterating to ensure no double destructor
			// calls would occur in case one occurs now. (It shouldn't happen
			// but who knows).
			while (_size > 0) {
				_inline_elem[--_size].~T();
			}
		} else {
			// Ditto
			while (_size > 0) {
				_heap_elem[--_size].~T();
			}
		}
	}

	~InlineVector<T, inline_capacity>() {
		clear();
		if (capacity() > inline_capacity) {
			free(_heap_elem);
		}
	}

	size_t capacity() const {
		assert(_capacity >= inline_capacity);
		return _capacity;
	}

	size_t size() const {
		return _size;
	}

	bool empty() const {
		return size() == 0;
	}

	const T &operator [](size_t i) const {
		assert(size() <= capacity());
		assert(i < size());
		if (capacity() <= inline_capacity) {
			return _inline_elem[i];
		} else {
			return _heap_elem[i];
		}
	}

	T &operator [](size_t i) {
		assert(size() <= capacity());
		assert(i < size());
		if (capacity() <= inline_capacity) {
			return _inline_elem[i];
		} else {
			return _heap_elem[i];
		}
	}

	const T &at(size_t i) const {
		assert(size() <= capacity());
		if (i >= size()) {
			throw std::out_of_range("index out of range");
		}
		return (*this)[i];
	}

	T &at(size_t i) {
		assert(size() <= capacity());
		if (i >= size()) {
			throw std::out_of_range("index out of range");
		}
		return (*this)[i];
	}

	void reserve(size_t new_cap) {
		assert(size() <= capacity());
		if (new_cap <= capacity()) {
			return;
		} else {
			// There is no realloc() equivalent for new/delete *shrug*
			// It should also prevent redundant/unwanted constructor calls, so
			// there's that.
			T *curr = capacity() <= inline_capacity ? nullptr : _heap_elem;
			curr = (T*)realloc(curr, new_cap * sizeof(T));
			if (curr == nullptr) {
				throw std::bad_alloc();
			}
			if (new_cap > capacity() && is_inline()) {
				// We need to do a copy ourselves.
				for (size_t i = 0; i < size(); i++) {
					curr[i] = _inline_elem[i];
				}
			}
			_heap_elem = curr;
			_capacity = new_cap;
		}
		assert(size() <= capacity());
	}

	void push_back(const T &elem) {
		assert(size() <= capacity());
		if (size() == capacity()) {
			reserve(capacity() * 2);
		}
		(*this)[_size++] = elem;
		assert(size() <= capacity());
	}

	void push_back(T &&elem) {
		assert(size() <= capacity());
		if (size() == capacity()) {
			reserve(capacity() * 2);
		}
		(*this)[_size++] = elem;
		assert(size() <= capacity());
	}

	void emplace_back(T &&elem) {
		assert(size() <= capacity());
		if (size() == capacity()) {
			reserve(capacity() * 2);
		}
		(*this)[_size++] = elem;
		assert(size() <= capacity());
	}
};

}
}
