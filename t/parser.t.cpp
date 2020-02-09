#include <tappp/tappp.hpp>
#include <propcalc/propcalc.hpp>

using namespace TAP;

static std::string parse_to_postfix(const std::string& fm) {
	return Propcalc::Formula(fm).to_postfix();
}

static void is_postfix(const std::string& fm, const std::string& postfix) {
	is(parse_to_postfix(fm), postfix, fm);
}

static void is_postfix(const std::string& fm, const std::string& postfix, const std::string& message) {
	is(parse_to_postfix(fm), postfix, message);
}

static void throws_parser(const std::string& fm, const std::string& what) {
	bool is_ok = throws_like<Propcalc::X::Formula::Parser>([&] {
		(void) parse_to_postfix(fm);
	}, what, what + " " + fm);
	if (!is_ok)
		diag(parse_to_postfix(fm));
}

static void lives_parser(const std::string& fm, const std::string& message) {
	lives([&] { (void) parse_to_postfix(fm); }, message);
}

int main(void) {
	plan(4);

	SUBTEST(6, "basics") {
		is_postfix("~a", "[a] ~");
		is_postfix("~~a", "[a] ~ ~");
		is_postfix("~~~a", "[a] ~ ~ ~");
		is_postfix("  ~~  ~a", "[a] ~ ~ ~", "whitespace is insignificant");
		is_postfix("~a&b", "[a] ~ [b] &");
		is_postfix("~(a&b)", "[a] [b] & ~");
	}

	SUBTEST(20, "exceptions") {
		throws_parser("  ", "Empty formula");
		throws_parser("~a + b", "Unrecognized token");
		throws_parser("a?", "Unrecognized token");
		throws_parser("?a", "Unrecognized token");
		throws_parser("~~a&", "Missing operands");
		throws_parser("~", "Missing operands");
		throws_parser("&", "Missing operands");
		throws_parser("a&b&c&", "Missing operands");
		TODO("unary ~ attaches as postfix operator if needed?");
		throws_parser("a&b&c&d~", "Missing operands");
		throws_parser("a&b&c&d&~", "Missing operands");
		throws_parser("~a&()", "Missing operands");
		throws_parser("()", "Empty formula");
		throws_parser(")", "Missing opening paren.*");
		throws_parser("~a&x)", "Missing opening paren.*");
		throws_parser("(~a)&x)", "Missing opening paren.*");
		throws_parser("~a&x3 a", "Excess operands.*");
		lives_parser( "~a&x3a", "long variable token");
		throws_parser("(~a&x)(3)(a)", "Excess operands.*");
		throws_parser("(~a&x)(3&a)", "Excess operands.*");
		TODO("parentheses do not enclose a term");
		throws_parser("a (= b)", "");
	}

	SUBTEST(13, "associativity & precedence") {
		is_postfix("a & b & c",     "[a] [b] [c] & &");
		is_postfix("a | b | c",     "[a] [b] [c] | |");
		is_postfix("a -> b -> c",   "[a] [b] [c] > >");
		is_postfix("a <-> b <-> c", "[a] [b] [c] = =");
		is_postfix("a ^ b ^ c",     "[a] [b] [c] ^ ^");

		is_postfix("~a & b", "[a] ~ [b] &");
		is_postfix("a & b | c", "[a] [b] & [c] |");
		is_postfix("a > b | c", "[a] [b] [c] | >");
		is_postfix("a = b ^ c", "[a] [b] [c] ^ =");
		is_postfix("a ^ b = c", "[a] [b] [c] = ^");
		is_postfix("~a & b ^ ~c = d", "[a] ~ [b] & [c] ~ [d] = ^");

		is_postfix("a -> b = c -> a", "[a] [b] > [c] [a] > =");
		is_postfix("a ^ b > c ^ a", "[a] [b] [c] > [a] ^ ^");
	}

	std::string fmstr = "(ab&3 | x&a34) -> (\\T ^ x) -> (y = x) <-> (ab | cd ^ a34)";
	SUBTEST(5, "properties of " + fmstr) {
		Propcalc::Formula fm(fmstr);
		is(fm.to_postfix(), "[ab] [3] & [x] [a34] & | \\T [x] ^ [y] [x] = > > [ab] [cd] | [a34] ^ =", "postfix");
		is(fm.to_prefix(),  "= > | & [ab] [3] & [x] [a34] > ^ \\T [x] = [y] [x] ^ | [ab] [cd] [a34]", "prefix");
		is(fm.to_infix(),   "[ab] & [3] | [x] & [a34] > (\\T ^ [x]) > ([y] = [x]) = [ab] | [cd] ^ [a34]", "infix");

		is(fm.get_domain(), Propcalc::Formula::DefaultDomain, "parser defaults to DefaultDomain");
		is(fm.vars().size(), 6, "correct variable count");
	}

	return EXIT_SUCCESS;
}
