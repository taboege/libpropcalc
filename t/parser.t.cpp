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

static void throws_parser(const std::string& fm, const std::string& what, unsigned int where = 0) {
	std::string message = what + " ";
	if (where > 0)
		message += "at pos. " + std::to_string(where) + ": ";
	message += fm;

	SUBTEST(message) {
		std::string postfix = "";
		try {
			postfix = parse_to_postfix(fm);
			fail("code succeeded");
			diag("Postfix: " + postfix);
		}
		catch (const Propcalc::X::Formula::Parser& e) {
			bool is_ok = like(e.what(), what, "reason matches");
			if (where > 0)
				is_ok &= is_ok && is(1 + e.offset, where, "position matches");
			if (!is_ok) {
				diag("Input: " + fm);
				diag("     " + std::string(e.offset, ' ') + "--^");
			}
		}
		catch (...) {
			fail("different exception occurred");
		}
		done_testing();
	}
}

static void lives_parser(const std::string& fm, const std::string& message) {
	lives([&] { (void) parse_to_postfix(fm); }, message);
}

static void infix_roundtrips(const std::string& fm) {
	std::string infix = Propcalc::Formula(fm).to_infix();
	is(infix, Propcalc::Formula(infix).to_infix(), fm);
}

int main(void) {
	plan(5);

	SUBTEST(6, "basics") {
		is_postfix("~a", "[a] ~");
		is_postfix("~~a", "[a] ~ ~");
		is_postfix("~~~a", "[a] ~ ~ ~");
		is_postfix("  ~~  ~a", "[a] ~ ~ ~", "whitespace is insignificant");
		is_postfix("~a&b", "[a] ~ [b] &");
		is_postfix("~(a&b)", "[a] [b] & ~");
	}

	SUBTEST(28, "exceptions") {
		throws_parser("  ", "Term expected.*");
		throws_parser("~a + b", "Unrecognized token", 4);
		throws_parser("a?", "Unrecognized token", 2);
		throws_parser("?a", "Unrecognized token", 1);
		throws_parser("~", "Term expected.*", 2);
		throws_parser("a~", "Infix expected.*", 2);
		throws_parser("a&", "Term expected.*", 3);
		throws_parser("a&~", "Term expected.*", 4);
		throws_parser("a&b~", "Infix expected.*", 4);
		throws_parser("a&b~c", "Infix expected.*", 4);
		throws_parser("a b &", "Infix expected.*", 3);
		throws_parser("&", "Term expected.*", 1);
		throws_parser("a&b&c&", "Term expected.*", 7);
		throws_parser("a&b&c&d~", "Infix expected.*", 8);
		throws_parser("a&b&c&d&~", "Term expected.*", 10);
		throws_parser("~a&()", "Term expected.*", 5);
		throws_parser("~a&()b", "Term expected.*", 5);
		throws_parser("()", "Term expected.*", 2);
		throws_parser("a)", "Missing opening paren.*", 2);
		throws_parser(")", "Term expected.*", 1);
		throws_parser("~a&x)", "Missing opening paren.*", 5);
		throws_parser("(~a)&x)", "Missing opening paren.*", 7);
		throws_parser("~a&x3 a", "Infix expected.*", 7);
		lives_parser( "~a&x3a", "long variable token");
		throws_parser("(~a&x)(3)(a)", "Infix expected.*", 7);
		throws_parser("(~a&x)(3&a)", "Infix expected.*", 7);
		throws_parser("a (= b)", "Infix expected.*", 3);
		throws_parser("a (=) b", "Infix expected.*", 3);
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

	SUBTEST(10, "infix stringification roundtrips") {
		infix_roundtrips("~a & b");
		infix_roundtrips("~(a & b)");
		infix_roundtrips("(a & b) -> c = d");
		infix_roundtrips("(a & b) -> (c = d)");
		infix_roundtrips("((a & b) -> c) = d");
		infix_roundtrips("a = b ^ c = d");
		infix_roundtrips("a ^ b = c ^ d");
		infix_roundtrips("a = b > c = d");
		infix_roundtrips("a > b = c > d");
		infix_roundtrips("(ab&3 | x&a34) -> (\\T ^ x) -> (y = x) <-> (ab | cd ^ a34)");
	}

	std::string fm = "(ab&3 | x&a34) -> (\\T ^ x) -> (y = x) <-> (ab | cd ^ a34)";
	SUBTEST(5, "properties of " + fm) {
		Propcalc::Formula F(fm);
		is(F.to_postfix(), "[ab] [3] & [x] [a34] & | \\T [x] ^ [y] [x] = > > [ab] [cd] | [a34] ^ =", "postfix");
		is(F.to_prefix(),  "= > | & [ab] [3] & [x] [a34] > ^ \\T [x] = [y] [x] ^ | [ab] [cd] [a34]", "prefix");
		is(F.to_infix(),   "[ab] & [3] | [x] & [a34] > (\\T ^ [x]) > ([y] = [x]) = [ab] | [cd] ^ [a34]", "infix");

		is(F.get_domain(), Propcalc::Formula::DefaultDomain, "parser defaults to DefaultDomain");
		is(F.vars().size(), 6, "correct variable count");
	}

	return EXIT_SUCCESS;
}
