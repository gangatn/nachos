#ifndef _STDDEF_H_
#define _STDDEF_H_

#ifndef	_SIZE_T
#define	_SIZE_T
typedef unsigned size_t;
#endif

#ifndef NULL
#define NULL ((void*)0)
#endif

#ifndef offsetof
#define offsetof(st, m) ((size_t)(&((st *)0)->m))
#endif

#endif /* _STDDEF_H_ */
