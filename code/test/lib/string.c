#include <string.h>
#include <stdlib.h>

/*
 * TODO(Jeremy):
 * Basically pretty all functions of this file are
 * not optimized, if we have time left, we should optimize this file
 */

/*
 * TODO(Jeremy): Optimise this function by counting
 * word by word (4 bytes)
 */
size_t strlen(const char *str)
{
	int i = 0;
	while(str[i]) i++;
	return i;
}

int strcmp(const char *s1, const char *s2)
{
	while (*s1 == *s2)
	{
		if (*s1 == '\0')
			return 0;
		s1++, s2++;
	}
	return (*(unsigned char *)s1 < *(unsigned char *)s2) ? -1 : +1;
}

char *strdup(const char *str)
{
	size_t len = strlen(str) + 1;
	char *ret = malloc(len);

	if(ret)
	{
		int i;
		/* Once again, should be optimised using words */
		for(i = 0; i < len; i++)
			ret[i] = str[i];
	}
	return ret;
}
