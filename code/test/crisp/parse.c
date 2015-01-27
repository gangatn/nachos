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

	while (token_str[i] != '\0')
	{
		res *= 10;
		res += token_str[i] - '0';
		i++;
	}
	return res;
}

static int lookahead;
#define NEXT() lookahead = lexer_next_token()

static struct sexp *sexp_list(void)
{
	if (lookahead != TOKEN_CLOSE)
	{
		struct sexp *car, *cdr;
		car = sexp();
		NEXT();
		cdr = sexp_list();
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
	case TOKEN_STR:
		return sexp_make_str(token_str);
	case TOKEN_SYM:
		return sexp_make_sym(token_str);
	case TOKEN_CHAR:
		return sexp_make_char(token_str[0]);
	case TOKEN_INT:
		return sexp_make_int(parse_int());
	case TOKEN_NONE:
	case TOKEN_ERR:
		PutString("Error: bad token\n");
		break;
	}
	return NULL;
}

struct sexp *parse(void)
{
	NEXT();
	return sexp();
}
