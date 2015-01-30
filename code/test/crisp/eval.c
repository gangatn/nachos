#include <stddef.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

#include "eval.h"
#include "symbol_table.h"
#include "syscall.h"

#include "builtin.h"

typedef struct sexp *(*eval_func)(struct sexp *sexp, struct symbol_table *st);

struct symbol
{
	const char *name;
    eval_func func;
};

#define ERROR ((void *)-1)

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
	{"*", eval_mult },
	{"=", eval_equal },
	{">", eval_greater },
	{"quote", eval_quote },
	{"car", eval_car },
	{"cdr", eval_cdr },
	{"cons", eval_cons },
	{"define", eval_define },
	{FUNC_SYMBOL_NAME, eval_lambda },
	{"if", eval_if },
	{"print", eval_print },
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

	tmp = sexp_list_at(sexp, 0);

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


int save_sym(int argc, struct entry *saved,
				struct sexp *args, struct symbol_table *st)
{
	int i;
	for (i = 0; i < argc; i++)
	{
		/* elem can't fail by construction */
		struct sexp *sym = sexp_list_at(args, i);
		struct sexp *dup;

		/* We don't need to copy the key in memory */
		saved[i].key = sym->atom_sym;

		dup = symbol_table_get(st, sym->atom_sym);

		if (dup != ERROR && dup != NULL)
		{
			dup = sexp_dup(dup);
			if (dup == NULL)
			{
				PutString("Error: (save_args) not enough memory\n");
				return 1;
			}
		}
		saved[i].data = dup;
	}
	return 0;
}


static int apply_sym(int argc, struct sexp *fun_args,
					  struct sexp *call_args, struct symbol_table *st)
{
	int i;

	for (i = 0; i < argc; i++)
	{
		struct sexp *sym = sexp_list_at(fun_args, i);
		/* FIXME(Jeremy): we need to eval all parameters then add them
		 * this might not work otherwise */
		struct sexp *val = eval(sexp_list_at(call_args, i), st);

		if (symbol_table_set(st, sym->atom_sym, val) != 0)
		{
			sexp_free(val);
			return 1;
		}
		sexp_free(val);
	}
	return 0;
}

static int restore_sym(int argc, struct entry *saved, struct symbol_table *st)
{
	int i;
	int success = 1;
	for (i = 0; i < argc; i++)
	{
		if (saved[i].data != ERROR)
		{
			if (symbol_table_set(st, saved[i].key, saved[i].data) != 0)
			{
				PutString("Error: Could not restore the symbols!");
				success = 0;
			}
			sexp_free(saved[i].data);
		}
	}
	return success ? 0 : 1;
}

static struct sexp *eval_exec_function(struct sexp *function, struct sexp *args,
									   struct symbol_table *st)
{
	int fun_argc, given_argc;
	struct entry *saved_symbols;
	struct sexp *res;

	fun_argc = sexp_list_len(function->cons.car);
	given_argc = sexp_list_len(args);

	if (fun_argc != given_argc)
	{
		PutString("Error: function required ");
		PutInt(fun_argc);
		PutString(" arguments, ");
		PutInt(given_argc);
		PutString(" given.\n");
		return ERROR;
	}

	res = sexp_list_at(function, 1);
	if (res == ERROR)
	{
		PutString("Error: (eval lambda) invalid body!");
		return ERROR;
	}

	saved_symbols = malloc(sizeof(*saved_symbols) * fun_argc);
	if (saved_symbols == NULL)
	{
		PutString("Error: (eval lambda) not enough memory\n");
		return ERROR;
	}

	if (save_sym(fun_argc, saved_symbols, function->cons.car, st) != 0)
	{
		free(saved_symbols);
		return ERROR;
	}

	if (apply_sym(fun_argc, function->cons.car, args, st) != 0)
	{
		if (restore_sym(fun_argc, saved_symbols, st) == 0)
		{
			PutString("Error: (eval lambda) canot apply parameters\n");
		}
		return ERROR;
	}

	res = eval(res, st);

	if (restore_sym(fun_argc, saved_symbols, st) != 0)
	{
		sexp_free(res);
		res = ERROR;
	}

	free(saved_symbols);

	return res;
}

struct sexp *eval(struct sexp *sexp, struct symbol_table *st)
{
	/* evaluation an error sexp is a programmatic error */
	assert(sexp != ERROR);

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
			struct sexp *ret = eval_exec_function(
				sexp_fun->cons.cdr,
				sexp->cons.cdr, st
				);
			sexp_free(sexp_fun);
			return ret;
		}
		else
		{
			PutString("Error: ");
			sexp_print(sexp_fun);
			PutString(" is not a function.\n");
			sexp_free(sexp_fun);
			return ERROR;
		}
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
