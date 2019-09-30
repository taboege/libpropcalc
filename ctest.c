#include <stdio.h>
#include <stdlib.h>

#include <propcalc/propcalc.h>

int main(int argc, char **argv) {
	if (argc < 2) {
		fprintf(stderr, "no formula given\n");
		return 1;
	}

	propform_t fm = propcalc_formula_new(argv[1]);

	char *str = propcalc_formula_rpn(fm);
	printf("%s\n", str);
	free(str);

	str = propcalc_formula_pn(fm);
	printf("%s\n", str);
	free(str);

	propcalc_formula_destroy(fm);
	return 0;
}
