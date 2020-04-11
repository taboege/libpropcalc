#include <iostream>

#include <tappp/tappp.hpp>
#include <propcalc/propcalc.hpp>

#include <cstdlib>

using namespace TAP;
using namespace Propcalc;

static auto ttfms = std::vector<Formula>{
	{"\\T"}, {"\\F"},

	{"a"}, {"~a"},

	{"a & b"}, {"~a & b"},
	{"a | b"}, {"~a | b"},
	{"a > b"}, {"~a > b"},
	{"a = b"}, {"~a = b"},
	{"a ^ b"}, {"~a ^ b"},

	{"a & b & c"}, {"a & b | c"}, {"a & b > c"}, {"a & b = c"}, {"a & b ^ c"},
	{"a | b | c"}, {"a | b > c"}, {"a | b = c"}, {"a | b ^ c"},
	{"a > b > c"}, {"a > b = c"}, {"a > b ^ c"},
	{"a = b = c"}, {"a = b ^ c"},
	{"a ^ b ^ c"},

	{"a & b & a"}, {"a & b | a"}, {"a & b > a"}, {"a & b = a"}, {"a & b ^ a"},
	{"a | b | a"}, {"a | b > a"}, {"a | b = a"}, {"a | b ^ a"},
	{"a > b > a"}, {"a > b = a"}, {"a > b ^ a"},
	{"a = b = a"}, {"a = b ^ a"},
	{"a ^ b ^ a"},
};

static auto ttvals = std::vector<std::vector<bool>>{
	{ true },  /* \T */
	{ false }, /* \F */

	/* [~a], [a] */
	{ false, true }, /*  a */
	{ true, false }, /* ~a */

	/* [~a ~b], [a ~b], [~a b] [a b] */
	{ false, false, false, true }, /*  a & b */
	{ false, false, true, false }, /* ~a & b */
	{ false, true, true, true },   /*  a | b */
	{ true, false, true, true },   /* ~a | b */
	{ true, false, true, true },   /*  a > b */
	{ false, true, true, true },   /* ~a > b */
	{ true, false, false, true },  /*  a = b */
	{ false, true, true, false },  /* ~a = b */
	{ false, true, true, false },  /*  a ^ b */
	{ true, false, false, true },  /* ~a ^ b */

	/* [~a ~b ~c], [a ~b ~c], [~a b ~c], [a b ~c], [~a ~b c], [a ~b c], [~a b c], [a b c] */
	{ false, false, false, false, false, false, false, true }, /* a & b & c */
	{ false, false, false, true, true, true, true, true },     /* a & b | c */
	{ true, true, true, false, true, true, true, true },       /* a & b > c */
	{ true, true, true, false, false, false, false, true },    /* a & b = c */
	{ false, false, false, true, true, true, true, false },    /* a & b ^ c */
	{ false, true, true, true, true, true, true, true },       /* a | b | c */
	{ true, false, false, false, true, true, true, true },     /* a | b > c */
	{ true, false, false, false, false, true, true, true },    /* a | b = c */
	{ false, true, true, true, true, false, false, false },    /* a | b ^ c */
	{ true, true, true, false, true, true, true, true },       /* a > b > c */
	{ false, true, false, false, true, false, true, true },    /* a > b = c */
	{ true, false, true, true, false, true, false, false },    /* a > b ^ c */
	{ false, true, true, false, true, false, false, true },    /* a = b = c */
	{ true, false, false, true, false, true, true, false },    /* a = b ^ c */
	{ false, true, true, false, true, false, false, true },    /* a ^ b ^ c */

	/* [~a ~b], [a ~b], [~a b] [a b] */
	{ false, false, false, true }, /* a & b & a */
	{ false, true, false, true },  /* a & b | a */
	{ true, true, true, true },    /* a & b > a */
	{ true, false, true, true },   /* a & b = a */
	{ false, true, false, false }, /* a & b ^ a */
	{ false, true, true, true },   /* a | b | a */
	{ true, true, false, true },   /* a | b > a */
	{ true, true, false, true },   /* a | b = a */
	{ false, false, true, false }, /* a | b ^ a */
	{ true, true, true, true },    /* a > b > a */
	{ false, false, false, true }, /* a > b = a */
	{ true, true, true, false },   /* a > b ^ a */
	{ false, false, true, true },  /* a = b = a */
	{ true, true, false, false },  /* a = b ^ a */
	{ false, false, true, true },  /* a ^ b ^ a */
};

