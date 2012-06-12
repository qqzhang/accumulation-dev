#ifndef _RWLIST_H_INCLUDED_
#define _RWLIST_H_INCLUDED_

#ifdef  __cplusplus
extern "C" {
#endif

struct rwlist_s;
struct stack_s;

struct rwlist_s* rwlist_new(int num, int element_size);
void rwlist_delete(struct rwlist_s* self);
void rwlist_push(struct rwlist_s* self, const void* data);
struct stack_s* rwlist_get_readlist(struct rwlist_s* self);
int rwlist_get_readlistnum(struct rwlist_s* self);

#ifdef  __cplusplus
}
#endif

#endif
