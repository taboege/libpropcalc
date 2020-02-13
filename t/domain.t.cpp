#include <tappp/tappp.hpp>
#include <propcalc/propcalc.hpp>

#include <cstdlib>

using namespace TAP;
using namespace Propcalc;

int main(void) {
	plan(8);

	Cache temp;
	VarRef v3, v3_4, v_, vonce;

	SUBTEST(4, "add some variables") {
		lives([&] { v3 = temp.resolve("3"); }, "new variable '3'");
		lives([&] { v3_4 = temp.resolve("3_4"); }, "new variable '3_4'");
		lives([&] { v_ = temp.resolve("_"); }, "new variable '_'");
		lives([&] { vonce = temp.resolve("Once upon a Time..."); }, "new variable 'Once upon a Time...");
	}

	SUBTEST(7, "add more variables") {
		is(temp.resolve("3")->name, "3", "3");
		is(temp.resolve("3_4")->name, "3_4", "3_4");
		is(temp.resolve("xyz")->name, "xyz", "xyz");
		is(temp.resolve("a25")->name, "a25", "a25");
		is(temp.resolve("_")->name, "_", "_");
		is(temp.resolve("12|47")->name, "12|47", "12|47");
		is(temp.resolve("Once upon a Time...")->name, "Once upon a Time...", "Once upon a Time...");
	}

	is(temp.size(), 7, "size of cache is 7");

	SUBTEST(4, "consistent answers") {
		is(v3, temp.resolve("3"), "3");
		is(v3_4, temp.resolve("3_4"), "3_4");
		is(v_, temp.resolve("_"), "_");
		is(vonce, temp.resolve("Once upon a Time..."), "Once upon a Time...");
	}

	SUBTEST(6, "autovivification, freeze and thaw") {
		temp.freeze();
		throws<Propcalc::X::Cache::Frozen>([&] {
			temp.resolve("a");
		}, "new variable not allowed when frozen");
		throws<Propcalc::X::Cache::Frozen>([&] {
			temp.resolve("Once upon a time...");
		}, "case matters");
		throws<Propcalc::X::Cache::Frozen>([&] {
			temp.resolve("[xyz]");
		}, "brackets matters");
		lives([&] {
			temp.resolve("xyz");
		}, "known variable is fine 1/3");
		lives([&] {
			temp.resolve("Once upon a Time...");
		}, "known variable is fine 2/3");
		lives([&] {
			temp.resolve("_");
		}, "known variable is fine 3/3");
		temp.thaw();
	}

	is(temp.size(), 7, "size of cache is still 7");

	SUBTEST(29, "pack and unpack") {
		is(temp.pack(v3),    1, "3 is 1");
		is(temp.pack(v3_4),  2, "3_4 is 2");
		is(temp.pack(v_),    3, "_ is 3");
		is(temp.pack(vonce), 4, "Once upon a Time... is 4");

		is(temp.pack(temp.resolve("12|47")), 7, "12|47 is 7");
		is(temp.pack(temp.resolve("a25")), 6, "a25 is 6");

		is(temp.unpack(1), v3, "1 is 3");
		is(temp.unpack(2), v3_4, "2 is 3_4");
		is(temp.unpack(3), v_, "3 is _");
		is(temp.unpack(4), vonce, "4 is Once upon a Time...");

		is(temp.unpack(7), temp.resolve("12|47"), "7 is 12|47");
		is(temp.unpack(5), temp.resolve("xyz"), "5 is xyz");

		lives([&] { temp.unpack(12); }, "unpack of large number succeeds");
		is(temp.size(), 12, "unpack autovivifies");

		temp.freeze();
		is(temp.unpack(12)->name, "12", "12 is 12");
		is(temp.unpack(11)->name, "11", "11 is 11");
		is(temp.unpack(10)->name, "10", "10 is 10");
		is(temp.unpack( 9)->name,  "9",  "9 is 9");
		is(temp.unpack( 8)->name,  "8",  "8 is 8");
		is(temp.unpack( 7)->name,  "12|47", "7 is 12|47");
		is(temp.unpack( 6)->name,  "a25", "6 is a25");
		is(temp.unpack( 5)->name,  "xyz",  "5 is xyz");
		is(temp.unpack( 4)->name,  "Once upon a Time...",  "4 is Once upon a Time...");
		is(temp.unpack( 3)->name,  "_",  "3 is _");
		is(temp.unpack( 2)->name,  "3_4",  "2 is 3_4");
		is(temp.unpack( 1)->name,  "3",  "1 is 3");
		throws<Propcalc::X::Domain::InvalidVarNr>([&] {
			temp.unpack(0);
		}, "0 is not a VarNr");

		is(temp.resolve("11")->name, "11", "resolved 11");

		throws<Propcalc::X::Cache::Frozen>([&] {
			temp.unpack(13);
		}, "unpack fails to autovivify if frozen");
		temp.thaw();
	}

	is(temp.size(), 12, "size of cache is 12 now");

	return EXIT_SUCCESS;
}
