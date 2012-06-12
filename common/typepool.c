#include <string.h>
#include <stdlib.h>

#include "array.h"
#include "stack.h"
#include "typepool.h"

struct type_pool_node_s
{
    struct array_s* data_array;
    struct stack_s* ptr_stack;
    struct type_pool_node_s* next;
};

struct type_pool_s
{
    int element_size;
    struct type_pool_node_s* free_node;
    struct type_pool_node_s* head;
    struct type_pool_node_s* tail;
};

static void type_pool_node_delete(struct type_pool_node_s* self)
{
    if(NULL != self)
    {
        if(NULL != self->data_array)
        {
            array_delete(self->data_array);
            self->data_array = NULL;
        }

        if(NULL != self->ptr_stack)
        {
            stack_delete(self->ptr_stack);
            self->ptr_stack = NULL;
        }

        self->next = NULL;
        free(self);
        self = NULL;
    }
}

static struct type_pool_node_s* type_pool_node_new(int num, int element_size)
{
    struct type_pool_node_s* ret = (struct type_pool_node_s*)malloc(sizeof(struct type_pool_node_s));

    if(NULL != ret)
    {
        memset(ret, 0, sizeof(*ret));

        if(NULL != (ret->ptr_stack = stack_new(num, sizeof(void*))) && 
           NULL != (ret->data_array = array_new(num, element_size)))
        {
            int i = 0;

            for(; i < num; ++i)
            {
                char* element_data = array_at(ret->data_array, i);
                memset(element_data, 0, element_size);
                memcpy(element_data, &ret, sizeof(struct type_pool_node_s*));
                stack_push(ret->ptr_stack, &element_data);
            }

            ret->next = NULL;
        }
        else
        {
            type_pool_node_delete(ret);
            ret = NULL;
        }
    }

    return ret;
}

struct type_pool_s* type_pool_new(int num, int element_size)
{
    struct type_pool_s* ret = (struct type_pool_s*)malloc(sizeof(struct type_pool_s));

    if(NULL != ret)
    {
        memset(ret, 0, sizeof(*ret));

        ret->head = type_pool_node_new(num, element_size+sizeof(struct type_pool_node_s*));
        if(NULL != ret->head)
        {
            ret->free_node = ret->tail = ret->head;
            ret->element_size = element_size;
        }
        else
        {
            type_pool_delete(ret);
            ret = NULL;
        }
    }

    return ret;
}

void type_pool_delete(struct type_pool_s* self)
{
    if(NULL != self)
    {
        struct type_pool_node_s* node = self->head;
        while(NULL != node)
        {
            struct type_pool_node_s* node_temp = node;
            node = node_temp->next;
            type_pool_node_delete(node_temp);
        }

        self->head = self->tail = self->free_node = NULL;
        free(self);
        self = NULL;
    }
}

char* type_pool_claim(struct type_pool_s* self)
{
    char* ret = NULL;
    struct type_pool_node_s* node = self->free_node;

    if(NULL != node)
    {
        ret = stack_pop(node->ptr_stack);
    }

    if(NULL == ret)
    {
        node = self->head;
        while(NULL == ret && NULL != node)
        {
            ret = stack_pop(node->ptr_stack);
            node = node->next;
        }
    }
    
    if(NULL != ret)
    {
        self->free_node = node;
        ret = *(char**)ret;
        ret += sizeof(struct type_pool_node_s*);
    }
    
    return ret;
}

void type_pool_reclaim(struct type_pool_s* self, char* data)
{
    struct type_pool_node_s* node = NULL;
    data -= sizeof(struct type_pool_node_s*);
    node = *(struct type_pool_node_s**)(data);
    stack_push(node->ptr_stack, &data);
    self->free_node = node;
}

void type_pool_increase(struct type_pool_s* self, int increase_num)
{
    struct type_pool_node_s* node = type_pool_node_new(increase_num, self->element_size+sizeof(struct type_pool_node_s*));
    
    if(NULL != node)
    {
        self->tail->next = node;
        self->tail = node;
        self->free_node = node;
    }
}
