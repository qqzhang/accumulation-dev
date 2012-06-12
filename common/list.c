#include <string.h>
#include <stdlib.h>

#include "typepool.h"
#include "list.h"

struct list_s
{
    struct list_node_s head;
    struct list_node_s tail;

    struct type_pool_s* node_pool;
    struct type_pool_s* data_pool;

    int element_size;
};

struct list_s* list_new(int node_num, int element_size)
{
    struct list_s* ret = (struct list_s*)malloc(sizeof(struct list_s));

    if(NULL != ret)
    {
        memset(ret, 0, sizeof(*ret));

        ret->node_pool = type_pool_new(node_num, sizeof(struct list_node_s));
        ret->data_pool = type_pool_new(node_num, element_size);

        if(NULL != ret->node_pool && NULL != ret->data_pool)
        {
            ret->head.next = &(ret->tail);
            ret->tail.prior = &(ret->head);

            ret->element_size = element_size;
        }
        else
        {
            list_delete(ret);
            ret = NULL;
        }
    }

    return ret;
}

void list_delete(struct list_s* self)
{
    if(NULL != self)
    {
        if(NULL != self->node_pool)
        {
            type_pool_delete(self->node_pool);
            self->node_pool = NULL;
        }
        
        if(NULL != self->data_pool)
        {
            type_pool_delete(self->data_pool);
            self->data_pool = NULL;
        }

        self->element_size = 0;

        free(self);
        self = NULL;
    }
}

void list_push_back(struct list_s* self, const void* data)
{
    void* data_buffer = NULL;
    struct list_node_s* current_tail = NULL;
    struct list_node_s* node = (struct list_node_s*)type_pool_claim(self->node_pool);

    if(NULL == node)
    {
        return;
    }

    data_buffer = type_pool_claim(self->data_pool);
    if(NULL == data_buffer)
    {
        type_pool_reclaim(self->node_pool, (char*)node);
        return;
    }

    node->data = data_buffer;
    memcpy(data_buffer, data, self->element_size);

    current_tail = self->tail.prior;
    current_tail->next = node;

    node->prior = current_tail;
    node->next = &(self->tail);
    self->tail.prior = node;
}

struct list_node_s* list_erase(struct list_s* self, struct list_node_s* node)
{
    struct list_node_s* prior = node->prior;
    struct list_node_s* next = node->next;

    prior->next = next;
    next->prior = prior;

    type_pool_reclaim(self->node_pool, (char*)node);
    type_pool_reclaim(self->data_pool,(char*) node->data);
    return next;
}

struct list_node_s* list_begin(struct list_s* self)
{
    return (self->head.next);
}

const struct list_node_s* list_end(struct list_s* self)
{
    return &(self->tail);
}
