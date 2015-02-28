#ifndef _ST_ATOMIC_H_
#define _ST_ATOMIC_H_
#ifdef _HAVE_STDATOMIC_H
#include <stdatomic.h>
#endif /* _HAVE_STDATOMIC_H_ */
#include "st_arch_x86-64.h"
#define mb()		__asm__ volatile ("sync" : : : "memory")
#define LOCK_PREFIX	"lock ; "
typedef struct int128_t {
  int64_t v1;
  int64_t v2;
}int128_t;

ST_INLINE uint64_t fetch_and_store(uint64_t  *address, uint64_t value)
{__asm__ volatile("xchgq %k0,%1"
		:  "=m"(*address),"=r"(value)
		:   "m" (*address), "1" (value)
		: "memory");
	return value;
}

ST_INLINE uint64_t fetch_and_add( uint64_t *address, uint64_t value)
{	__asm__ volatile(
		LOCK_PREFIX "xaddq %0, %1"
		: "+r" (value), "+m" (*address)
		: : "memory");
	return value;
}
ST_INLINE uint64_t add_and_fetch( uint64_t *address, uint64_t value)
{
	int prev = value;
	__asm__ volatile(
		LOCK_PREFIX "xaddl %0, %1"
		: "=m" (*address), "=r" (prev)
		:  "m" (*address),  "r" (value) :"cc", "memory");
	return prev ;
}
static inline int cmpxchgq(uint64_t *ptr, uint64_t *old1, uint64_t new1)
{
  char res;
  uint64_t v1;
	__asm__ volatile(LOCK_PREFIX "cmpxchg8b %0;\n\t"
      "setz %2\n\t"
    : "=m" (*ptr)
		, "=a" (v1)
    , "=r" (res)
    :  "m" (*ptr)
    ,  "a" (*old1)
    ,  "b" (new1)
		: "cc"
    , "memory");
  *old1 = v1;
	return res;
}

ST_INLINE int cmpxchg16b(int128_t *ptr, int128_t *o, int128_t n){
  char res;
  int128_t  v = *o;
  asm volatile("cmpxchg16b %0;setz %3"
      :"=m"(*ptr)
      ,"=d"(v.v2)
      ,"=a"(v.v1)
      ,"=r"(res)
      : "m"(*ptr)
      , "d"(o->v2)
      , "a"(o->v1)
      , "c"(n.v2)
      , "b"(n.v1)
      : "cc","memory");
  *o = v;
  return res;
}
#endif /* _ST_ATOMIC_H_*/


