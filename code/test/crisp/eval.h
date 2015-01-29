#ifndef EVAL_H_
#define EVAL_H_

#include "sexp.h"
#include "symbol_table.h"

/*
 * evaluate the S-Expression
 *
 * returns (void*)-1 on error
 */
struct sexp *eval(struct sexp *sexp, struct symbol_table *st);

#define FUNC_SYMBOL_NAME "lambda"

#endif /* EVAL_H_ */
