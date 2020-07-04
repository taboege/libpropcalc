/*
 * dimacs.cpp - DIMACS CNF files
 *
 * Copyright (C) 2020 Tobias Boege
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the Artistic License 2.0
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * Artistic License 2.0 for more details.
 */

#include <string>
#include <sstream>
#include <type_traits>

#include <propcalc/dimacs.hpp>

using namespace std;

namespace Propcalc {

static inline bool starts_with(const string& line, const string& prefix) {
	return line.rfind(prefix, 0) == 0;
}

DIMACS::In& DIMACS::In::operator++(void) {
	/*
	 * TODO: The DIMACS CNF format gives the serializer a bit more slack
	 * than we anticipate here: a single clause may span several lines.
	 * It consists of non-zero integers separated by spaces and linebreaks,
	 * terminated by a zero integer. Here we expect a clause to end on the
	 * same line it started and to have at most one clause per line.
	 *
	 * We do not even detect when this assumption is violated and read
	 * the formula incorrectly!
	 */
	while (!in.eof()) {
		string line;
		getline(in, line);
		if (not line.length())
			continue;

		/* Don't care about the program line... */
		/* TODO: We should probably care for validation purposes. */
		if (starts_with(line, "p cnf "))
			continue;
		/* ... or comments. */
		if (starts_with(line, "c "))
			continue;

		Clause last;
		stringstream ss(line, ios_base::in);
		while (!ss.eof()) {
			long lit;
			ss >> lit;
			if (!lit)
				break;
			auto var = domain->unpack(abs(lit));
			last[var] = lit > 0;
		}
		produce(last);
		break;
	}
	return *this;
}

Formula DIMACS::read(istream& in, Domain* domain) {
	auto clauses = DIMACS::In(in, domain);
	return Formula(clauses, domain);
}

string DIMACS::Out::operator*(void) {
	auto cl = *st;
	string line;
	for (auto& v : cl.vars()) {
		auto nr = static_cast<make_signed<VarNr>::type>(domain->pack(v));
		if (not cl[v])
			nr = -nr;
		line += to_string(nr) + " ";
	}
	line += "0";
	produce(line);
	return line;
}

void DIMACS::write(ostream& out, Stream<Clause>& clauses, Domain* domain, vector<string> comments) {
	size_t nclauses = clauses.cache_all();
	DIMACS::Header header{comments, 0, nclauses};
	for (auto cl : clauses) {
		for (auto& v : cl.vars()) {
			auto nr = domain->pack(v);
			header.maxvar = max(header.maxvar, nr);
		}
	}
	DIMACS::write(out, clauses, domain, header);
}

void DIMACS::write(ostream& out, Stream<Clause>& clauses, Domain* domain, DIMACS::Header header) {
	auto st = DIMACS::Out(clauses, domain);
	for (auto& line : header.comments)
		out << "c " << line << endl;
	out << "p cnf " << header.maxvar << " " << header.nclauses << endl;
	for (auto line : st)
		out << line << endl;
}

} /* namespace Propcalc */
