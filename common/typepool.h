#ifndef _TYPEPOOL_H_INCLUDED_
#define _TYPEPOOL_H_INCLUDED_

#ifdef  __cplusplus
extern "C" {
#endif

struct type_pool_s;

struct type_pool_s* type_pool_new(int num, int element_size);
void type_pool_delete(struct type_pool_s* self);
char* type_pool_claim(struct type_pool_s* self);
void type_pool_reclaim(struct type_pool_s* self, char* data);
void type_pool_increase(struct type_pool_s* self, int increase_num);

#ifdef  __cplusplus
}
#endif

#endif
