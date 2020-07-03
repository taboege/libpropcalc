#include <iostream>
#include <sstream>
#include <string>

#include <propcalc/propcalc.hpp>

using namespace std;

string dimacs = R"(
c Gaussoids on n=3
p cnf 6 42
1 4 -3 0
1 4 -2 0
2 4 -1 0
2 4 -3 0
1 3 -2 0
1 3 -4 0
1 2 -3 -5 0
3 2 -1 0
3 2 -4 0
4 2 -3 0
4 2 -1 0
3 1 -4 0
3 1 -2 0
3 4 -1 -5 0
1 6 -5 0
1 6 -2 0
2 6 -1 0
2 6 -5 0
1 5 -2 0
1 5 -6 0
1 2 -5 -3 0
5 2 -1 0
5 2 -6 0
6 2 -5 0
6 2 -1 0
5 1 -6 0
5 1 -2 0
5 6 -1 -3 0
3 6 -5 0
3 6 -4 0
4 6 -3 0
4 6 -5 0
3 5 -4 0
3 5 -6 0
3 4 -5 -1 0
5 4 -3 0
5 4 -6 0
6 4 -5 0
6 4 -3 0
5 3 -6 0
5 3 -4 0
5 6 -3 -1 0
)";

template<typename T>
static size_t count_stream(Propcalc::Stream<T>& st) {
	size_t size = 0;
	for ([[maybe_unused]] auto v : st)
		size++;
	return size;
}

int main(int argc, char* argv[]) {
	if (argc < 2) {
		cerr << "no formula given" << endl;
		return 1;
	}

	string fmstr(argv[1]);
	Propcalc::Formula fm(fmstr);
	Propcalc::Formula fm1("[12|]&[12|3]");
	Propcalc::Formula fm2("[13|]^[23|]");
	Propcalc::Formula fm3("[12|]");

	cout << fm.to_postfix() << endl;
	cout << (~fm).to_postfix() << endl;
	cout << ((fm & fm1) | fm2).to_postfix() << endl;
	cout << fm.eqvf(fm1).to_postfix() << endl;
	cout << fm1.thenf(fm2).to_postfix() << endl;
	cout << endl;

	cout << (fm1 | ~fm3).to_infix() << endl;
	cout << ((fm1 | fm2) & ~~fm3).to_infix() << endl;
	cout << (fm1 & fm3 & fm2).to_infix() << endl;
	cout << endl;

	cout << "seen the following variables:" << endl; {
		Propcalc::VarNr i = 1;
		for (auto& v : Propcalc::Formula::DefaultDomain.list())
			cout << v->to_string() << ": " << i++ << endl;
	}
	cout << endl;

	auto vars = fm.vars();
	auto assign = Propcalc::Assignment(vars);
	while (!assign.overflown()) {
		for (auto& v : vars)
			cout << v->name << ": " << assign[v] << " ";
		cout << "(short: ";
		for (auto& v : assign.vars())
			cout << assign[v];
		cout << ")" << endl;
		++assign;
	}
	cout << endl;

	cout << "truth table of " << fm.to_infix() << ":" << endl;
	for (const auto& [assigned, value] : fm.truthtable()) {
		for (auto& v : assigned.vars())
			cout << assigned[v];
		cout << ": " << value << endl;
	}
	cout << endl;

	cout << "satisfying assignments of " << fm.to_infix() << ":" << endl;
	for (const auto& [assigned, value] : fm.truthtable()) {
		if (value) {
			cout << "{ ";
			for (auto& v : assigned.vars())
				cout << v->name << " ";
			cout << "}" << endl;
		}
	}
	cout << endl;

	cout << "CNF clauses of " << fm.to_infix() << ":" << endl;
	for (auto clause : fm.cnf()) {
		cout << "{ ";
		int i = 0;
		for (auto& v : clause.vars()) {
			if (i++)
				cout << "| ";
			cout << (clause[v] ? "" : "~") << v->name << " ";
		}
		cout << "}" << endl;
	}
	cout << endl;

	cout << "Tseitin transform of " << fm.to_infix() << ":" << endl;
	for (auto clause : fm.tseitin()) {
		cout << "{ ";
		int i = 0;
		for (auto& v : clause.vars()) {
			if (i++)
				cout << "| ";
			cout << (clause[v] ? "" : "~") << v->name << " ";
		}
		cout << "}" << endl;
	}
	cout << endl;

	cout << "Original   formula: " << fm.to_infix() << endl;
	cout << "Simplified formula: " << fm.simplify().to_infix() << endl;
	cout << endl;

	cout << "Allocating many variables using unpack:" << endl; {
		Propcalc::Formula::DefaultDomain.unpack(15);
		Propcalc::VarNr i = 1;
		for (auto& v : Propcalc::Formula::DefaultDomain.list())
			cout << v->to_string() << ": " << i++ << endl;
	}
	cout << endl;

	{
		auto cnf = fm.cnf();
		cout << "Number of CNF clauses: " << count_stream(cnf) << endl;
		cout << "Counting again:        " << count_stream(cnf) << endl;
		auto cache = fm.cnf().cache();
		cout << "Number of CNF clauses (cached): " << count_stream(cache) << endl;
		cout << "Counting again (cached):        " << count_stream(cache) << endl;
	}
	cout << endl;

	{
		cout << "Reading DIMACS CNF file:" << endl;
		auto ss = stringstream(dimacs);
		auto fm = Propcalc::DIMACS::read(ss);
		cout << fm.to_infix() << endl;
	}
	cout << endl;

	{
		cout << "Writing DIMACS CNF file:" << endl;
		auto cnf = fm.cnf();
		Propcalc::DIMACS::write(cout, cnf, fm.domain);
		cout << endl;

		cout << "Writing DIMACS CNF of Tseitin transform:" << endl;
		auto tsei = fm.tseitin();
		Propcalc::DIMACS::write(cout, tsei, tsei.domain, {"Tseitin transform of " + fm.to_infix()});
	}
	cout << endl;

	return 0;
}
