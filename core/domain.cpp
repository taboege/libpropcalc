/*
 * domain.cpp - Variable and Domain
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

#include <propcalc/domain.hpp>

using namespace std;

namespace Propcalc {

/*
 * Cache
 */

/* Needs the lock to be held! */
pair<VarNr, VarRef> Cache::new_variable(string name) {
	if (frozen)
		throw X::Cache::Frozen();
	auto uvar = make_unique<Variable>(name);
	return put_variable(move(uvar));
}

/* Needs the lock to be held! */
pair<VarNr, VarRef> Cache::put_variable(unique_ptr<Variable> uvar) {
	if (frozen)
		throw X::Cache::Frozen();
	VarRef var = uvar.get();
	cache.push_back(move(uvar));
	by_name.insert({ var->name, var });
	by_nr.push_back(var);
	/* VarNr are 1-based, so by_nr.size() after the
	 * push_back() we just did is the right thing. */
	VarNr nr = by_nr.size();
	by_ref.insert({ var, nr });
	return make_pair(nr, var);
}

VarRef Cache::resolve(std::string name) {
	const std::lock_guard<std::mutex> lock(access);

	VarRef var;
	auto it = by_name.find(name);
	if (it == by_name.end()) {
		tie(ignore, var) = new_variable(name);
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

	if (nr == 0)
		throw X::Domain::InvalidVarNr();

	auto max = cache.size();
	while (max < nr) {
		tie(max, ignore) = new_variable(to_string(max + 1));
	}
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

/**
 * Slightly more efficient sorting than the default in Domain.
 */
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

void Cache::freeze(void) {
	const std::lock_guard<std::mutex> lock(access);
	frozen = true;
}

void Cache::thaw(void) {
	const std::lock_guard<std::mutex> lock(access);
	frozen = false;
}

}
