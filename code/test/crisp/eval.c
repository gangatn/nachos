#include "eval.h"
#include "syscall.h"

typedef struct sexp *(*eval_func)(struct sexp *sexp);

struct symbol
{
	const char *name;
    eval_func func;
};

struct sexp *eval_add(struct sexp *sexp)
{
	int result = 0;
	struct sexp *cur = sexp;
	struct sexp *car;

	while(cur != NULL)
	{
		car = cur->cons.car;
		if(car->type != SEXP_ATOM_INT)
		{
			/* Temporary error message */
			PutString("error: + needs number as argument\n");
			return NULL;
		}

		result += car->atom_int;
		cur = cur->cons.cdr;
	}
	return sexp_make_int(result);
}

struct symbol builtins[] =
{
	{"+", eval_add}
};

static int strcmp(const char *s1, const char *s2)
{
	for ( ; *s1 == *s2; s1++, s2++)
		if (*s1 == '\0')
			return 0;
    return ((*(unsigned char *)s1 < *(unsigned char *)s2) ? -1 : +1);
}

static eval_func get_builtin(const char *name)
{
	int i = 0;
	for(i = 0; i < (sizeof(builtins) / sizeof(struct symbol)); i++)
	{
		if(!strcmp(builtins[i].name, name))
		{
			return builtins[i].func;
		}
	}
	return NULL;
}

struct sexp *eval(struct sexp *sexp)
{
	if (sexp && sexp->type == SEXP_CONS)
	{
		struct sexp *car = sexp->cons.car;
		if (car && car->type == SEXP_ATOM_SYM)
		{
			eval_func func = get_builtin(car->atom_sym);

			if(func != NULL)
			{
				return func(sexp->cons.cdr);
			}
		}
		PutString("error: This is not a function\n");
		return NULL;
	}
	else
	{
		return sexp;
	}
}
