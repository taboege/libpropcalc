/*
 * variable.cpp - Variable and Domain
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

#include <memory>
#include <mutex>

#include <propcalc/variable.hpp>

using namespace std;

namespace Propcalc {

/*
 * Cache
 */

VarRef Cache::resolve(std::string name) {
	const std::lock_guard<std::mutex> lock(access);
	VarRef var;

	auto it = by_name.find(name);
	if (it == by_name.end()) {
		auto uvar = make_unique<Variable>(name);
		var = uvar.get();
		cache.push_back(move(uvar));
		by_name.insert({ name, var });
		by_nr.push_back(var);
		/* VarNr are 1-based, so by_nr.size() after the
		 * push_back() we just did is the right thing. */
		by_ref.insert({ var, by_nr.size() });
	}
	else {
		var = it->second;
	}
	return var;
}

VarNr Cache::pack(VarRef var) {
	const std::lock_guard<std::mutex> lock(access);
	return by_ref[var];
}

VarRef Cache::unpack(VarNr nr) {
	const std::lock_guard<std::mutex> lock(access);
	return by_nr[nr - 1];
}

vector<VarRef> Cache::list(void) const {
	const std::lock_guard<std::mutex> lock(access);
	return by_nr;
}

size_t Cache::size(void) const {
	const std::lock_guard<std::mutex> lock(access);
	return cache.size();
}

vector<VarRef> Cache::sort(unordered_set<VarRef>& pile) const {
	const std::lock_guard<std::mutex> lock(access);
	vector<VarRef> vec;

	/* Amortized linear in domain size */
	for (auto& v : by_nr) {
		if (pile.count(v) > 0)
			vec.push_back(v);
	}
	return vec;
}

}
