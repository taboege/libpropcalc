/*
 * stream.hpp - Stream
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

namespace Propcalc {
	template<typename T>
	class Stream {
	public:
		virtual T operator*(void) const     = 0;
		virtual Stream<T>& operator++(void) = 0;
		virtual operator bool(void) const   = 0;

		class iterator {
			Stream<T>* st;

		public:
			using iterator_category = std::forward_iterator_tag;
			using value_type = T;
			//using difference_type = std::ptrdiff_t;
			//using pointer = T*;
			//using reference = T&;

			iterator(Stream<T>* st) : st(st) { }
			iterator(void)     : st(nullptr) { }

			bool operator!=(iterator& b) const {
				if (!b.st)
					return !!*st;
				throw "comparing stream iterators";
			}

			T operator*(void) const {
				return **st;
			}

			iterator& operator++(void) {
				++*st;
				return *this;
			}
		};

		iterator begin(void) { return iterator(this); }
		iterator end(void)   { return iterator();     }
	};
}

#endif /* PROPCALC_STREAM_HPP */
