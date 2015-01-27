#include <stdlib.h>
#include <string.h>

#include "sexp.h"
#include "syscall.h"


/* sexp_free:
 * Free the whole S-Expression recursively */
void sexp_free(struct sexp *sexp)
{
	if(sexp == NULL)
	{
		return;
	}

	if(sexp->type == SEXP_CONS)
	{
		sexp_free(sexp->cons.car);
		sexp_free(sexp->cons.cdr);
	}
	else if(sexp->type == SEXP_ATOM_STR)
	{
		free(sexp->atom_str);
	}
	else if(sexp->type == SEXP_ATOM_SYM)
	{
		free(sexp->atom_sym);
	}

	free(sexp);
}


/* sexp_make_*:
 * Bunch of make functions to make allocation easier
 * It might seem not the best way to do it, but this is
 * actually good
 */
struct sexp *sexp_make_cons(struct sexp *car, struct sexp *cdr)
{
	struct sexp *ret = malloc(sizeof(struct sexp));

	if (ret != NULL)
	{
		ret->type = SEXP_CONS;
		ret->cons.car = car;
		ret->cons.cdr = cdr;
	}

	return ret;
}

struct sexp *sexp_make_int(int val)
{
	struct sexp *ret = malloc(sizeof(struct sexp));

	if (ret != NULL)
	{
		ret->type = SEXP_ATOM_INT;
		ret->atom_int = val;
	}

	return ret;
}

struct sexp *sexp_make_char(char val)
{
	struct sexp *ret = malloc(sizeof(struct sexp));

	if (ret != NULL)
	{
		ret->type = SEXP_ATOM_CHAR;
		ret->atom_char = val;
	}

	return ret;
}

struct sexp *sexp_make_str(char *val)
{
	struct sexp *ret = malloc(sizeof(struct sexp));

	if (ret != NULL)
	{
		ret->type = SEXP_ATOM_STR;
		ret->atom_str = strdup(val);
	}

	return ret;
}


struct sexp *sexp_make_sym(char *val)
{
	struct sexp *ret = malloc(sizeof(struct sexp));

	if (ret != NULL)
	{
		ret->type = SEXP_ATOM_SYM;
		ret->atom_sym = strdup(val);
	}

	return ret;
}

void sexp_print(struct sexp *sexp)
{
	if(sexp == NULL)
	{
		PutString("Nil");
	}
	else
	{
		switch (sexp->type)
		{
		case SEXP_CONS:
			PutString("Cons(");
			sexp_print(sexp->cons.car);
			PutString(", ");
			sexp_print(sexp->cons.cdr);
			PutString(")");
			break;
		case SEXP_ATOM_INT:
			PutString("Int(");
		    PutInt(sexp->atom_int);
			PutString(")");
			break;
		case SEXP_ATOM_CHAR:
			PutString("Char(");
		    PutChar(sexp->atom_char);
			PutString(")");
			break;
		case SEXP_ATOM_STR:
			PutString("Str(");
		    PutString(sexp->atom_str);
			PutString(")");
			break;
		case SEXP_ATOM_SYM:
			PutString("Sym(");
		    PutString(sexp->atom_sym);
			PutString(")");
			break;
		}
	}
}

struct sexp *sexp_dup(struct sexp *sexp)
{
	if(sexp != NULL)
	{
		switch (sexp->type)
		{
		case SEXP_CONS:
		    return sexp_make_cons(
				sexp_dup(sexp->cons.car),
				sexp_dup(sexp->cons.cdr)
				);
		case SEXP_ATOM_INT:
		    return sexp_make_int(sexp->atom_int);
		case SEXP_ATOM_CHAR:
			return sexp_make_char(sexp->atom_char);
		case SEXP_ATOM_STR:
		    return sexp_make_str(sexp->atom_str);
		case SEXP_ATOM_SYM:
			return sexp_make_sym(sexp->atom_sym);
		}
	}
	return NULL;
}
