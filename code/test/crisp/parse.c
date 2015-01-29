#include <stddef.h>
#include "parse.h"
#include "lexer.h"
#include "syscall.h"

/* Grammar components */
static struct sexp *sexp(void);
static struct sexp *sexp_list(void);
/* ------------------ */

static int parse_int(void)
{
	int res = 0, i = 0;
	int signedness = 1;

	if(token_str[i] == '-' || token_str[i] == '+')
	{
		signedness = token_str[i] == '-' ? -1 : 1;
		i++;
	}

	while (token_str[i] != '\0')
	{
		res *= 10;
		res += token_str[i] - '0';
		i++;
	}
	return signedness * res;
}

static int lookahead;
#define NEXT() lookahead = lexer_next_token()

#define ERROR ((void*)-1)

static struct sexp *sexp_list(void)
{
	if (lookahead != TOKEN_CLOSE)
	{
		struct sexp *car, *cdr;
		car = sexp();

		if (car == ERROR)
		{
			return ERROR;
		}

		NEXT();
		cdr = sexp_list();

		if (cdr == ERROR)
		{
			sexp_free(car);
			return ERROR;
		}
		return sexp_make_cons(car, cdr);
	}
	else
	{
		return NULL;
	}
}

static struct sexp *sexp(void)
{
	switch(lookahead)
	{
	case TOKEN_OPEN:
	{
		NEXT();
	    return sexp_list();
	}
	case TOKEN_QUOTE:
	{
		NEXT();
		if(lookahead != TOKEN_OPEN)
		{
			PutString("Error: syntax error, ' expect an {\n");
			return (void*)-1;
		}
		else
		{
			struct sexp *quoted_sexp;

			NEXT();
			quoted_sexp = sexp_list();
			return sexp_make_cons(
				sexp_make_sym("quote"),
				sexp_make_cons(quoted_sexp, NULL)
				);
		}
	}
	case TOKEN_STR:
		return sexp_make_str(token_str);
	case TOKEN_SYM:
		return sexp_make_sym(token_str);
	case TOKEN_CHAR:
		return sexp_make_char(token_str[0]);
	case TOKEN_INT:
		return sexp_make_int(parse_int());
	case TOKEN_TRUE:
		return sexp_make_bool(1);
	case TOKEN_FALSE:
		return sexp_make_bool(0);

	case TOKEN_CLOSE:
	case TOKEN_ERR:
		PutString("Unexpected token : \"");
		PutString(token_str);
		PutString("\"\n");
		break;
	case TOKEN_NONE:
		return NULL;
	}
	return (void*)-1;
}

struct sexp *parse(void)
{
	NEXT();
	return sexp();
}