static bool is_truthtable(const Formula& f, std::vector<bool>& ttval, std::string message = "") {
	bool is_ok = true;
	Assignment assign;
	bool val;

	if (message.empty())
		message = f.to_postfix();

	auto tt = f.truthtable().cache();
	if (tt.size() != 1UL << f.vars().size()) {
		fail(message);
		diag("truthtable has ", tt.size(), " rows instead of ", 1UL << f.vars().size());
		return false;
	}

	unsigned int idx = 0;
	for (const auto& [assigned, value] : tt) {
		is_ok &= value == ttval[idx];
		if (!is_ok) {
			/* For diagnostic */
			assign = assigned;
			val = value;
			break;
		}
		idx++;
	}

	if (!ok(is_ok, message)) {
		diag("mismatched at assignment ", assign);
		diag("  Got:      ", val);
		diag("  Expected: ", ttval[idx]);
	}
	return is_ok;
}

static auto testfms = std::vector<Formula>{
	{"\\T"}, {"\\F"},

	{"a"}, {"~a"},

	{"a & b"}, {"~a & b"}, {"a & ~b"}, {"~a & ~b"},
	{"a | b"}, {"~a | b"}, {"a | ~b"}, {"~a | ~b"},
	{"a > b"}, {"~a > b"}, {"a > ~b"}, {"~a > ~b"},
	{"a = b"}, {"~a = b"}, {"a = ~b"}, {"~a = ~b"},
	{"a ^ b"}, {"~a ^ b"}, {"a ^ ~b"}, {"~a ^ ~b"},

	{"a & b & c"}, {"~a & b & c"}, {"a & ~b & c"}, {"a & b & ~c"}, {"~a & ~b & c"}, {"~a & b & ~c"}, {"a & ~b & ~c"}, {"~a & ~b & ~c"},
	{"a & b | c"}, {"~a & b | c"}, {"a & ~b | c"}, {"a & b | ~c"}, {"~a & ~b | c"}, {"~a & b | ~c"}, {"a & ~b | ~c"}, {"~a & ~b | ~c"},
	{"a | b & c"}, {"~a | b & c"}, {"a | ~b & c"}, {"a | b & ~c"}, {"~a | ~b & c"}, {"~a | b & ~c"}, {"a | ~b & ~c"}, {"~a | ~b & ~c"},
	{"a & b > c"}, {"~a & b > c"}, {"a & ~b > c"}, {"a & b > ~c"}, {"~a & ~b > c"}, {"~a & b > ~c"}, {"a & ~b > ~c"}, {"~a & ~b > ~c"},
	{"a > b & c"}, {"~a > b & c"}, {"a > ~b & c"}, {"a > b & ~c"}, {"~a > ~b & c"}, {"~a > b & ~c"}, {"a > ~b & ~c"}, {"~a > ~b & ~c"},
	{"a & b = c"}, {"~a & b = c"}, {"a & ~b = c"}, {"a & b = ~c"}, {"~a & ~b = c"}, {"~a & b = ~c"}, {"a & ~b = ~c"}, {"~a & ~b = ~c"},
	{"a = b & c"}, {"~a = b & c"}, {"a = ~b & c"}, {"a = b & ~c"}, {"~a = ~b & c"}, {"~a = b & ~c"}, {"a = ~b & ~c"}, {"~a = ~b & ~c"},
	{"a & b ^ c"}, {"~a & b ^ c"}, {"a & ~b ^ c"}, {"a & b ^ ~c"}, {"~a & ~b ^ c"}, {"~a & b ^ ~c"}, {"a & ~b ^ ~c"}, {"~a & ~b ^ ~c"},
	{"a ^ b & c"}, {"~a ^ b & c"}, {"a ^ ~b & c"}, {"a ^ b & ~c"}, {"~a ^ ~b & c"}, {"~a ^ b & ~c"}, {"a ^ ~b & ~c"}, {"~a ^ ~b & ~c"},

	{"a | b | c"}, {"~a | b | c"}, {"a | ~b | c"}, {"a | b | ~c"}, {"~a | ~b | c"}, {"~a | b | ~c"}, {"a | ~b | ~c"}, {"~a | ~b | ~c"},
	{"a | b > c"}, {"~a | b > c"}, {"a | ~b > c"}, {"a | b > ~c"}, {"~a | ~b > c"}, {"~a | b > ~c"}, {"a | ~b > ~c"}, {"~a | ~b > ~c"},
	{"a > b | c"}, {"~a > b | c"}, {"a > ~b | c"}, {"a > b | ~c"}, {"~a > ~b | c"}, {"~a > b | ~c"}, {"a > ~b | ~c"}, {"~a > ~b | ~c"},
	{"a | b = c"}, {"~a | b = c"}, {"a | ~b = c"}, {"a | b = ~c"}, {"~a | ~b = c"}, {"~a | b = ~c"}, {"a | ~b = ~c"}, {"~a | ~b = ~c"},
	{"a = b | c"}, {"~a = b | c"}, {"a = ~b | c"}, {"a = b | ~c"}, {"~a = ~b | c"}, {"~a = b | ~c"}, {"a = ~b | ~c"}, {"~a = ~b | ~c"},
	{"a | b ^ c"}, {"~a | b ^ c"}, {"a | ~b ^ c"}, {"a | b ^ ~c"}, {"~a | ~b ^ c"}, {"~a | b ^ ~c"}, {"a | ~b ^ ~c"}, {"~a | ~b ^ ~c"},
	{"a ^ b | c"}, {"~a ^ b | c"}, {"a ^ ~b | c"}, {"a ^ b | ~c"}, {"~a ^ ~b | c"}, {"~a ^ b | ~c"}, {"a ^ ~b | ~c"}, {"~a ^ ~b | ~c"},

	{"a > b > c"}, {"~a > b > c"}, {"a > ~b > c"}, {"a > b > ~c"}, {"~a > ~b > c"}, {"~a > b > ~c"}, {"a > ~b > ~c"}, {"~a > ~b > ~c"},
	{"a > b = c"}, {"~a > b = c"}, {"a > ~b = c"}, {"a > b = ~c"}, {"~a > ~b = c"}, {"~a > b = ~c"}, {"a > ~b = ~c"}, {"~a > ~b = ~c"},
	{"a = b > c"}, {"~a = b > c"}, {"a = ~b > c"}, {"a = b > ~c"}, {"~a = ~b > c"}, {"~a = b > ~c"}, {"a = ~b > ~c"}, {"~a = ~b > ~c"},
	{"a > b ^ c"}, {"~a > b ^ c"}, {"a > ~b ^ c"}, {"a > b ^ ~c"}, {"~a > ~b ^ c"}, {"~a > b ^ ~c"}, {"a > ~b ^ ~c"}, {"~a > ~b ^ ~c"},
	{"a ^ b > c"}, {"~a ^ b > c"}, {"a ^ ~b > c"}, {"a ^ b > ~c"}, {"~a ^ ~b > c"}, {"~a ^ b > ~c"}, {"a ^ ~b > ~c"}, {"~a ^ ~b > ~c"},

	{"a = b = c"}, {"~a = b = c"}, {"a = ~b = c"}, {"a = b = ~c"}, {"~a = ~b = c"}, {"~a = b = ~c"}, {"a = ~b = ~c"}, {"~a = ~b = ~c"},
	{"a = b ^ c"}, {"~a = b ^ c"}, {"a = ~b ^ c"}, {"a = b ^ ~c"}, {"~a = ~b ^ c"}, {"~a = b ^ ~c"}, {"a = ~b ^ ~c"}, {"~a = ~b ^ ~c"},
	{"a ^ b = c"}, {"~a ^ b = c"}, {"a ^ ~b = c"}, {"a ^ b = ~c"}, {"~a ^ ~b = c"}, {"~a ^ b = ~c"}, {"a ^ ~b = ~c"}, {"~a ^ ~b = ~c"},

	{"a ^ b ^ c"}, {"~a ^ b ^ c"}, {"a ^ ~b ^ c"}, {"a ^ b ^ ~c"}, {"~a ^ ~b ^ c"}, {"~a ^ b ^ ~c"}, {"a ^ ~b ^ ~c"}, {"~a ^ ~b ^ ~c"},

	{"a & a"}, {"a & ~a"},
	{"a | a"}, {"a | ~a"},
	{"a > a"}, {"a > ~a"},
	{"a = a"}, {"a = ~a"},
	{"a ^ a"}, {"a ^ ~a"},

	{"a & b & a"}, {"~a & b & a"}, {"a & ~b & a"}, {"a & b & ~a"}, {"~a & ~b & a"}, {"~a & b & ~a"}, {"a & ~b & ~a"}, {"~a & ~b & ~a"},
	{"a & b | a"}, {"~a & b | a"}, {"a & ~b | a"}, {"a & b | ~a"}, {"~a & ~b | a"}, {"~a & b | ~a"}, {"a & ~b | ~a"}, {"~a & ~b | ~a"},
	{"a | b & a"}, {"~a | b & a"}, {"a | ~b & a"}, {"a | b & ~a"}, {"~a | ~b & a"}, {"~a | b & ~a"}, {"a | ~b & ~a"}, {"~a | ~b & ~a"},
	{"a & b > a"}, {"~a & b > a"}, {"a & ~b > a"}, {"a & b > ~a"}, {"~a & ~b > a"}, {"~a & b > ~a"}, {"a & ~b > ~a"}, {"~a & ~b > ~a"},
	{"a > b & a"}, {"~a > b & a"}, {"a > ~b & a"}, {"a > b & ~a"}, {"~a > ~b & a"}, {"~a > b & ~a"}, {"a > ~b & ~a"}, {"~a > ~b & ~a"},
	{"a & b = a"}, {"~a & b = a"}, {"a & ~b = a"}, {"a & b = ~a"}, {"~a & ~b = a"}, {"~a & b = ~a"}, {"a & ~b = ~a"}, {"~a & ~b = ~a"},
	{"a = b & a"}, {"~a = b & a"}, {"a = ~b & a"}, {"a = b & ~a"}, {"~a = ~b & a"}, {"~a = b & ~a"}, {"a = ~b & ~a"}, {"~a = ~b & ~a"},
	{"a & b ^ a"}, {"~a & b ^ a"}, {"a & ~b ^ a"}, {"a & b ^ ~a"}, {"~a & ~b ^ a"}, {"~a & b ^ ~a"}, {"a & ~b ^ ~a"}, {"~a & ~b ^ ~a"},
	{"a ^ b & a"}, {"~a ^ b & a"}, {"a ^ ~b & a"}, {"a ^ b & ~a"}, {"~a ^ ~b & a"}, {"~a ^ b & ~a"}, {"a ^ ~b & ~a"}, {"~a ^ ~b & ~a"},

	{"a | b | a"}, {"~a | b | a"}, {"a | ~b | a"}, {"a | b | ~a"}, {"~a | ~b | a"}, {"~a | b | ~a"}, {"a | ~b | ~a"}, {"~a | ~b | ~a"},
	{"a | b > a"}, {"~a | b > a"}, {"a | ~b > a"}, {"a | b > ~a"}, {"~a | ~b > a"}, {"~a | b > ~a"}, {"a | ~b > ~a"}, {"~a | ~b > ~a"},
	{"a > b | a"}, {"~a > b | a"}, {"a > ~b | a"}, {"a > b | ~a"}, {"~a > ~b | a"}, {"~a > b | ~a"}, {"a > ~b | ~a"}, {"~a > ~b | ~a"},
	{"a | b = a"}, {"~a | b = a"}, {"a | ~b = a"}, {"a | b = ~a"}, {"~a | ~b = a"}, {"~a | b = ~a"}, {"a | ~b = ~a"}, {"~a | ~b = ~a"},
	{"a = b | a"}, {"~a = b | a"}, {"a = ~b | a"}, {"a = b | ~a"}, {"~a = ~b | a"}, {"~a = b | ~a"}, {"a = ~b | ~a"}, {"~a = ~b | ~a"},
	{"a | b ^ a"}, {"~a | b ^ a"}, {"a | ~b ^ a"}, {"a | b ^ ~a"}, {"~a | ~b ^ a"}, {"~a | b ^ ~a"}, {"a | ~b ^ ~a"}, {"~a | ~b ^ ~a"},
	{"a ^ b | a"}, {"~a ^ b | a"}, {"a ^ ~b | a"}, {"a ^ b | ~a"}, {"~a ^ ~b | a"}, {"~a ^ b | ~a"}, {"a ^ ~b | ~a"}, {"~a ^ ~b | ~a"},
};

