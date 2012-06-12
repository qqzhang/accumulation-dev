#ifndef _STACK_H_INCLUDED_
#define _STACK_H_INCLUDED_

#include <stdbool.h>

#ifdef  __cplusplus
extern "C" {
#endif

struct stack_s;

struct stack_s* stack_new(int num, int element_size);
void stack_delete(struct stack_s* self);
bool stack_push(struct stack_s* self, const void* data);
char* stack_pop(struct stack_s* self);
bool stack_isfull(struct stack_s* self);
bool stack_increase(struct stack_s* self, int increase_num);
int stack_num(struct stack_s* self);
int stack_top(struct stack_s* self);

#ifdef  __cplusplus
}
#endif

#endif
