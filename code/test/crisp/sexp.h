/*
 * S-Expression Module
 * ===================
 * S-Expression (sexp) are an easy way to reprent
 * lisp like language.
 *
 * An S-Expression can either be an atom
 * or expression (X . Y) where x and y are S-Expression
 *
 * In our implementation, NULL pointers are nil
 */

#ifndef SEXP_H_
#define SEXP_H_

#define NULL (0)

enum sexp_type
{
	SEXP_CONS,
	SEXP_ATOM_INT,
	SEXP_ATOM_CHAR,
	SEXP_ATOM_STR,
	SEXP_ATOM_SYM
};

/*
 * We use the cons terminology for the good'ol way
 * of lisp family interpreters
 */
struct sexp_cons
{
	struct sexp *car;
	struct sexp *cdr;
};

/*
 * We integrate all atoms directly in the sexp
 * for convenience, we still use intermediary
 * cons structure to avoid confusions
 *
 * Note: This may change in the future
 */
struct sexp
{
	int type;
	union
	{
		struct sexp_cons cons;
	    int atom_int;
	    char atom_char;
		char *atom_str;
	    char *atom_sym;
	};
};


/* sexp_free:
 * Free the whole S-Expression recursively */
void sexp_free(struct sexp *sexp);

/* sexp_print:
 * Print the S-Expression to standard output
 * useful for debugging purpose
 */
void sexp_print(struct sexp *sexp);

/* sexp_make_*:
 * Bunch of make functions to make allocation easier
 * It might seem not the best way to do it, but this is
 * actually good
 */
struct sexp *sexp_make_cons(struct sexp *car, struct sexp *cdr);
struct sexp *sexp_make_int(int val);
struct sexp *sexp_make_char(char val);
struct sexp *sexp_make_str(char *val);
struct sexp *sexp_make_sym(char *val);


#endif /* SEXP_H_ */
