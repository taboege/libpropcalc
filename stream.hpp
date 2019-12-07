/*
 * stream.hpp - Stream
 *
 * Copyright (C) 2019 Tobias Boege
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

#include <propcalc/assignment.hpp>

namespace Propcalc {
	template<typename T>
	class Stream {
		virtual bool exhausted(void) const = 0;

		virtual Stream<T>& operator++(void) = 0;

		virtual T value(void) = 0;
		T operator*(void) {
			return this->value();
		}
	};
}


#endif /* PROPCALC_STREAM_HPP */
