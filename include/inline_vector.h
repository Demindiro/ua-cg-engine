#pragma once

#include <algorithm>
#include <cstdlib>
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

	const T *_get(size_t i) const {
	}

	T *_get(size_t i) {
		if (capacity() <= inline_capacity) {
			return &_inline_elem[i];
		} else {
			return &_heap_elem[i];
		}
	}

public:
	InlineVector<T, inline_capacity>() : _size(0), _capacity(inline_capacity) {}

	InlineVector<T, inline_capacity>(std::initializer_list<T> l)
		: _size(0), _capacity(inline_capacity)
	{
		if (l.size() >= capacity()) {
			grow(l.size());
		}
		for (auto &&e : l) {
			push_back(l);
		}
	}

	void clear() {
		if (capacity() <= 0) {
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
		return _capacity;
	}

	size_t size() const {
		return _size;
	}

	bool empty() const {
		return size() == 0;
	}

	const T &operator [](size_t i) const {
		if (capacity() <= inline_capacity) {
			return _inline_elem[i];
		} else {
			return _heap_elem[i];
		}
	}

	T &operator [](size_t i) {
		if (capacity() <= inline_capacity) {
			return _inline_elem[i];
		} else {
			return _heap_elem[i];
		}
	}

	const T &at(size_t i) const {
		if (i >= size()) {
			throw std::out_of_range("index out of range");
		}
		return (*this)[i];
	}

	T &at(size_t i) {
		if (i >= size()) {
			throw std::out_of_range("index out of range");
		}
		return (*this)[i];
	}

	void reserve(size_t new_cap) {
		if (new_cap <= inline_capacity && capacity() <= inline_capacity) {
			// Nothing to do
		} else if (new_cap <= inline_capacity && size() <= inline_capacity) {
			// Copy back to inline storage
			T *ptr = _heap_elem;
			for (size_t i = 0; i < size(); i++) {
				_inline_elem[i] = ptr[i];
			}
			free(ptr);
			_capacity = inline_capacity;
		} else {
			// Prevent currently allocated elements from being destroyed.
			new_cap = std::max(new_cap, size());
			// There is no realloc() equivalent for new/delete *shrug*
			// It should also prevent redundant/unwanted constructor calls, so
			// there's that.
			T *curr = capacity() <= inline_capacity ? nullptr : _heap_elem;
			curr = (T*)realloc(curr, new_cap * sizeof(T));
			if (curr == nullptr) {
				throw std::bad_alloc();
			}
			if (capacity() < inline_capacity) {
				// We need to do a copy ourselves.
				for (size_t i = 0; i < size(); i++) {
					curr[i] = _heap_elem[i];
				}
			}
			_heap_elem = curr;
			_capacity = new_cap;
		}
	}

	void push_back(const T &elem) {
		if (size() == capacity()) {
			reserve(capacity() * 2);
		}
		(*this)[_size++] = elem;
	}

	void push_back(T &&elem) {
		if (size() == capacity()) {
			reserve(capacity() * 2);
		}
		(*this)[_size++] = elem;
	}

	void emplace_back(T &&elem) {
		if (size() == capacity()) {
			reserve(capacity() * 2);
		}
		(*this)[_size++] = elem;
	}
};

}
}
