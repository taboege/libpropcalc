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
	Propcalc::Formula fm2("[13|]|[23|]");

	cout << fm.to_rpn() << endl;
	cout << (~fm).to_rpn() << endl;
	cout << ((fm & fm1) | fm2).to_rpn() << endl;
	cout << (fm == fm1).to_rpn() << endl;
	cout << (fm1 >> fm2).to_rpn() << endl;

	return 0;
}
