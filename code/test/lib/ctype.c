#include <ctype.h>

int isdigit(int c)
{
	return c >= '0' && c <= '9';
}

int isblank(int c)
{
	return c == ' ' || c == '\t';
}

int isspace(int c)
{
	return
		c == ' '  || c == '\t' ||
		c == '\f' || c == '\n' ||
		c == '\r' || c == '\v' ;
}

int islower(int c)
{
	return c >= 'a' && c <= 'z';
}

int isupper(int c)
{
	return c >= 'A' && c <= 'Z';
}

int isalpha(int c)
{
	return islower(c) || isupper(c);
}

int isalnum(int c)
{
	return isalpha(c) || isdigit(c);
}
