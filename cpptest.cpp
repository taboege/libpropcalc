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
	cout << fm.to_rpn() << endl;
	cout << fm.to_pn() << endl;
	return 0;
}
