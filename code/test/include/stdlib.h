#ifndef _STDLIB_H_
#define _STDLIB_H_

#ifndef	_SIZE_T
#define	_SIZE_T
typedef unsigned size_t;
#endif

#ifndef NULL
#define NULL ((void*)0)
#endif

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

void *malloc(size_t);
void *realloc(void *, size_t);
void *calloc(size_t, size_t);
void free(void*);

#endif /* _STDLIB_H_ */
