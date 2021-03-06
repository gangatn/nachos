#ifndef PARSE_H_
#define PARSE_H_

#include "sexp.h"

/*
 * Parse an S-Expression from standard input
 *
 * returns (void*)-1 if there was a parsing error
 * TODO:
 *       - Make the lexer work for any buffer
 *       - Proper error handling
 */
struct sexp *parse(void);

#endif /* PARSE_H_ */
