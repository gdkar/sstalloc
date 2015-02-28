#ifndef __INCLUDE_STACK_H__
#define __INCLUDE_STACK_H__
#include <stdatomic.h>
#include "cpu.h"

typedef atomic_size_t         atomic_ptr;
typedef size_t                tagged_ptr;

#define ADDR_BITS        (48)
#define TAG_BITS         (sizeof(ptr_t)-ADDR_BITS)
#define ADDR_MASK        ((1L<<ADDR_BITS)-1)
#define ADDR_GET_INT(ptr)(((ptr_t)ptr)&ADDR_MASK)
#define ADDR_GET(ptr)    ((void*)(((ptr_t)ptr)&ADDR_MASK))
#define TAG_INC          (1<<ADDR_BITS)
#define TAG_MASK         (~ADDR_MASK)
#define TAG_GET(ptr)     (((ptr_t)ptr)>>ADDR_BITS)
#define TAG_SET(ptr,tag) (ADDR_GET_INT(ptr)|(((ptr_t)tag)<<ADDR_BITS))

typedef struct lifo_t{
    CACHE_ALIGN aptr_t head;
} lifo_t;
typedef struct node_t {
  aptr_t next;
} node_t;

void lifo_init(lifo_t *s);
void lifo_put_tagged(lifo_t *s, node_t *n);
node_t *lifo_pop_tagged(lifo_t *s);
void lifo_put_untagged(lifo_t *s, node_t *n);
node_t *lifo_pop_untagged(lifo_t *s, node_t *n);
void seq_fifo_put(void *q, void *e){

}

/* Sequential LIFO Queue */

static inline void seq_stack_push(lifo_t *s, void *element){
    *(void**)element = *(void**)&s;
    *(void**)&s->head = (ptr_t)element;
}
static inline void *seq_stack_pop(lifo_t *s ){
   void* oldh;
   if((oldh = (void*)s->head))
      *(void **)s->head = *(void**)oldh;
   return oldh;
}

#define seq_head(queue) (queue)

/* Counted Queue */
static inline void* seq_count_push(lifo_t *s , void* elem) {
    ptr_t oldh, newh;
    oldh = (ptr_t)s->head;
    do {
        oldh = s->head;
        *(ptr_t*)elem = ADDR_GET_INT(oldh);
        newh = (ptr_t)elem;
        newh|= TAG_GET(oldh) + TAG_INC;
        newh = TAG_SET(elem,TAG_GET(oldh)+1);
       
    } while(!(atomic_compare_exchange_weak(
            &s->head,
            &oldh,
           newh 
    )));    

    return (void*)oldh;
}

static inline void* counted_chain_enqueue(lifo_t *s, void* elems, void* tail, int cnt) {
    unsigned long long old_head, new_head, prev;
    do {
        old_head = s->head;
        *(ptr_t*)tail = (ptr_t)ADDR_GET(old_head);
        new_head = (ptr_t)elems;
        new_head |= TAG_GET(old_head) + TAG_INC* cnt;

    } while((prev=compare_and_swap64_value (
            &s->head,
            old_head,
            new_head
    ))!=old_head);    

    return (void*)prev;
}

static inline void* counted_chain_dequeue(lifo_t * s, uint32_t *count) {
    unsigned long long old_head;
	while(1) {
		old_head = *(ptr_t*)s;
		if (old_head == 0)
			return(NULL);
		if (compare_and_swap64(&s->head, old_head, 0)) {
            *count = TAG_GET(old_head) >> ADDR_BITS;
			return(ADDR_GET(old_head));
		}
	}
}

#endif
#include "stack.h"


void lifo_init(lifo_t *s){ATOMIC_VAR_INIT(s,0);}

void lifo_put_tagged ( lifo_t *s, node_t *n ){
  ptr_t existing, replacemnt;
  existing = atomic_load((aptr_t*)(&s->head));
  node_t *addr;
  do{
    *(ptr_t*)(&n->next) = ADDR_GET_INT(existing);
    replacement         = TAG_SET(n,TAG_GET(existing)+1);
  }while(!atomic_compare_exchange_weak(&s->head,&existing,replacement));
}

node_t *lifo_pop_tagged ( lifo_t *s ){
  ptr_t existing, replacement;
  existing = atomic_load(&s->head);
  node_t *addr;
  do{
    if(!(addr = ADDR_GET(existing)))break;
    replacement = TAG_SET(*(ptr_t*)addr,TAG_GET(existing)+1);
  }while(!atomic_compare_exchange_weak(&s->head,&existing,replacement));
  return addr;
}
void lifo_push(lifo_t *s, node_t *n){
  ptr_t existing, replacement;
  existing = atomic_load(&s->head);
  do{
    *(ptr_t*)(&n->next) = existing;
    replacement         = *(ptr_t*)&n->next;
  }while(atomic_compare_exchange_weak(&s->head,&existing,replacement));
}
node_t *lifo_pop(lifo_t *s){
  ptr_t existing, replacement;
  node_t  *addr;
  existing = atomic_load(&s->head);
  do{
    if(!(addr = (node_t *)existing))break;
    replacement = *(ptr_t*)&addr->next;
  }while(!atomic_compare_exchange_weak(&s->head,&existing,replacement));
  return addr;
}


