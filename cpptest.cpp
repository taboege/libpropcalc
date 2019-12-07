#include <iostream>
#include <string>

#include <propcalc/propcalc.hpp>

using namespace std;

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

	cout << "seen the following variables:" << endl;
	size_t i = 0;
	for (auto& v : Propcalc::DefaultDomain->list())
		cout << v->to_string() << ": " << i++ << endl;
	cout << endl;

	auto vars = fm.vars();
	auto assign = Propcalc::Assignment(vars);
	while (!assign.overflown()) {
		for (auto& v : vars)
			cout << v->name << ": " << assign[v] << " ";
		cout << "(short: ";
		for (size_t i = 1; i <= assign.vars().size(); ++i)
			cout << assign[i];
		cout << ")" << endl;
		++assign;
	}
	cout << endl;

	cout << "truth table of " << fm.to_infix() << ":" << endl;
	auto tt = fm.truthtable();
	while (!tt.exhausted()) {
		for (size_t i = 1; i <= tt.assigned().vars().size(); ++i)
			cout << tt.assigned()[i];
		cout << ": " << tt.value() << endl;
		++tt;
	}
	cout << endl;

	cout << "satisfying assignments of " << fm.to_infix() << ":" << endl;
	tt = fm.truthtable();
	while (!tt.exhausted()) {
		if (tt.value()) {
			cout << "{ ";
			for (auto& v : tt.assigned().set())
				cout << v->name << " ";
			cout << "}" << endl;
		}
		++tt;
	}
	cout << endl;

	cout << "CNF clauses of " << fm.to_infix() << ":" << endl;
	auto cnf = fm.cnf();
	while (!cnf.exhausted()) {
		cout << "{ ";
		int i = 0;
		for (auto& v : cnf.clause().vars()) {
			if (i++)
				cout << "& ";
			cout << (cnf.clause()[v] ? "" : "~") << v->name << " ";
		}
		cout << "}" << endl;
		++cnf;
	}
	cout << endl;

	cout << "Tseitin transform of " << fm.to_infix() << ":" << endl;
	auto tsei = fm.tseitin();
	while (!tsei.exhausted()) {
		cout << "{ ";
		int i = 0;
		for (auto& v : tsei.clause().vars()) {
			if (i++)
				cout << "& ";
			cout << (tsei.clause()[v] ? "" : "~") << v->name << " ";
		}
		cout << "}" << endl;
		++tsei;
	}
	cout << endl;

	return 0;
}
