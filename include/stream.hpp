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

	/**
	 * A Stream<T> is a lazy, stateful generator of values of type T.
	 * It should not be consumed concurrently.
	 *
	 * This is a virtual class demanding that implementations provide
	 * overloads for `operator*` to get the current value, `operator++`
	 * to (pre-)increment the stream to point to the next value, and
	 * `operator bool` to return if the stream is valid and can be
	 * dereferenced.
	 */
	template<typename T>
	class Stream {
	public:
		/** Return the current element. */
		virtual T operator*(void) const     = 0;
		/** Produce the next element. */
		virtual Stream<T>& operator++(void) = 0;
		/** Return if the stream points at a valid value. */
		virtual operator bool(void) const   = 0;

		class iterator;
		class Cached;

		virtual iterator begin(void) { return iterator(this); }
		virtual iterator end(void)   { return iterator();     }

		/** Return a cached version of this stream. */
		Cached cache(void) { return Cached(this); }
	};

	/**
	 * STL-compatible iterator for a Stream<T>. Instances of this are
	 * obtained from `Stream<T>::{begin,end}` enabling range-based
	 * for loops over streams.
	 */
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

		/**
		 * Comparison of Stream<T>::iterators are only defined when
		 * the RHS of the comparison is the special iterator constructed
		 * from a `nullptr`, which stands for the `end` iterator of a
		 * Stream<T> range.
		 */
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

	/**
	 * A caching version of Stream<T>.
	 *
	 * This is the return value of Stream<T>::cache, an object implementing
	 * the Stream<T> protocol which can in addition be reset and iterated
	 * again. The price to pay for this is that a copy of the original
	 * stream must be stored inside the Cached object.
	 */
	template<typename T>
	class Stream<T>::Cached : public Stream<T> {
		std::vector<T> cache;
		unsigned int i = 0;

	public:
		/**
		 * Construct a Cached version of the given stream. The constructor
		 * already iterates the entire input stream and caches it.
		 *
		 * In the future, this object may instead store a copy of the input
		 * stream and gradually cache its output.
		 */
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

		/** Return the number of elements in the cache. */
		size_t size(void) const {
			return cache.size();
		}

		/** Reset the stream pointer to the beginning.  */
		Cached& reset(void) {
			i = 0;
			return *this;
		}

		/** Reset the stream and return an iterator to the beginning. */
		virtual iterator begin(void) {
			this->reset();
			return iterator(this);
		}
	};
}

#endif /* PROPCALC_STREAM_HPP */
