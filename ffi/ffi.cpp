#include <cstring>

#include <propcalc/propcalc.hpp>

extern "C" {

#include <propcalc/propcalc.h>

/*
 * TODO: Error handling/conversion from exception to error code?
 */

unsigned int propcalc_version(void) {
	return PROPCALC_VERSION;
}

#define REFORM(x)		static_cast<propform_t>(x)
#define DEFORM(x)		static_cast<Propcalc::Formula*>(x)
#define NEWREFORM(x)	REFORM(new Propcalc::Formula(x))

propform_t propcalc_formula_new(const char *fm) {
	return REFORM(new Propcalc::Formula(std::string(fm)));
}

void propcalc_formula_destroy(propform_t fm) {
	delete DEFORM(fm);
}

char *propcalc_formula_rpn(const propform_t fm) {
	return strdup(DEFORM(fm)->to_rpn().c_str());
}

char *propcalc_formula_pn(const propform_t fm) {
	return strdup(DEFORM(fm)->to_pn().c_str());
}

propform_t propcalc_formula_neg(const propform_t rhs) {
	return NEWREFORM(~*DEFORM(rhs));
}

propform_t propcalc_formula_and(const propform_t lhs, const propform_t rhs) {
	return NEWREFORM(*DEFORM(lhs) & *DEFORM(rhs));
}

propform_t propcalc_formula_or(const propform_t lhs, const propform_t rhs) {
	return NEWREFORM(*DEFORM(lhs) | *DEFORM(rhs));
}

propform_t propcalc_formula_impl(const propform_t lhs, const propform_t rhs) {
	return NEWREFORM(*DEFORM(lhs) >> *DEFORM(rhs));
}

propform_t propcalc_formula_eqv(const propform_t lhs, const propform_t rhs) {
	return NEWREFORM(*DEFORM(lhs) == *DEFORM(rhs));
}

propform_t propcalc_formula_xor(const propform_t lhs, const propform_t rhs) {
	return NEWREFORM(*DEFORM(lhs) ^ *DEFORM(rhs));
}

} /* extern "C" */
