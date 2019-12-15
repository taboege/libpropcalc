/*
 * variable.cpp - Variable and Domain
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

#include <memory>
#include <mutex>

#include <propcalc/variable.hpp>

using namespace std;

namespace Propcalc {

/*
 * DefaultDomain is a Cache object. It is the default global domain for
 * Variables used by the Formula parser.
 */

auto DefaultDomain = make_shared<Cache>();

/*
 * Cache
 */

VarRef Cache::resolve(std::string name) {
	const std::lock_guard<std::mutex> lock(access);
	VarRef var;

	auto it = cache.find(name);
	if (it == cache.end()) {
		auto uvar = make_unique<Variable>(name);
		var = uvar.get();
		cache.insert({ name, move(uvar) });
		order.push_back(var);
	}
	else {
		var = it->second.get();
	}
	return var;
}

vector<VarRef> Cache::list(void) const {
	const std::lock_guard<std::mutex> lock(access);
	return order;
}

vector<VarRef> Cache::sort(unordered_set<VarRef>& pile) const {
	const std::lock_guard<std::mutex> lock(access);
	vector<VarRef> vec;

	/* Amortized linear in domain size */
	for (auto& v : order) {
		if (pile.count(v) > 0)
			vec.push_back(v);
	}
	return vec;
}

}
