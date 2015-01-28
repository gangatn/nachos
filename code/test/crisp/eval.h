#ifndef EVAL_H_
#define EVAL_H_

#include "sexp.h"
#include "symbol_table.h"

struct sexp *eval(struct sexp *sexp, struct symbol_table *st);

#endif /* EVAL_H_ */
