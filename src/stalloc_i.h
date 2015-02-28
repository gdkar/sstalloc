#ifndef _STALLOC_I_H_
#define _STALLOC_I_H_
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
#define ST_TAG_BITS                (sizeof(void*)-ST_VMA_BITS)
#define ST_VMA_MASK                ((1ULL<<ST_VMA_BITS)-1)
#define ST_TAG_MASK                (~ST_VMA_MASK)
#define ST_ROUND_UP(x,align)       ((((x)+((align)-1)))&~((align)-1))
#define ST_ROUND_DOWN(x,align)     ((x) & ~((algn)-1))
#define ST_PAGE_ROUND_UP(x)        ST_ROUND_UP(x,ST_PAGE_SIZE)
#define ST_PAGE_ROUND_DOWN(x)      ST_ROUND_DOWN(x,ST_PAGE_SIZE)
#define ST_DECL_ALIGNED(align)     __attribute__((aligned(align)))
#define ST_CACHE_ALIGNED           ST_DECL_ALIGNED(ST_CACHE_LINE)
#define ST_POW2(x)                 ((x) && !((x) &((x)-1)))
#define ST_MAX_CPUID               (8)
#define ST_MAX_BLOCK_CLASS         (128)
#define ST_LARGE_CLASS         (ST_MAX_BLOCK_CLASS+1)
#define ST_DUMMY_CLASS         (ST_MAX_BLOCK_CLASS+2)
#define ST_DHEADER_SIZE     (sizeof(dchunk_t))
#define ST_MAX_FREE_CHUNK      (ST_MAX_FREE_SIZE/ST_CHUNK_SIZE)
#define ST_MAX_FREE_SIZE       (4*1024*1024)
#define ST_LARGE_OWNER         ((void*)0xDEAD)
#define ST_ACTIVE              ((void*)1)
#define ST_ALLOC_UNIT          (4*1024*1024)
#define ST_CHUNK_DATA_SIZE     (16*ST_PAGE_SIZE)
#define ST_CHUNK_SIZE          (ST_CHUNK_DATA_SIZE+sizeof(dchunk_t))
#define ST_CHUNK_MASK          (~(ST_CHUNK_SIZE-1))
#define ST_RAW_POOL_START      ((void*)((0x600000000000/ST_CHUNK_SIZE+1)*ST_CHUNK_SIZE))
#define ST_BLOCK_BUF_CNT       (16)
/* Utility Macros */
#define unlikely(x)         __builtin_expect(!!(x),0)
#define likely(x)           __builtin_expect(!!(x),1)
#include "stalloc.h"
#include "st_llist.h"
typedef struct int128 {
  int64_t v1;
  int64_t v2;
}int128;

ST_DECL_ALIGNED(sizeof(void*))
typedef struct stack_node{
  struct stack_node      *next;
}stack_node;

ST_DECL_ALIGNED(sizeof(int128))
typedef struct stack{
      stack_node               *first;
      stack_node               *next;
}stack;
ST_CACHE_ALIGNED
typedef struct bulk_buf{
  stack_node              *head;
  stack_node              *tail;
  int64_t                  cnt;
}bulk_buf;

typedef struct lheap lheap;
typedef struct gheap gheap;
typedef struct dchunk dchunk;
typedef struct lchunk lchunk;
typedef enum {
    FOREGROUND,
    BACKGROUND,
    FULL
} dchunk_state;
typedef enum {
    UNINITIALIZED,
    READY
} init_state;

/* Data chunk header */
typedef struct obj_buf {
    dchunk      *dc;
    stack_node *first;
    stack       free;
    int         cnt;
}obj_buf;

struct chunk{
  ST_CACHE_ALIGNED llist    chunk_list;
                   uint32_t numa_node;
};
struct lchunk {
    chunk     as_chunk;
    size_t    size;
    void*     mem;
    ST_CACHE_ALIGNED(struct lheap *owner);
};
struct dchunk {
    /* Local Area */
    chunk     as_chunk;
    /* Read Area */
    ST_CACHE_ALIGNED(struct lheap *owner);
    int64_t  size_cls;
    /* Local Write Area */
    ST_CACHE_ALIGNED(dchunk_state state);
    int64_t  blk_size;
    int64_t  blk_cnt;
    int64_t  free_cnt;
    char    *free_mem;
    stack    free_head;
    /* Remote Write Area */
    ST_CACHE_ALIGNED(bulk_buf remote_free_head);
};
struct lheap{
    chunk         as_chunk;
    uint32_t      free_cnt;
    bulk_buf      free_buf;
    dchunk       *front[ST_MAX_BLOCK_CLASS];
    llist         backq[ST_MAX_BLOCK_CLASS];
    obj_buf       bufs [ST_BLOCK_BUF_CNT];
    ST_CACHE_ALIGNED(
    bulk_buf      need_gc[ST_MAX_BLOCK_CLASS]
         );
    dchunk   dummy_chunk;
};
ST_CACHE_ALIGNED(
struct gheap{
    pthread_mutex_t lock;
    volatile char *pool_start;
    volatile char *pool_end;
    volatile char *free_start;
    stack  dc_cache[ST_MAX_CPUID];
    stack  lh_cache[ST_MAX_CPUID];
    stack  unmapped[ST_MAX_CPUID];
});
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
ST_INLINE int64_t read_tsc(void){
    uint64_t a;    __asm__ volatile("rdtsc":"=A"(a) );return a;}
ST_INLINE uint64_t xchg64(int64_t  *address, int64_t value)
{__asm__ volatile(
    "xchgq %k0,%1"
		:  "=m"(*address),"=r"(value)
		:   "m" (*address), "1" (value)
		:   "memory");
	return value;
}

ST_INLINE int64_t faa64( int64_t *address, int64_t value)
{	__asm__ volatile(
		ST_LOCK_PREFIX "xaddq %0, %1"
		: "=a" (value)
    :  "a" (value), "m" (*address);
		: "cc", "memory");
	return value;
}
ST_INLINE uint64_t aaf64( int64_t *address, int64_t value)
{
	int prev = value;
	__asm__ volatile(
		ST_LOCK_PREFIX "xaddq %%eax, %1"
		: "=a" (value) 
		:  "a" (value), "m" (*address)
    :"cc", "memory");
	return prev + value;
}
static inline int cas64(int64_t *p, int64_t *o, const int64_t n){
  char r;
  int64_t v;
	__asm__ volatile("cmpxchg8b %0;\n\t"
      "setz %2\n\t"
    : "=m" (*p), "=A" ( v), "=r" ( r)
    :  "m" (*p),  "A" (*o),  "b" ( n)
		: "cc","rbx","rcx","rdx", "memory");
  *o = v;
	return r;
}

ST_INLINE int cas128(int128 *p, int128 *o, const int128 n){
  char r;
  int128  v = *o;
  asm volatile("cmpxchg16b %0;setz %3"
      :"=m"(*p),"=d"( v.v2), "=a"( v.v1), "=r"( r)
      : "m"(*p), "d"( o->v2), "a"( o->v1), "c"( n.v2), "b"( n.v1)
      : "cc","rbx","rcx","rdx","memory");
  *o = v;
  return r;
}
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <sched.h>
#include <stdatomic.h>
#include <sys/mman.h>
#include <pthread.h>
#include <assert.h>
#include <unistd.h>
#include <time.h>
#if ST_DEBUG
#include <signal.h>
#include <execinfo.h>
#endif
#include "st_stack.h"
#endif
#endif /* _STALLOC_I_H_ */


