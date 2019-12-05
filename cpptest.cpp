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
	while (!tt.end()) {
		for (size_t i = 1; i <= tt.assigned().vars().size(); ++i)
			cout << tt.assigned()[i];
		cout << ": " << tt.value() << endl;
		++tt;
	}
	cout << endl;

	cout << "satisfying assignments of " << fm.to_infix() << ":" << endl;
	tt = fm.truthtable();
	while (!tt.end()) {
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
	auto cc = fm.clauses();
	while (!cc.end()) {
		cout << "{ ";
		int i = 0;
		for (auto& v : cc.assigned().set()) {
			if (i++)
				cout << "& ";
			cout << "~" << v->name << " ";
		}
		cout << "}" << endl;
		++cc;
	}
	cout << endl;

	return 0;
}
