#ifndef _ASSERT_H_
#define _ASSERT_H_

#ifndef NDEBUG

extern void __assert_fail(const char *assertion, const char *file,
						  unsigned line);

#ifndef assert
#define assert(expr) \
	((expr) ? (void)0 : __assert_fail( #expr, __FILE__, __LINE__)) ;
#endif

#else

#ifndef assert
#define assert(expr) (void)0
#endif

#endif

#endif /* _ASSERT_H_ */
