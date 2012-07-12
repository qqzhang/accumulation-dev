#include <string.h>
#include <stdlib.h>

#include "array.h"
#include "stack.h"

struct stack_s
{
    struct array_s* array;
    int element_num;
    int top;
};

struct stack_s* stack_new(int num, int element_size)
{
    struct stack_s* ret = (struct stack_s*)malloc(sizeof(struct stack_s));

    if(NULL != ret)
    {
        memset(ret, 0, sizeof(*ret));

        ret->array = array_new(num, element_size);

        if(NULL != ret->array)
        {
            ret->element_num = num;
            ret->top = 0;
        }
        else
        {
            stack_delete(ret);
            ret = NULL;
        }
        
    }

    return ret;
}

void stack_delete(struct stack_s* self)
{
    if(NULL != self)
    {
        if(NULL != self->array)
        {
            array_delete(self->array);
            self->array = NULL;
        }
        
        self->element_num = 0;
        self->top = 0;
        free(self);
        self = NULL;
    }
}

bool stack_push(struct stack_s* self, const void* data)
{
    int top_index = self->top;

    if(top_index < self->element_num)
    {
        array_set(self->array, top_index, data);
        self->top++;
        return true;
    }
    else
    {
        return false;
    }
}

char* stack_pop(struct stack_s* self)
{
    char* ret = NULL;

    if(self->top > 0)
    {
        self->top--;
        ret = array_at(self->array, self->top);
    }
    
    return ret;
}

bool stack_isfull(struct stack_s* self)
{
    return (self->top == self->element_num);
}

bool stack_increase(struct stack_s* self, int increase_num)
{
    bool ret = array_increase(self->array, increase_num);
    if(ret)
    {
        self->element_num = array_num(self->array);
    }

    return ret;
}

int stack_num(struct stack_s* self)
{
    return self->element_num;
}

int stack_top(struct stack_s* self)
{
    return self->top;
}

