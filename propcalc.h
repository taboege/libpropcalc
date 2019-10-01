/*
 * propcalc.h - Propositional calculus package, C interface
 *
 * Copyright (C) 2019 Tobias Boege
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the Artistic License 2.0
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * Artistic License 2.0 for more details.
 */

#ifndef PROPCALC_H
#define PROPCALC_H

/*
 * General
 */
extern unsigned int propcalc_version(void);

/*
 * Formula
 */

typedef void *propform_t;

extern propform_t propcalc_formula_new     (const char *fm);
extern void       propcalc_formula_destroy (propform_t fm);

extern char *     propcalc_formula_rpn     (const propform_t fm);
extern char *     propcalc_formula_pn      (const propform_t fm);

extern propform_t propcalc_formula_not     (const propform_t rhs);
extern propform_t propcalc_formula_and     (const propform_t lhs, const propform_t rhs);
extern propform_t propcalc_formula_or      (const propform_t lhs, const propform_t rhs);
extern propform_t propcalc_formula_impl    (const propform_t lhs, const propform_t rhs);
extern propform_t propcalc_formula_eqv     (const propform_t lhs, const propform_t rhs);
extern propform_t propcalc_formula_xor     (const propform_t lhs, const propform_t rhs);

#endif /* PROPCALC_H */
