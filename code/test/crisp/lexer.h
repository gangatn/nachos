#ifndef LEXER_H_
#define LEXER_H_

#define MAX_TOKEN_STR_SIZE 128

/*
 * The "open" token and "close" token represent
 * the opening and closing lexeme for a list, usually
 * '(' and ')', in crisp, it's '{' and '}'
 */
enum token_id
{
	TOKEN_ERR = -1,
	TOKEN_NONE,
	TOKEN_OPEN,
	TOKEN_CLOSE,
	TOKEN_QUOTE,
	TOKEN_STR,
	TOKEN_SYM,
	TOKEN_CHAR,
	TOKEN_INT
};

extern char token_str[MAX_TOKEN_STR_SIZE];

int lexer_next_token(void);

#endif /* LEXER_H_ */
