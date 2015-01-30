#include "builtin.h"
#include "sexp.h"
#include "syscall.h"

#define ERROR ((void *)-1)

struct sexp *eval_add(struct sexp *sexp, struct symbol_table *st)
{
	int result = 0;
	struct sexp *cur = sexp;
	struct sexp *arg;

	while(cur != NULL)
	{
		if (cur->type != SEXP_CONS)
		{
			PutString("error: (+) arguments are not a valid list\n");
			return ERROR;
		}

		arg = eval(cur->cons.car, st);
		if(arg->type != SEXP_ATOM_INT)
		{
			PutString("Error: (+) needs number as argument\n");
			sexp_free(arg);
			return ERROR;
		}

		result += arg->atom_int;
		cur = cur->cons.cdr;
		sexp_free(arg);
	}
	return sexp_make_int(result);
}

struct sexp *eval_mult(struct sexp *sexp, struct symbol_table *st)
{
	int result = 1;
	struct sexp *cur = sexp;
	struct sexp *arg;

	while(cur != NULL)
	{
		if (cur->type != SEXP_CONS)
		{
			PutString("error: (*) arguments are not a valid list\n");
			return ERROR;
		}

		arg = eval(cur->cons.car, st);
		if(arg->type != SEXP_ATOM_INT)
		{
			/* Temporary error message */
			PutString("Error: (*) needs number as argument\n");
			sexp_free(arg);
			return ERROR;
		}

		result *= arg->atom_int;
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
			PutString("Error: (-) arguments are not a valid list\n");
			return ERROR;
		}

		arg = eval(cur->cons.car, st);
		if(arg->type != SEXP_ATOM_INT)
		{
			PutString("Error: (-) needs number as argument\n");
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
	int arg_count = sexp_list_len(sexp);

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

	if (target == ERROR)
	{
		return ERROR;
	}

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
		arg = sexp_list_at(sexp, i);
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

	args = sexp_list_at(sexp, 0);
	body = sexp_list_at(sexp, 1);

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

struct sexp *eval_equal(struct sexp *sexp, struct symbol_table *st)
{
	int result = -1, cmp;
	int argc = 0;
	struct sexp *cur = sexp;
	struct sexp *arg;

	while(cur != NULL)
	{
		if (cur->type != SEXP_CONS)
		{
			PutString("error: (=) arguments are not a valid list\n");
			return ERROR;
		}

		arg = eval(cur->cons.car, st);
		if(arg->type != SEXP_ATOM_INT)
		{
			PutString("Error: (=) needs number as argument\n");
			sexp_free(arg);
			return ERROR;
		}

		if (result == -1)
		{
			cmp = arg->atom_int;
			result = 1;
		}
		else
		{
			if (arg->atom_int != cmp)
			{
				result = 0;
			}
		}

		argc++;

		cur = cur->cons.cdr;
		sexp_free(arg);
	}

	if(argc <= 1)
	{
		PutString("Error: (=) too few arguments (at least 2, got ");
		PutInt(argc);
		PutString(")\n");
		return ERROR;
	}
	return sexp_make_bool(result);
}

struct sexp *eval_greater(struct sexp *sexp, struct symbol_table *st)
{
	int result = -1, cmp;
	int argc = 0;
	struct sexp *cur = sexp;
	struct sexp *arg;

	while(cur != NULL)
	{
		if (cur->type != SEXP_CONS)
		{
			PutString("error: (>) arguments are not a valid list\n");
			return ERROR;
		}

		arg = eval(cur->cons.car, st);
		if(arg->type != SEXP_ATOM_INT)
		{
			/* Temporary error message */
			PutString("Error: (>) needs number as argument\n");
			sexp_free(arg);
			return ERROR;
		}

		if (result == -1)
		{
			cmp = arg->atom_int;
			result = 1;
		}
		else
		{
			if (arg->atom_int > cmp)
			{
				result = 0;
			}
			cmp = arg->atom_int;
		}

		argc++;

		cur = cur->cons.cdr;
		sexp_free(arg);
	}

	if(argc <= 1)
	{
		PutString("Error: (>) too few arguments (at least 2, got ");
		PutInt(argc);
		PutString(")\n");
		return ERROR;
	}
	return sexp_make_bool(result);
}

struct sexp *eval_if(struct sexp *sexp, struct symbol_table *st)
{
	struct sexp *bool, *res;
	int argc;

	argc = sexp_list_len(sexp);

	if (argc < 2)
	{
		PutString("Error: (if) too few arguments (at least 2, got ");
		PutInt(argc);
		PutString(")\n");
	}

	bool = sexp_list_at(sexp, 0);

	if (bool == ERROR)
	{
		PutString("Error: (if) ill formed condition\n");
		return ERROR;
	}

	bool = eval(bool, st);
	if (bool == NULL && bool->type == SEXP_ATOM_BOOL)
	{
		PutString("Error: (if) condition must be a boolean\n");
		sexp_free(bool);
		return ERROR;
	}

	res = sexp_list_at(sexp, bool->atom_bool ? 1 : 2);
	if (res == ERROR)
	{
		PutString("Error: (if) ill formed then or else\n");
		sexp_free(bool);
		return ERROR;
	}

	res = eval (res, st);
	sexp_free(bool);
	return res;
}

static void print_sexp_internal(struct sexp *sexp)
{
	switch (sexp->type)
	{
	case SEXP_CONS:
	{
	    print_sexp_internal(sexp->cons.car);
		if (sexp->cons.cdr != NULL)
		{
			PutChar(' ');
			print_sexp_internal(sexp->cons.cdr);
		}
	}
		break;
	case SEXP_ATOM_INT:
		PutInt(sexp->atom_int);
		break;
	case SEXP_ATOM_CHAR:
		PutChar(sexp->atom_char);
		break;
	case SEXP_ATOM_STR:
		PutString(sexp->atom_str);
		break;
	case SEXP_ATOM_SYM:
		PutString(sexp->atom_sym);
		break;
	case SEXP_ATOM_BOOL:
		PutChar('#');
		PutChar(sexp->atom_bool ? 't' : 'f');
	}
}

static void print_sexp(struct sexp *sexp)
{
	if(sexp != NULL)
	{
		if (sexp->type == SEXP_CONS)
		{
			PutString("(");
			print_sexp_internal(sexp);
			PutString(")");
		}
		else
		{
			print_sexp_internal(sexp);
		}
	}
}


struct sexp *eval_print(struct sexp *sexp, struct symbol_table *st)
{
	struct sexp *cur = sexp;
	struct sexp *arg;

	while(cur != NULL)
	{
		if (cur->type != SEXP_CONS)
		{
			PutString("error: (print) arguments are not a valid list\n");
			return ERROR;
		}

		arg = eval(cur->cons.car, st);
		if (arg == ERROR)
		{
			return ERROR;
		}

		print_sexp(arg);

		cur = cur->cons.cdr;
		sexp_free(arg);
	}
	return NULL;
}
