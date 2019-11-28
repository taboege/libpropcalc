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

const Variable* Cache::get(std::string name) {
	const std::lock_guard<std::mutex> lock(update);
	const Variable* var;

	auto it = cache.find(name);
	if (it == cache.end()) {
		auto uvar = make_unique<Variable>(name);
		var = uvar.get();
		cache.insert({ name, move(uvar) });
	}
	else {
		var = it->second.get();
	}
	return var;
}

vector<const Variable*> Cache::list(void) {
	const std::lock_guard<std::mutex> lock(update);
	vector<const Variable*> values;

	for (auto& pair : cache)
		values.push_back(pair.second.get());
	return values;
}

}
