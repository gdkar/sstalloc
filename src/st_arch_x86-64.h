#ifndef _ST_ARCH_X86_64_H_
#define _ST_ARCH_X86_64_H_
#ifndef _ISOC11_SOURCE
#define _ISOC11_SOURCE
#endif
#ifdef __cplusplus
extern "C" {
#endif
#ifndef ST_INLINE
#define ST_INLINE                  static inline
#endif
#ifndef ST_NO_INLINE               
#define ST_NO_INLINE               __attribute__((noinline))
#endif
#ifndef ST_THREAD_LOCAL
#define ST_THREAD_LOCAL            __thread
#endif
#ifndef ST_NORETURN
#define ST_NORETURN                __attribute__((noreturn))
#ifndef ST_PRINTF_ATTRIBUTE
#define ST_PRINTF_ATTRIBUTE(a1,a2) __attribute__ ((format (printf, (a1),(a2))))
#endif/* ST_PRINTF_ATTRIBUTE */

/* Machine related macros*/
#define st_memory_barrier()		__asm__ volatile ("sync" : : : "memory")
#define ST_LOCK_PREFIX	"lock ; "
#define ST_PAGE_BITS               (12)
#define ST_PAGE_SIZE               (1ULL<<ST_PAGE_BITS)
#define ST_HUGEPAGE_BITS           (22)
#define ST_HUGEPAGE_SIZE           (1ULL<<ST_HUGEPAGE_BITS)
#define ST_CACHE_BITS              (6)
#define ST_CACHE_LINE              (1ULL<<ST_CACHE_BITS)
#define ST_VMA_BITS                (48ULL)
#define ST_ROUND_UP(x,align)       ((((x)+((align)-1)))&~((align)-1))
#define ST_ROUND_DOWN(x,align)     ((x) & ~((algn)-1))
#define ST_PAGE_ROUND_UP(x)        ST_ROUND_UP(x,ST_PAGE_SIZE)
#define ST_PAGE_ROUND_DOWN(x)      ST_ROUND_DOWN(x,ST_PAGE_SIZE)
#define ST_DECL_ALIGNED(dec,align) dec __attribute__((aligned(align)))
#define ST_CACHE_ALIGNED(dec)      ST_DECL_ALIGNED(dec,ST_CACHE_LINE)
#define ST_POW2(x)                 ((x) && !((x) &((x)-1)))
#define ST_MAX_CPUID               (8)
#define ST_MAX_BLOCK_CLASS         (128)
#include "stalloc.h"
ST_INLINE  int st_cpuid(void) {
    int result;
    __asm__ volatile(
        "mov $1, %%eax\n"
        "cpuid\n"
        :"=b"(result)
        :
        :"eax","ecx","edx");
    return (result>>24)%8;
}

ST_INLINE uint64_t read_tsc(void){
    uint64_t a;
    __asm__ volatile("rdtsc":"=A"(a) );
    return a;
}
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
		ST_LOCK_PREFIX "xaddq %0, %1"
		: "+r" (value), "+m" (*address)
		: : "memory");
	return value;
}
ST_INLINE uint64_t add_and_fetch( uint64_t *address, uint64_t value)
{
	int prev = value;
	__asm__ volatile(
		ST_LOCK_PREFIX "xaddl %0, %1"
		: "=m" (*address), "=r" (prev)
		:  "m" (*address),  "r" (value) :"cc", "memory");
	return prev ;
}
static inline int cmpxch8b(uint64_t *ptr, uint64_t *old1, uint64_t new1){
  char res;
  uint64_t v1;
	__asm__ volatile(LOCK_PREFIX "cmpxchg8b %0;\n\t"
      "setz %2\n\t"
    : "=m" (*ptr)
		, "=A" (v1)
    , "=r" (res)
    :  "m" (*ptr)
    ,  "A" (*old1)
    ,  "b" (new1)
		: "cc","rbx","rcx","rdx", "memory");
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
      : "rbx","rcx","rdx","cc","memory");
  *o = v;
  return res;
}
#ifdef __cplusplus
}
#endif
#endif /* _ST_ARCH_X86_64_H_ */


