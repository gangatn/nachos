#include <stddef.h>
#include <string.h>
#include "eval.h"
#include "symbol_table.h"
#include "syscall.h"

#define FUNC_SYMBOL_NAME "lambda"

typedef struct sexp *(*eval_func)(struct sexp *sexp, struct symbol_table *st);

struct symbol
{
	const char *name;
    eval_func func;
};

#define ERROR ((void *)-1)

/*
 * elem:
 * return the 'index''nt element of the list
 * NULL if there is no such index
 * -1 if the list is ill formed
 */
static struct sexp *elem(struct sexp *sexp, int index)
{
	while (index --> 0)
	{
		if (sexp->type != SEXP_CONS)
		{
			return ERROR;
		}
		sexp = sexp->cons.cdr;
	}

	if (sexp == NULL)
	{
		return NULL;
	}
	else if (sexp->type != SEXP_CONS)
	{
		return ERROR;
	}
	else
	{
		return sexp->cons.car;
	}
}

/*
 * list_len:
 * return the length of the given sexp list, -1 if it's not a list
 */
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

	if (!sexp || sexp->type != SEXP_CONS)
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

	if (sexp->cons.cdr && sexp->cons.cdr->type != SEXP_CONS)
	{
		PutString("FormatError: (define) arguments a incorrectly formated\n");
		return ERROR;
	}


	target = sexp->cons.cdr == NULL ? NULL : eval(sexp->cons.cdr->cons.car, st);

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

static int args_valid(struct sexp *sexp)
{
	int i = 0;
	struct sexp *arg;

	/*
	 * Arguments are either a list of symbols :
	 * {one two three} or one symbol
	 */

	if (sexp && sexp->type == SEXP_ATOM_SYM)
	{
		return 1;
	}

	do
	{
		/* TODO(Jeremy): Do not use elem, do it by hand, it's faster */
		arg = elem(sexp, i);
		if (arg == ERROR)
		{
			return 0;
		}

		if (arg && arg->type != SEXP_ATOM_SYM)
		{
			return 0;
		}

		i++;
	}
	while (arg != NULL);
	return 1;
}

struct sexp *eval_lambda(struct sexp *sexp, struct symbol_table *st)
{
	struct sexp *args, *body;

	args = elem(sexp, 0);
	body = elem(sexp, 1);

	if (args == ERROR || body == ERROR)
	{
		PutString("Error: (lambda) expect two arguments\n");
		return ERROR;
	}

	/* TODO(Jeremy):
	 * Scheme accept symbols as argument list, maybe should have
	 * a similar semantic
	 */
	if (!args_valid(args))
	{
		PutString("Error: (lambda) function arguments are invalid\n");
		return ERROR;
	}

	/* TODO(Jeremy): do we need to duplicate ? im not so sure */
	return sexp_make_cons(
		sexp_make_sym(FUNC_SYMBOL_NAME),
		sexp_dup(sexp)
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
	{"define", eval_define },
	{FUNC_SYMBOL_NAME, eval_lambda }
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



static int is_function(struct sexp *sexp)
{
	struct sexp *tmp;
	/* A function is a lambda term, this implies a
	 * cons construction */
	if (sexp == NULL || sexp->type != SEXP_CONS)
	{
		return 0;
	}

	tmp = elem(sexp, 0);

	/* The first element of the list must be a symbol with
	 * the name of FUNC_SYMBOL_NAME */
	if (tmp->type != SEXP_ATOM_SYM ||
		strcmp(tmp->atom_sym, FUNC_SYMBOL_NAME))
	{
		return 0;
	}

	/* Since the function has already been evaluated, the function is valid */
	return 1;
}


/* TODO(Jeremy): implements save_args and restore args
static struct entry *save_args(struct sexp *args, struct symbol_table *st)
{
	return NULL;
}
*/

static struct sexp *eval_exec_function(struct sexp *function, struct sexp *args,
								struct symbol_table *st)
{
	int fun_argc, given_argc;

	fun_argc = list_len(function->cons.car);
	given_argc = list_len(args);

	if (fun_argc != given_argc)
	{
		PutString("Error: function required ");
		PutInt(fun_argc);
		PutString(" arguments, ");
		PutInt(given_argc);
		PutString(" given.\n");
		return ERROR;
	}

	return NULL;
}

struct sexp *eval(struct sexp *sexp, struct symbol_table *st)
{
	if (sexp == NULL)
		return NULL;

	if (sexp->type == SEXP_CONS)
	{
		/*
		 * If we deal with a list, we have to evaluate the list,
		 * we start by evaluate the first element, and if it's a builtin
		 * or a function we apply it, otherwise we return an error
		 */
		struct sexp *sexp_fun = eval(sexp->cons.car, st);

		if (sexp_fun && sexp_fun->type == SEXP_ATOM_SYM)
		{
			eval_func func = get_builtin(sexp_fun->atom_sym);

			if(func != NULL)
			{
				return func(sexp->cons.cdr, st);
			}
			else
			{
				/* The symbol is not builtin, we get the associated sexp of the symbol */
				struct sexp *symval = symbol_table_get(st, sexp_fun->atom_sym);
				sexp_free(sexp_fun);
				sexp_fun = symval;
			}
		}

		if (is_function(sexp_fun))
		{
			return eval_exec_function(sexp_fun->cons.cdr, sexp->cons.cdr, st);
		}
		else
		{
			PutString("Error: ");
			sexp_print(sexp_fun);
			PutString(" is not a function.\n");
		}
		return ERROR;
	}
	else if(sexp->type == SEXP_ATOM_SYM)
	{
		struct sexp *sym_sexp;

		if (get_builtin(sexp->atom_sym) != NULL)
		{
			return sexp;
		}

		sym_sexp = symbol_table_get(st, sexp->atom_sym);

		if (sym_sexp == ERROR)
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
		/* It's already an atom we just copy the sexp */
		return sexp_dup(sexp);
	}
}
