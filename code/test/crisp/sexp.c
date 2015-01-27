#include "sexp.h"
#include "syscall.h"

/* JD: '..' include isn't very nice
 * maybe use real include dir
 */
#include "../malloc_stub.h"

/* sexp_free:
 * Free the whole S-Expression recursively */
void sexp_free(struct sexp *sexp)
{
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


/* Our libc does not implements string functions yet */
static int strlen(const char *s)
{
	int i = 0;
	while(s[i]) i++;
	return i;
}

static char *strdup(char *str)
{
	char *ret;
	int len, i;

	len = strlen(str);
	ret = malloc((len+1) * sizeof(char));

	for(i = 0; i < len; i++) ret[i] = str[i];
	ret[i] = '\0';

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
