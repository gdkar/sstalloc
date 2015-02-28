#ifndef _ST_LLIST_H_
#define _ST_LLIST_H_
#include "stalloc_i.h"
#include <assert.h>
#include <errno.h>
typedef struct llist_node{
	//void* __padding;
	struct llist_node * next;
	struct llist_node * prev;
}llist_node;
typedef list_node llist;
/* Places new_node at the front of the list. */
ST_INLINE void list_init(llist *list){
  /**
   *  construct as a ring-list, pointed into by the head and tail pointers.
   *  with this construction, the list reference is --- strictly --- itself
   *  a list element, such that the empty list is the ring of the list_ref
   *  pointing to itself in both directions;
   *
   */
  list->next = list;
  list->prev = list;
}
ST_INLINE int list_put_head(llist *list, llist_node* node){
	llist_node* head;
  if(!list || !node || !(head=list->next) )
    return EINVAL;
  /**
   *  on insert into an empty list, we retrieve head = list->head ( = list )
   *  and prev = head->priv ( = list->priv = list )
   *  and set node->next = head ( = list )
   *          node->prev = list ( = list )
   *  and     head->prev = node
   *          list->next = node
   */
  node->next=head,node->prev=list;
  list->next=node,head->prev=node;
  return 0;
}
ST_INLINE int list_put_tail(list_ref *list, llist_node *node){
  llist_node *tail;
  if(!list || !node || !(tail=list->prev))
    return EINVAL;
  node->prev=tail;node->next=list;
  list->prev=node;tail->next=node;
  return 0;
}
ST_INLINE int list_empty(llist*list){
  if(list->next == list && list->prev==list)
    return 1;
  if(!list->next || !list->prev
   ||!(list==list->next->prev)
   ||!(list==list->prev->next))
    return EINVAL;
  return 0;
}
/* Removes node from the list. */
ST_INLINE int list_delete(llist_node *node){
  if(!node || !node->next || !node->prev || (node->next==node) || (node->prev==node))
    return EINVAL;
  llist_node *next = node->next, *prev=node->prev;
  next->prev=prev;
  prev->next=next; 
  return 0;
}
ST_INLINE llist_node *list_pop_head(llist*list){
  llist_node *head,*next;
  if(!list || !(head=list->next)||(head==list)||!(next=head->next))
    return NULL;
  list->next = next;next->prev = list;head->next = head;head->prev = head;
  return head;
}
ST_INLINE list_node *list_pop_tail(list_ref *list){
  list_node *tail,*prev;
  if(!list || !(tail=list->prev)||(tail==list)||!(prev=tail->prev))
    return NULL;
  list->prev = prev;prev->next = list;tail->next = tail;tail->prev = tail;
  return tail;
}
#endif /* _ST_LLIST_H_ */
