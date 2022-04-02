#pragma once

#include <cassert>
#include <memory>

/**
 * \brief A copy-on-write type.
 *
 * Multiple references may exist to the same objects, but on a write the object will be copied
 * if multiple references exist.
 */
template<typename T>
class Cow {
	std::shared_ptr<T> data;

	void make_unique() {
		assert(data.use_count() > 0 && "data got deleted somehow");
		if (data.use_count() > 1) {
			data = std::make_shared<T>(*data);
		}
	}

public:
	Cow<T>() : data(std::make_shared<T>(T())) {};
	Cow<T>(T &&data) : data(std::make_shared(data)) {};
	Cow<T>(const T &data) : data(std::make_shared(data)) {};
	Cow<T>(T data) : data(std::make_shared(data)) {};
	Cow<T>(Cow<T> &&cow) : data(cow.data) {};
	Cow<T>(const Cow<T> &cow) : data(cow.data) {};

	Cow<T> &operator =(Cow<T> &&cow) {
		this->data = cow.data;
		return *this;
	}

	Cow<T> &operator =(const Cow<T> &cow) {
		this->data = cow.data;
		return *this;
	}

	const T &operator *() const {
		return *data;
	}

	const T *operator ->() const {
		return &*data;
	}

	T &operator *() {
		make_unique();
		return *data;
	}

	T *operator ->() {
		make_unique();
		return &*data;
	}

	static Cow<T> make(T data) {
		Cow cow;
		cow.data = std::make_shared<T>(data);
		return cow;
	}
};
