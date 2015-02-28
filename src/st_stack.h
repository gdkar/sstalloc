#ifndef _ST_STACK_H_
#define _ST_STACK_H_
#include "stalloc_i.h"

#define ST_VMA_GET(x)          (((uint64_t)x) & ST_VMA_MASK)
#define ST_TAG_GET(x)          (((uint64_t)x) >>ST_VMA_BITS)
#define ST_TAG_SET(x,tag)      (ST_VMA_GET(x) | (((uint64_t)tag)<<ST_VMA_BITS))

ST_INLINE void stack_init(stack *s){
  s->firstptr = NULL;
  s->nextptr  = NULL;
}
ST_INLINE void stack_put (stack *s, stack_node *node){
  stack     sref,nref;
  sref = *s;
  do{
    nref.first = node;
    nref.next  = sref.first;  
    node->next = sref.first;
  }while(!cas128((int128*)s,(int128*)&sref,*(int128*)&nref));
}
ST_INLINE stack_node * stack_pop(stack *s){
  stack sref, nref;
  sref = *s;
  do{
    nref.first = sref.next;
    nref.next  = nref.first->next;
  }while(!cas128((int128*)s,(int128*)&sref,*(int128*)&nref));
  return sref.first;
}
ST_INLINE void local_stack_put (stack *s, stack_node *node)
{node->next = s->first,s->next = s->first,s->first = node;}
ST_INLINE stack_node *local_stack_pop (stack *s){
  stack_node *node = s->first;
  if(!node)return NULL;
  s->first         = s->next;
  s->next          = (s->first)?s->first->next : NULL;
  return node;
}
ST_INLINE bulk_buf_init(bulk_buf *buf){
  buf->head = NULL;
  buf->tail = NULL;
  buf->count= 0;
}
ST_INLINE void local_buf_put(bulk_buf *buf,stack_node *node){
  node->next = buf->head;
  buf->head  = node;
  buf->cnt++;
  if(unlikely(!buf->tail)){
    buf->tail = node;
  }
}
ST_INLINE stack_node *local_buf_pop(bulk_buf *buf){
  stack_node *node = buf->head;
  if(node){
    buf->head               =node->next;
    if(!node->next)buf->tail=NULL;
  }
  buf->cnt--;
  return node;
}
ST_INLINE void indexed_stack_put(stack *s, bulk_buf *buf){
  stack     sref,nref;
  int64_t tag;
  sref = *s;
  do{
    tag             = ST_TAG_GET(sref.first) + buf->cnt;
    buf->tail->next = (stack_node*)ST_VMA_GET(sref.first);
    nref.first      = (stack_node*)ST_TAG_SET(buf->tail,tag);
    nref.next       = nref.first->next;
  }while(!cas128((int128*)s,(int128*)&sref,*(int128*)&nref));
}

ST_INLINE stack_node *indexed_stack_pop_all(stack *s,int64_t *cnt){
  stack     sref,nref;
  nref = {NULL,NULL};
  sref = *s;
  do{
  }while(!cas128((int128*)s,(int128*)&sref,*(int128*)&nref));
  *cnt = ST_TAG_GET(sref.next);
  (stack_node*)ST_VMA_GET(sref.first);
}

ST_INLINE void indexed_stack_put(stack *s, stack_node *head, stack_node *tail,int64_t cnt){
  int64_t old_head,new_head,tag;
  old_head = (int64_t)s->first;
  do{
    int64_t tag        = ST_TAG_GET(old_head);
    tail->next.nextint = ST_VMA_SET(old_head);
    new_head.nextint   = ST_TAG_SET(head,tag+cnt);
  }while(!cas64(&s->first.nextint,&old_head,new_head));
}
ST_INLINE stack_node *unsafe_buf_pop(bulk_buf *s){
  stack_node *node = unsafe_stack_pop(&s->head);
  if(!node)return NULL;
  s->count--;
  if(!s->head.first)
    s->tail = NULL;
  return node;
}
#endif /* _ST_STACK_H_*/
