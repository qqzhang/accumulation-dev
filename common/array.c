#include <string.h>
#include <stdlib.h>

#include "array.h"

struct array_s
{
    char*   buffer;
    int     buffer_size;
    int     element_size;
    int     element_num;
};

struct array_s* array_new(int num, int element_size)
{
    int buffer_size = num * element_size;
    struct array_s* ret = (struct array_s*)malloc(sizeof(struct array_s));

    if(NULL != ret)
    {
        memset(ret, 0, sizeof(*ret));

        ret->buffer = (char*)malloc(buffer_size);

        if(NULL != ret->buffer)
        {
            memset(ret->buffer, 0, buffer_size);
            ret->element_size = element_size;
            ret->element_num = num;
            ret->buffer_size = buffer_size;
        }
        else
        {
            array_delete(ret);
            ret = NULL;
        }
    }

    return ret;
}

void array_delete(struct array_s* self)
{
    if(NULL != self)
    {
        if(NULL != self->buffer)
        {
            free(self->buffer);
            self->buffer = NULL;
        }

        self->element_num = 0;
        free(self);
        self = NULL;
    }
}

char* array_at(struct array_s* self, int index)
{
    char* ret = NULL;
    
    if(index < self->element_num)
    {
        ret = self->buffer + (index * self->element_size);
    }

    return ret;
}

bool array_set(struct array_s* self, int index, const void* data)
{
    char* old_data = array_at(self, index);

    if(NULL != old_data)
    {
        memcpy(old_data, data, self->element_size);
        return true;
    }
    else
    {
        return false;
    }
}

bool array_increase(struct array_s* self, int increase_num)
{
    int new_buffer_size = 0;
    char* new_buffer = NULL;

    if(increase_num <= 0)
    {
        return false;
    }

    new_buffer_size = self->buffer_size + increase_num * self->element_size;
    new_buffer = (char*)malloc(new_buffer_size);

    if(NULL != new_buffer)
    {
        memcpy(new_buffer, self->buffer, self->buffer_size);
        free(self->buffer);
        self->buffer = new_buffer;
        self->element_num += increase_num;
        self->buffer_size = new_buffer_size;

        return true;
    }
    else
    {
        return false;
    }
}

int array_num(struct array_s* self)
{
    return self->element_num;
}