static auto extrafms = std::vector<Formula>{
	{"(ab&3 | x&a34) -> (\\T ^ x) -> (y = x) <-> (ab | cd ^ a34)"},
};

// TODO: This should be a method on Clause as soon as Clause is a real type.
static bool clause_eval(const Clause& cl, const Assignment& assign) {
	for (auto v : assign.vars()) {
		if (cl.exists(v) && cl[v] == assign[v])
			return true;
	}
	return false;
}

// TODO: This could be a method on Conjunctive which is a specialization of
// Stream<Clause>.
static bool clstream_eval(Stream<Clause>& g, const Assignment& assign) {
	bool value = true;
	for (auto cl : g)
		value &= clause_eval(cl, assign);
	return value;
}

static bool is_eqv(const Formula& f, CNF& g, std::string message = "") {
	bool is_ok = true;
	Assignment assign;

	if (message.empty())
		message = f.to_postfix();

	auto h = g.cache();
	assign = f.assignment();
	while (!assign.overflown()) {
		is_ok &= f.eval(assign) == clstream_eval(h, assign);
		if (!is_ok)
			break;
		++assign;
	}

	if (!ok(is_ok, message)) {
		diag("mismatched at assignment ", assign);
		diag("  Got:      ", clstream_eval(h, assign));
		diag("  Expected: ", f.eval(assign));
		diag("CNF clauses:");
		for (auto cl : h)
			diag("  ", cl);
	}
	return is_ok;
}

