# libpropcalc - Propositional calculus package

`libpropcalc` aims to become an all-in-one library for propositional calculus.
Written in C++ with an `external "C"` wrapper, it can be accessed from many
languages. My personal goal is to have a convenient library to build on top
of in Perl.

## Milestones

**v0.1:** "Ground work"
- [X] Formula parsing and printing
- [X] Evaluation and truth tables
- [X] CNF conversion
- [X] DIMACS import/export
- [ ] Documentation and tests

**v0.2:** "SAT solvers"
- [ ] SAT, #SAT and AllSAT solvers
- [ ] Documentation and tests

**v0.3:** "Knowledge compilers"
- [ ] BDD, ZDD, d-DNNF
- [ ] Documentation and tests

## Description

To come.

Note that in according with semantic versioning, the public interface
of this library is considered unstable until the major version rises
above zero.

## Please help me!

The implementation language of this library is C++. I picked it for the
following reasons:

- potentially fast,
- sufficiently close to C to be FFIable,
- JIT compiler library available,
- plenty of SAT solvers and other 3rd party code available to incorporate.

The problem is I have programmed in C, but never in C++ before, and I recognize
it's a complex language that wants to be well understood, and where best
practices change over time. Since I'm under time constraints, this library
is basically written in C + exceptions + containers + RAII, as far as my
skills go. Patches that make it into real C++17 **very** welcome!
