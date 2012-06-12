#include <stdlib.h>
#include <string.h>
#include "stack.h"
#include "mutex.h"
#include "rwlist.h"

struct rwlist_s
{
    struct stack_s*    read;
    struct stack_s*    write;
    struct mutex_s*    mutex;
};

struct rwlist_s* rwlist_new(int num, int element_size)
{
    struct rwlist_s* ret = (struct rwlist_s*)malloc(sizeof(struct rwlist_s));

    if(NULL != ret)
    {
        memset(ret, 0, sizeof(*ret));

        ret->read = stack_new(num, element_size);
        ret->write = stack_new(num, element_size);
        ret->mutex = mutex_new();

        if(NULL == ret->read || NULL == ret->write || NULL == ret->mutex)
        {
            rwlist_delete(ret);
            ret = NULL;
        }
    }

    return ret;
}

void rwlist_delete(struct rwlist_s* self)
{
    if(NULL != self)
    {
        if(NULL != self->mutex)
        {
            mutex_lock(self->mutex);
        }
        
        if(NULL != self->write)
        {
            stack_delete(self->write);
            self->write = NULL;
        }
        
        if(NULL != self->read)
        {
            stack_delete(self->read);
            self->read = NULL;
        }

        if(NULL != self->mutex)
        {
            mutex_unlock(self->mutex);
        }

        if(NULL != self->mutex)
        {
            mutex_delete(self->mutex);
            self->mutex = NULL;
        }

        free(self);
        self = NULL;
    }
}

void rwlist_push(struct rwlist_s* self, const void* data)
{
    mutex_lock(self->mutex);
    if(stack_isfull(self->write))
    {
        stack_increase(self->write, stack_num(self->write));
    }
    stack_push(self->write, data);
    mutex_unlock(self->mutex);
}

struct stack_s* rwlist_get_readlist(struct rwlist_s* self)
{
    struct stack_s* temp = NULL;
    mutex_lock(self->mutex);
    temp = self->write;
    self->write = self->read;
    self->read = temp;
    mutex_unlock(self->mutex);

    return self->read;
}

int rwlist_get_readlistnum(struct rwlist_s* self)
{
    return stack_top(self->write);
}
