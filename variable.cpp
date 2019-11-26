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

shared_ptr<Cache> DefaultDomain = make_shared<Cache>();

/*
 * Cache
 */

shared_ptr<Variable> Cache::get(std::string name) {
	const std::lock_guard<std::mutex> lock(update);
	shared_ptr<Variable> var;

	auto it = cache.find(name);
	if (it == cache.end()) {
		var = make_shared<Variable>(name);
		size_t nr = 1 + cache.size();
		cache.insert({ name, make_pair(var, nr) });
	}
	else {
		var = it->second.first;
	}

	return var;
}

vector<shared_ptr<Variable>> Cache::list(void) {
	const std::lock_guard<std::mutex> lock(update);
	vector<shared_ptr<Variable>> values;

	for (auto& pair : cache)
		values.push_back(pair.second.first);
	return values;
}

size_t Cache::get_nr(string name) {
	const std::lock_guard<std::mutex> lock(update);

	auto it = cache.find(name);
	if (it == cache.end())
		return 0;
	return it->second.second;
}

size_t Cache::get_nr(shared_ptr<Variable> var) {
	return get_nr(var->name);
}

}
