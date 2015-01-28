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
		PutString("Error: (quote) no arguments\n");
		return NULL;
	}

	return sexp_dup(sexp->cons.car);
}

struct sexp *eval_car(struct sexp *sexp)
{
	struct sexp *list;
	struct sexp *res;

    if (sexp->type != SEXP_CONS)
	{
		PutString("Error: (car) no arguments\n");
		return NULL;
	}

	list = eval(sexp->cons.car);

	if (list->type != SEXP_CONS)
	{
		PutString("TypeError: (car) expect a list argument\n");
		return NULL;
	}

	res = sexp_dup(list->cons.car);
	sexp_free(list);
	return res;
}

struct sexp *eval_cdr(struct sexp *sexp)
{
	struct sexp *list;
	struct sexp *res;

    if (sexp->type != SEXP_CONS)
	{
		PutString("Error: (cdr) no arguments\n");
		return NULL;
	}

	list = eval(sexp->cons.car);

	if (list->type != SEXP_CONS)
	{
		PutString("TypeError: (cdr) expect a list argument\n");
		return NULL;
	}

	res = sexp_dup(list->cons.cdr);
	sexp_free(list);
	return res;
}

static int list_len(struct sexp *sexp)
{
	struct sexp *cur = sexp;
	int i = 0;

	while(cur)
	{
		if(cur->type != SEXP_CONS)
		{
			/* Invalid list */
			return -1;
		}
		i++;
		cur = cur->cons.cdr;
	}
	return i;
}

struct sexp *eval_cons(struct sexp *sexp)
{
	int arg_count = list_len(sexp);

	if(arg_count == -1)
	{
		PutString("FormatError: (cons) arguments are invalid\n");
		return NULL;
	}
	else if(arg_count != 2)
	{
		PutString("Error: (cons) expect exactly 2 arguments (");
		PutInt(arg_count);
		PutString(" given)\n");
		return NULL;
	}

	return sexp_make_cons(
		eval(sexp->cons.car),
		eval(sexp->cons.cdr->cons.car)
		);
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
	{"+", eval_add },
	{"-", eval_minus },
	{"quote", eval_quote },
	{"car", eval_car },
	{"cdr", eval_cdr },
	{"cons", eval_cons },
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
		return sexp_dup(sexp);
	}
}
