#ifndef _STALLOC_H_
#define _STALLOC_H_
#ifndef _ISOC11_SOURCE
#define _ISOC11_SOURCE
#endif
#ifdef __cplusplus
extern "C" {
#endif
#include <stdint.h>
#include <stddef.h>
#include <limits.h>
#include <sys/types.h>
#include <errno.h>
#ifndef ST_INLINE
#define ST_INLINE           static inline
#endif
#ifndef ST_NO_INLINE
#define ST_NO_INLINE        __attribute__((noinline))
#endif
#ifndef ST_THREAD_LOCAL
#define ST_THREAD_LOCAL     __thread
#endif
/* Configurations */
#ifndef ST_RETURN_MEMORY
#define ST_RETURN_MEMORY       1
#endif
#ifndef ST_DEBUG
#define ST_DEBUG               0 
#endif
// #define DEBUG
/* Other */
/* Per-thread data chunk pool */
#ifndef ST_MAX
#define ST_MAX(a,b) __extension__({__typeof__(a) __a__=(a);__typeof__(b) __b__=(b); (__a__ > __b__ )?__a__:__b__;})
#endif
#ifndef ST_MIN
#define ST_MIN(a,b) __extension__({__typeof__(a) __a__=(a);__typeof__(b) __b__=(b); (__a__ < __b__ )?__a__:__b__;})
#endif
void*  aligned_alloc(size_t align, size_t size);
void*       memalign(size_t align, size_t size);
int   posix_memalign(void **pptr, size_t align, size_t size); 
void*         calloc(size_t nmemb, size_t size);
void*         valloc(size_t size);
void*         malloc(size_t size);
void*        realloc(void *ptr, size_t size);
void*        pvalloc(size_t size);
void            free(void *ptr);
#ifdef __cplusplus
}
#endif
#endif /* _STALLOC_H_ */
