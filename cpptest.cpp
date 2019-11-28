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

	cout << (fm1 | ~fm3).to_infix() << endl;
	cout << ((fm1 | fm2) & ~~fm3).to_infix() << endl;
	cout << (fm1 & fm3 & fm2).to_infix() << endl;

	cout << "seen the following variables:" << endl;
	size_t i = 0;
	for (auto& v : Propcalc::DefaultDomain->list())
		cout << v->to_string() << ": " << i++ << endl;

	auto vars = fm.vars();
	auto assign = Propcalc::Assignment(vars);
	while (!assign.overflow) {
		for (auto& v : vars)
			cout << v->name << ": " << assign[v] << " ";
		cout << endl;
		++assign;
	}

	return 0;
}
