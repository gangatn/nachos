#ifndef _STRING_H_
#define _STRING_H_

#ifndef	_SIZE_T
#define	_SIZE_T
typedef unsigned size_t;
#endif

#ifndef NULL
#define NULL ((void*)0)
#endif

size_t strlen(const char *);
int strcmp(const char*, const char*);
char *strdup(const char*);

#endif /* _STRING_H_ */