static bool is_eqv(const Formula& f, Tseitin& g, std::string message = "") {
	bool is_ok = true;
	bool consistent, expected;
	Assignment assign, lassign;

	if (message.empty())
		message = f.to_postfix();

	auto h = g.cache();

	/* Go over all assignments on the Tseitin domain. If the assignment is
	 * consistent (the lift of its projection), then it must equal the value
	 * of the original formula. Otherwise it must be false. */
	lassign = Assignment(g.get_domain()->list());
	while (!lassign.overflown()) {
		assign = g.project(lassign);
		consistent = g.lift(assign) == lassign;
		expected = consistent ? f.eval(assign) : false;
		is_ok &= clstream_eval(h, lassign) == expected;
		if (!is_ok)
			break;
		++lassign;
	}

	if (!ok(is_ok, message)) {
		if (consistent)
			diag("mismatched at consistent assignment ", assign);
		else
			diag("mismatched at inconsistent assignment ", lassign);
		diag("  Got:      ", clstream_eval(h, lassign));
		diag("  Expected: ", expected);
		diag("Tseitin clauses:");
		for (auto cl : h)
			diag("  ", cl);
	}
	return is_ok;
}

int main(void) {
	plan(11);

	std::cout << std::boolalpha;

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
		diag(fm.eval(Assignment({{ y, false }})) ? true : false);
	}, "eval over undefined variable throws");

	is(Formula("\\T").truthtable().cache().size(), 1,
		"\\T has 1 row in truthtable");
	is(Formula("\\F").truthtable().cache().size(), 1,
		"\\F has 1 row in truthtable");
	is(Formula("~a").truthtable().cache().size(), 2,
		"~a has 2 rows in truthtable");
	is(Formula("(a|b)^(a>c)=(~a&(a|b|x))").truthtable().cache().size(), 16,
		"(a|b)^(a>c)=(~a&(a|b|x)) has 16 row in truthtable");

	SUBTEST("truthtable") {
		plan(std::size(ttfms));
		unsigned int idx = 0;
		for (auto& f : ttfms) {
			is_truthtable(f, ttvals[idx++]);
		}
	}

	SUBTEST("cnf") {
		plan(std::size(testfms));
		for (auto& f : testfms) {
			CNF cnf = f.cnf();
			is_eqv(f, cnf);
		}
	}

	SUBTEST("tseitin") {
		plan(std::size(testfms));
		for (auto& f : testfms) {
			Tseitin tsei = f.tseitin();
			is_eqv(f, tsei);
		}
	}

	return EXIT_SUCCESS;
}
