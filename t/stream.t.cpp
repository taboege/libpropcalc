#include <iostream>

#include <tappp/tappp.hpp>
#include <propcalc/propcalc.hpp>

#include <cstdlib>

using namespace TAP;
using namespace Propcalc;

/** A half-open range of integers [from, to). */
class Range : public Stream<int> {
	int cur, to;

public:
	Range(int from, int to, bool caching = false) :
			cur(from), to(to)
	{
		is_caching() = caching;

		if (cur < to)
			produce(cur);
	}

	virtual operator bool(void) const {
		return cur < to;
	}

	virtual Range& operator++(void) {
		++cur;
		if (cur < to)
			produce(cur);
		return *this;
	}

	Range operator++(int) {
		Range tmp(*this);
		operator++();
		return tmp;
	}
};

int main(void) {
	plan(3);

	std::cout << std::boolalpha;

	SUBTEST(6, "non-caching") {
		Range r(10, 13);
		is(*r++, 10, "10...");
		is(*r++, 11, "11...");
		is(*r++, 12, "12...");
		is(!!r, false, "exhausted");
		is(r.size(), 0, "none cached");
		is(r.begin(), r.end(), "reiteration impossible");
	}

	SUBTEST(6 + 3, "caching") {
		Range r(10, 13, true);
		is(*r++, 10, "10...");
		is(*r++, 11, "11...");
		is(*r++, 12, "12...");
		is(!!r, false, "exhausted");
		is(r.size(), 3, "all cached");
		isnt(r.begin(), r.end(), "reiteration possible");

		int expected = 10;
		for (auto got : r)
			is(got, expected++, to_string(expected) + "...");
	}

	SUBTEST(1 + 2 + 2 + 5 + 3, "caching with restart in between") {
		Range r(10, 13, true);
		is (*r++, 10, "10...");

		/* Reiterate before the stream is consumed. Should start at
		 * the beginning of the cache and according to the loop break
		 * consume one more element. */
		int expected = 10;
		for (auto got : r) {
			is(got, expected++, to_string(expected) + "...");
			if (expected == 12)
				break;
		}

		/* Same again. Should iterate what we had before, now does not
		 * consume anything new. */
		expected = 10;
		for (auto got : r) {
			is(got, expected++, to_string(expected) + "...");
			if (expected == 12)
				break;
		}

		is(!!r, true, "not exhausted yet");
		is(*r++, 11, "11...");
		is(*r++, 12, "12...");
		is(!!r, false, "exhausted");
		is(r.size(), 3, "all cached");

		/* Finally reiterate everything. */
		expected = 10;
		for (auto got : r)
			is(got, expected++, to_string(expected) + "...");
	}

	return EXIT_SUCCESS;
}
