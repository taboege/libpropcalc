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
	 * overloads for `operator++` to (pre-)increment the stream to point
	 * to the next value, use the protected `produce` to make available
	 * a new value, and `operator bool` to return if the stream is valid
	 * and can be dereferenced.
	 *
	 * The produced values can be recorded in a cache by setting
	 * is_caching() to true. A newly initialized iterator on a cached
	 * stream will start at the beginning of the cache.
	 */
	template<typename T>
	class Stream {
		bool caching;
		std::vector<T> cache;

	protected:
		T value;

		/** Produce a new value. Use this to get caching to work. */
		void produce(T v) {
			if (caching)
				cache.push_back(v);
			value = v;
		}

	public:
		/** Return the current element. */
		virtual T operator*(void) const { return value; };
		/** Produce the next element. */
		virtual Stream<T>& operator++(void) = 0;
		/** Return if the stream points at a valid value. */
		virtual operator bool(void) const   = 0;

		class iterator;
		friend class iterator;

		iterator begin(void) { return iterator(this); }
		iterator end(void)   { return iterator();     }

		/** Tell whether the stream is currently caching. */
		bool& is_caching(void) { return caching; }

		/** Return the number of elements in the cache. */
		size_t size(void) const { return cache.size(); }

		/** Start caching and exhaust the stream. Return the number
		 * of seen elements. */
		size_t cache_all(void) {
			is_caching() = true;
			for (auto it = begin(), e = end(); it != e; ++it)
				/* fills cache from afar */;
			return size();
		}
	};

	/**
	 * STL-compatible iterator for a Stream<T>. Instances of this are
	 * obtained from `Stream<T>::{begin,end}` enabling range-based
	 * for loops over streams.
	 */
	template<typename T>
	class Stream<T>::iterator {
		Stream<T>* st;
		unsigned int idx = 0;

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
		bool operator!=(const iterator& b) const {
			if (!b.st)
				return idx < st->size() || !!*st;
			throw X::Stream::Comparison();
		}

		bool operator==(const iterator& b) const {
			return not (*this != b);
		}

		T operator*(void) const {
			if (idx < st->size())
				return st->cache[idx];
			return **st;
		}

		iterator& operator++(void) {
			if (idx < st->size())
				++idx;
			else
				++*st;
			return *this;
		}
	};
}

#endif /* PROPCALC_STREAM_HPP */
