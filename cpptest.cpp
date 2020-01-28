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
	for (const auto& [assigned, value] : fm.truthtable()) {
		for (size_t i = 1; i <= assigned.vars().size(); ++i)
			cout << assigned[i];
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
	for (auto& clause : fm.cnf()) {
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
	for (auto& clause : fm.tseitin()) {
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

	return 0;
}
