#ifndef BUILTIN_H_
#define BUILTIN_H_

#include "eval.h"

struct sexp *eval_add(struct sexp *sexp, struct symbol_table *st);
struct sexp *eval_minus(struct sexp *sexp, struct symbol_table *st);
struct sexp *eval_mult(struct sexp *sexp, struct symbol_table *st);
struct sexp *eval_equal(struct sexp *sexp, struct symbol_table *st);
struct sexp *eval_greater(struct sexp *sexp, struct symbol_table *st);
struct sexp *eval_quote(struct sexp *sexp, struct symbol_table *st);
struct sexp *eval_car(struct sexp *sexp, struct symbol_table *st);
struct sexp *eval_cdr(struct sexp *sexp, struct symbol_table *st);
struct sexp *eval_cons(struct sexp *sexp, struct symbol_table *st);
struct sexp *eval_define(struct sexp *sexp, struct symbol_table *st);
struct sexp *eval_lambda(struct sexp *sexp, struct symbol_table *st);
struct sexp *eval_if(struct sexp *sexp, struct symbol_table *st);

#endif /* BUILTIN_H_ */
