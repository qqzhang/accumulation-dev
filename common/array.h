#ifndef _ARRAY_H_INCLUDED_
#define _ARRAY_H_INCLUDED_

#include <stdbool.h>

#ifdef  __cplusplus
extern "C" {
#endif

struct array_s;

struct array_s* array_new(int num, int element_size);
void array_delete(struct array_s* self);
char* array_at(struct array_s* self, int index);
bool array_set(struct array_s* self, int index, const void* data);
bool array_increase(struct array_s* self, int increase_num);
int array_num(struct array_s* self);

#ifdef  __cplusplus
}
#endif

#endif
