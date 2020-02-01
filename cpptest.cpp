#include <iostream>
#include <string>

#include <propcalc/propcalc.hpp>

using namespace std;

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
	cout << (fm == fm1).to_postfix() << endl;
	cout << (fm1 >> fm2).to_postfix() << endl;
	cout << endl;

	cout << (fm1 | ~fm3).to_infix() << endl;
	cout << ((fm1 | fm2) & ~~fm3).to_infix() << endl;
	cout << (fm1 & fm3 & fm2).to_infix() << endl;
	cout << endl;

	cout << "seen the following variables:" << endl; {
		Propcalc::VarNr i = 1;
		for (auto& v : Propcalc::Formula::DefaultDomain->list())
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
			for (auto& v : assigned.set())
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
				cout << "& ";
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
				cout << "& ";
			cout << (clause[v] ? "" : "~") << v->name << " ";
		}
		cout << "}" << endl;
	}
	cout << endl;

	cout << "Original   formula: " << fm.to_infix() << endl;
	cout << "Simplified formula: " << fm.simplify().to_infix() << endl;
	cout << endl;

	cout << "Allocating many variables using unpack:" << endl; {
		Propcalc::Formula::DefaultDomain->unpack(15);
		Propcalc::VarNr i = 1;
		for (auto& v : Propcalc::Formula::DefaultDomain->list())
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

	return 0;
}
