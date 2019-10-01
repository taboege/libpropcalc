#include <stdio.h>
#include <stdlib.h>

#include <propcalc/propcalc.h>

int main(int argc, char **argv) {
	if (argc < 2) {
		fprintf(stderr, "no formula given\n");
		return 1;
	}

	propform_t fm  = propcalc_formula_new(argv[1]);
	propform_t fm1 = propcalc_formula_new("[12|]&[12|3]");
	propform_t fm2 = propcalc_formula_new("[13|]|[23|]");

	char *str = propcalc_formula_rpn(fm);
	printf("%s\n", str);
	free(str);

	str = propcalc_formula_pn(fm);
	printf("%s\n", str);
	free(str);

	propform_t tmp1 = propcalc_formula_not(fm);
	propform_t tmp2 = propcalc_formula_impl(fm1, fm2);
	propform_t tmp3 = propcalc_formula_and(tmp1, tmp2);

	str = propcalc_formula_rpn(tmp3);
	printf("%s\n", str);
	free(str);

	propcalc_formula_destroy(tmp1);
	propcalc_formula_destroy(tmp2);
	propcalc_formula_destroy(tmp3);

	propcalc_formula_destroy(fm);
	propcalc_formula_destroy(fm1);
	propcalc_formula_destroy(fm2);
	return 0;
}
