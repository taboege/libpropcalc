/*
 * stream.hpp - Stream, lazy stateful generator of values
 *
 * Copyright (C) 2019-2020 Tobias Boege
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the Artistic License 2.0
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * Artistic License 2.0 for more details.
 */

#ifndef PROPCALC_STREAM_HPP
#define PROPCALC_STREAM_HPP

#include <stdexcept>
#include <vector>

namespace Propcalc {
	namespace X::Stream {
		/**
		 * Stream iterators do not support being compared to each other
		 * in general. The only valid comparison is iterator(Stream<T>*)
		 * vs. iterator(nullptr).
		 */
		struct Comparison : std::logic_error {
			Comparison(void) : std::logic_error("Invalid comparison of streams") { }
		 };
	}

	template<typename T>
	class Stream {
	public:
		virtual T operator*(void) const     = 0;
		virtual Stream<T>& operator++(void) = 0;
		virtual operator bool(void) const   = 0;

		class iterator;
		class Cached;

		virtual iterator begin(void) { return iterator(this); }
		virtual iterator end(void)   { return iterator();     }

		Cached cache(void) { return Cached(this); }
	};

	template<typename T>
	class Stream<T>::iterator {
		Stream<T>* st;

	public:
		using iterator_category = std::forward_iterator_tag;
		using value_type = T;
		using difference_type = std::ptrdiff_t;
		using pointer = T*;
		using reference = T&;

		iterator(Stream<T>* st) : st(st) { }
		iterator(void)     : st(nullptr) { }

		bool operator!=(iterator& b) const {
			if (!b.st)
				return !!*st;
			throw X::Stream::Comparison();
		}

		T operator*(void) const {
			return **st;
		}

		iterator& operator++(void) {
			++*st;
			return *this;
		}
	};

	template<typename T>
	class Stream<T>::Cached : public Stream<T> {
		std::vector<T> cache;
		unsigned int i = 0;

	public:
		Cached(Stream<T>* st) {
			for (auto c : *st)
				cache.push_back(c);
		}

		operator bool(void) const {
			return i < cache.size();
		}

		T operator*(void) const {
			return cache[i];
		}

		Cached& operator++(void) {
			++i;
			return *this;
		}

		size_t size(void) const {
			return cache.size();
		}

		Cached& reset(void) {
			i = 0;
			return *this;
		}

		virtual iterator begin(void) {
			this->reset();
			return iterator(this);
		}
	};
}

#endif /* PROPCALC_STREAM_HPP */
