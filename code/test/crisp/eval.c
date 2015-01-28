#include <stddef.h>
#include <string.h>
#include "eval.h"
#include "symbol_table.h"
#include "syscall.h"


typedef struct sexp *(*eval_func)(struct sexp *sexp, struct symbol_table *st);

struct symbol
{
	const char *name;
    eval_func func;
};

#define ERROR (void *)-1

struct sexp *eval_add(struct sexp *sexp, struct symbol_table *st)
{
	int result = 0;
	struct sexp *cur = sexp;
	struct sexp *arg;

	while(cur != NULL)
	{
		if (cur->type != SEXP_CONS)
		{
			PutString("error: + arguments are not a valid list\n");
			return ERROR;
		}

		arg = eval(cur->cons.car, st);
		if(arg->type != SEXP_ATOM_INT)
		{
			/* Temporary error message */
			PutString("Error: + needs number as argument\n");
			sexp_free(arg);
			return ERROR;
		}

		result += arg->atom_int;
		cur = cur->cons.cdr;
		sexp_free(arg);
	}
	return sexp_make_int(result);
}

struct sexp *eval_minus(struct sexp *sexp, struct symbol_table *st)
{
	int result = 0;
	int first = 1;
	struct sexp *cur = sexp;
	struct sexp *arg;

	while(cur != NULL)
	{
		if (cur->type != SEXP_CONS)
		{
			PutString("Error: (+) arguments are not a valid list\n");
			return ERROR;
		}

		arg = eval(cur->cons.car, st);
		if(arg->type != SEXP_ATOM_INT)
		{
			/* Temporary error message */
			PutString("Error: (+) needs number as argument\n");
			sexp_free(arg);
			return ERROR;
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

struct sexp *eval_quote(struct sexp *sexp, struct symbol_table *st)
{
	if (sexp->type != SEXP_CONS)
	{
		PutString("Error: (quote) no arguments\n");
		return ERROR;
	}

	return sexp_dup(sexp->cons.car);
}

struct sexp *eval_car(struct sexp *sexp, struct symbol_table *st)
{
	struct sexp *list;
	struct sexp *res;

    if (sexp->type != SEXP_CONS)
	{
		PutString("Error: (car) no arguments\n");
		return ERROR;
	}

	list = eval(sexp->cons.car, st);

	if (list->type != SEXP_CONS)
	{
		PutString("TypeError: (car) expect a list argument\n");
		return ERROR;
	}

	res = sexp_dup(list->cons.car);
	sexp_free(list);
	return res;
}

struct sexp *eval_cdr(struct sexp *sexp, struct symbol_table *st)
{
	struct sexp *list;
	struct sexp *res;

    if (sexp->type != SEXP_CONS)
	{
		PutString("Error: (cdr) no arguments\n");
		return ERROR;
	}

	list = eval(sexp->cons.car, st);

	if (list->type != SEXP_CONS)
	{
		PutString("TypeError: (cdr) expect a list argument\n");
		return ERROR;
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

struct sexp *eval_cons(struct sexp *sexp, struct symbol_table *st)
{
	int arg_count = list_len(sexp);

	if(arg_count == -1)
	{
		PutString("FormatError: (cons) arguments are invalid\n");
		return ERROR;
	}
	else if(arg_count != 2)
	{
		PutString("Error: (cons) expect exactly 2 arguments (");
		PutInt(arg_count);
		PutString(" given)\n");
		return ERROR;
	}

	return sexp_make_cons(
		eval(sexp->cons.car, st),
		eval(sexp->cons.cdr->cons.car, st)
		);
}

struct sexp *eval_define(struct sexp *sexp, struct symbol_table *st)
{
	struct sexp *arg;
	struct sexp *target;

	if (sexp->type != SEXP_CONS)
	{
		PutString("FormatError: (define) no arguments\n");
		return ERROR;
	}

	arg = sexp->cons.car;

	if (!arg || arg->type != SEXP_ATOM_SYM)
	{
		PutString("Error: (define) symbol expected\n");
		return ERROR;
	}

	if (sexp->cons.cdr->type != SEXP_CONS)
	{
		PutString("FormatError: (define) arguments a incorrectly formated\n");
		return ERROR;
	}

	target = eval(sexp->cons.cdr->cons.car, st);
	if (symbol_table_set(st, arg->atom_sym, target) != 0)
	{
		PutString("Error: (define) could not set symbol: \"");
		PutString(arg->atom_sym);
		PutString("\"\n");
		sexp_free(target);
		return ERROR;
	}
	sexp_free(target);
	return NULL;
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
	{"define", eval_define },
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

struct sexp *eval(struct sexp *sexp, struct symbol_table *st)
{
	if (sexp == NULL)
		return NULL;

	if (sexp->type == SEXP_CONS)
	{
		struct sexp *car = sexp->cons.car;
		if (car && car->type == SEXP_ATOM_SYM)
		{
			eval_func func = get_builtin(car->atom_sym);

			if(func != NULL)
			{
				return func(sexp->cons.cdr, st);
			}
		}

		PutString("Error: ");
		sexp_print(sexp->cons.car);
		PutString(" is not a function.\n");
		return ERROR;
	}
	else if(sexp->type == SEXP_ATOM_SYM)
	{
		struct sexp *sym_sexp = symbol_table_get(st, sexp->atom_sym);
		if (sym_sexp == (void*)-1)
		{
			PutString("Error: unbound symbol: \"");
			PutString(sexp->atom_sym);
			PutString("\"\n");
			return ERROR;
		}
		return eval(sym_sexp, st);
	}
	else
	{
		return sexp_dup(sexp);
	}
}
