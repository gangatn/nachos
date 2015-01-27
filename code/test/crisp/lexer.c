#include "lexer.h"
#include "syscall.h"

/* For no we use a static buffer for convenience */
static int offset = 0;
char token_str[MAX_TOKEN_STR_SIZE] = { 0 };
static int ungetbuf = -1;

static inline int getc(void)
{
	if (ungetbuf != -1)
	{
		int ret = ungetbuf;
		ungetbuf = -1;
		return ret;
	}
	return GetChar();
}

static inline void ungetc(int c)
{
	/* TODO: We do not handle the case when
	 * ungetbuf isn't equals to '\0'
	 * We do not have abort system call and ASSERT macro */
	ungetbuf = c;
}

static inline int isalpha(int c)
{
	return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

static inline int isnum(int c)
{
	return (c >= '0' && c <= '9');
}

static inline int isblank(int c)
{
	return c == ' ' || c == '\t' || c == '\n';
}

/* Return first non whitespace character */
static char skip_ws(void)
{
	char c;
	do { c = getc(); } while(isblank(c));
	return c;
}

static int read_while(int (*predicate)(int c))
{
	int c = getc();
	int i = offset;

	while(predicate(c))
	{
		token_str[i] = c;
		i++;
		c = getc();
	}

	ungetc(c);

	token_str[i] = '\0';
	return i;
}

static inline int isop(int c)
{
	return c == '+' || c == '-' || c == '/' || c == '*';
}

static inline int read_sym(void)
{
	return read_while(isalpha);
}

static inline int isstr(int c)
{
	return c != '"';
}

/* TODO: Handle escaping \" etc */
static inline int read_str(void)
{
	return read_while(isstr);
}

static inline int read_num(void)
{
	/* Note: Maybe this function will one day handle floating point number */
	read_while(isnum);
	return TOKEN_INT;
}


#define PUSH_CHAR(c) \
	do { \
	token_str[0] = c; \
	token_str[1] = '\0'; \
	} while (0)

int lexer_next_token(void)
{
	char c = skip_ws();
	offset = 0;

	/* List delimitation */
	if (c == '{')
	{
		PUSH_CHAR(c);
		return TOKEN_OPEN;
	}
	else if (c == '}')
	{
		PUSH_CHAR(c);
		return TOKEN_CLOSE;
	}
	/* Symbols */
	else if (isop(c))
	{
		PUSH_CHAR(c);

		/* There is an ambiguity if c is + or -
		 * this can be a number start */
		if(c != '+' && c != '-')
		{
			return TOKEN_SYM;
		}

		c = getc();
		ungetc(c);

		if (isnum(c))
		{
			offset++;
			return read_num();
		}
		else
		{
		    return TOKEN_SYM;
		}
	}
	else if (isalpha(c))
	{
		/*
		 * We prefer let the read_str handle the reading
		 * it's not a big deal since ungetc is really fast
		 * (and convenient)
		 */
		ungetc(c);
		read_sym();
		return TOKEN_SYM;
	}
	/* Numbers */
	else if (isnum(c))
	{
		ungetc(c);
		return read_num();
	}
	/* String */
	else if (c == '"')
	{
	    read_str();
		c = getc();

		return c != '"' ? TOKEN_ERR : TOKEN_STR;
	}
	/* Characters */
	else if (c == '\'')
	{
		/* TODO: Handle escaping '\144' etc */
		c = getc();
		PUSH_CHAR(c);
		c = getc();
		return c != '\'' ? TOKEN_ERR : TOKEN_CHAR;
	}
	else if (c == 0 || c == -1)
	{
		return TOKEN_NONE;
	}

	return TOKEN_ERR;
}