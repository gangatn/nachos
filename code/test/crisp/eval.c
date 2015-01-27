#include <stddef.h>
#include <string.h>
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
	struct sexp *arg;

	while(cur != NULL)
	{
		if (cur->type != SEXP_CONS)
		{
			PutString("error: + arguments are not a valid list\n");
			return NULL;
		}

		arg = eval(cur->cons.car);
		if(arg->type != SEXP_ATOM_INT)
		{
			/* Temporary error message */
			PutString("error: + needs number as argument\n");
			sexp_free(arg);
			return NULL;
		}

		result += arg->atom_int;
		cur = cur->cons.cdr;
		sexp_free(arg);
	}
	return sexp_make_int(result);
}

struct sexp *eval_minus(struct sexp *sexp)
{
	int result = 0;
	int first = 1;
	struct sexp *cur = sexp;
	struct sexp *arg;

	while(cur != NULL)
	{
		if (cur->type != SEXP_CONS)
		{
			PutString("error: + arguments are not a valid list\n");
			return NULL;
		}

		arg = eval(cur->cons.car);
		if(arg->type != SEXP_ATOM_INT)
		{
			/* Temporary error message */
			PutString("error: + needs number as argument\n");
			sexp_free(arg);
			return NULL;
		}

		if (first)
		{
			result += arg->atom_int;
			first = 0;
		}
		else
			result -= arg->atom_int;
		cur = cur->cons.cdr;
		sexp_free(arg);
	}
	return sexp_make_int(result);
}

struct sexp *eval_quote(struct sexp *sexp)
{
	if (sexp->type != SEXP_CONS)
	{
		PutString("error: (quote) argument must be a list\n");
		return NULL;
	}

	return sexp_dup(sexp->cons.car);
}

/*
 * We don't use the symbol table for builtins
 * This is only for convenience, but this may be changed later
 *
 * Comparison:
 * Pros:
 * - We only deal with sexp in symbol table, and not native function pointers
 * - We don't have to init the symbol table with builtins
 *   Which is really convenient: the interpreter is ready to go
 *
 * Cons:
 * - the evaluation can be slightly longer,
 *   indeed we start by checking if it's builtins
 *   and then check the symbol table
 *
 * Additional note:
 * Builtins are using linear table, so the cost is O(n) to find the eval
 * function where n is the count of builtins
 */
struct symbol builtins[] =
{
	{"+", eval_add}
};

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
