#include <tappp/tappp.hpp>
#include <propcalc/propcalc.hpp>

using namespace TAP;
using namespace Propcalc;

int main(void) {
	plan(4);

	Formula fm("x -> y -> z");
	VarRef x = fm.get_domain()->resolve("x");
	VarRef y = fm.get_domain()->resolve("y");
	VarRef z = fm.get_domain()->resolve("z");

	is(fm.simplify(Assignment({{ x, false }})).to_postfix(),
		"\\T", "implication simplifies to true");
	is(fm.eval(Assignment({{ x, false }})),
		true, "implication evals to true (short-circuit)");
	is(fm.eval(Assignment({{ x, true }, { y, true }, { z, false }})),
		false, "unsatisfying assignment (long-circuit)");

	throws<std::out_of_range>([&] {
		diag(fm.eval(Assignment({{ y, false }})) ? "true" : "false");
	}, "eval over undefined variable throws");

	return EXIT_SUCCESS;
}
