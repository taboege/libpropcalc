# Formula parser

The parser accepts any well-formed formula in propositional calculus and
turns it into an AST. The notion of well-formedness is standard (constants
and variables are atomic terms, atomic terms are terms, terms can be
combined with operators, parentheses may be used). A formula may *not*
produce an empty AST.

Except inside of atomic tokens like variable names or operators, whitespace
is insignificant.

## Operators

The following well-known logical connectives are supported:

- `~`: unary negation
- `&`: conjunction
- `|`: disjunction
- `->` or `>`: implication
- `<->` or `=`: equivalence
- `^`: exclusive or

These operators are listed in order of looser precedence: `~` binds the
tighest, followed by `&` and so on. All of them *except* for `=` and `^`
have distinct precedence levels. At the lower end, `=` and `^` together
have the loosest precedence.

All binary operators except for implication are associative, implication
is by convention *right*-associative.  When parsing formulas (in infix
form), the parser assembles an AST where all chained operators associate
to the right, i.e. `a & b & c` would parse as `a & (b & c)` and produce
this AST:

         &
        / \
       /   &
      /   / \
     a   b   c

This gives a predictable shape (although it may be unwise to rely on it)
without changing the conventional meaning of the infix expression.

## Variables and constants

Variables are strings that begin with an alphanumeric character and consist
entirely of alphanumerics or underscores. It is legal to make a variable
called `3` or `3_4` or `xyz` or `a25`. If more complicated/free-form variable
names are desired, place them inside square brackets. After an opening
square bracket, anything ASCII goes, until the closing square bracket.
So `[_]`, `[12|47]`, `[Once upon a Time...]` are also legal variables.

The constants *true* and *false* can be encoded as `\T` and `\F`.
Mind the leading backslash to distinguish them from variables.

## Compatibility with sagemath's propcalc

This library is developed as a replacement for the propcalc package of
sagemath, but even the formula parser is not 100% compatible. All the
operator symbols of sagemath-propcalc are supported, their variable notation
is supported as well. However, the implementations of `to_{pre,post,in}fix`
prefer the non-sagemath variants of operators and variables.

A deeper issue concerns precedence and associativity. In sage's propcalc,
all binary operators are *left*-associative. This means that AST will
almost always be different between them and us. Since there is one
non-associative connective, implication, this also means that the
semantics of formulas are incompatible.

The precedence list of libpropcalc is

> `~`  >  `&`  >  `|`  >  `->`  >  `<->` = `^`

whereas sagemath's is

> `~`  >  `&`  >  `|`  >  `^`  > `->`  >  `<->`

As far as I am aware, there is no authoritative source on the precedence
and associativity of these connectives (although implication has always
been right-associative when I encountered it). The low precedence of `^`
probably makes sense if you think of it as "addition mod 2". In libpropcalc,
its precedence is justified by being "not-equal".

Please pay attention to these differences when porting code.
