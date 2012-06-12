#include <stdlib.h>
#include <string.h>
#include "systemlib.h"
#include "stack.h"
#include "timeaction.h"

#define ERROR_COMPOSITOR_INDEX (-1)

struct timeaction_node_s
{
    int index;
    unsigned int left_time;
    void*   arg;
    timeaction_handler_pt    callback;
};

static void timeaction_node_init(struct timeaction_node_s* self, int index)
{
    self->index = index;
    self->left_time = 0;
    self->arg = NULL;
    self->callback = NULL;
}

struct timeaction_mgr_s
{
    int node_num;

    struct stack_s*    free_node_ids;
    struct timeaction_node_s*   node_array;
    int*    compositor_list;

    int     compositor_start;
    int     compositor_end;
    int     compositor_num;
};

static void timeaction_mgr_destroy(struct timeaction_mgr_s* self)
{
    if(NULL != self->free_node_ids)
    {
        stack_delete(self->free_node_ids);
        self->free_node_ids = NULL;
    }
    
    if(NULL != self->node_array)
    {
        free(self->node_array);
        self->node_array = NULL;
    }

    if(NULL != self->compositor_list)
    {
        free(self->compositor_list);
        self->compositor_list = NULL;
    }
}

static bool timeaction_mgr_init(struct timeaction_mgr_s* self, int num)
{
    struct timeaction_node_s* new_node_array = (struct timeaction_node_s*)malloc(num*sizeof(struct timeaction_node_s));
    int* new_compositor_list = (int*)malloc(num*sizeof(int));
    struct stack_s* new_freenode_ids = stack_new(num, sizeof(int));

    if(NULL != new_node_array && NULL != new_compositor_list && NULL != new_freenode_ids)
    {
        const int old_nodenum = self->node_num;
        int i = old_nodenum;

        if(NULL != self->free_node_ids)
        {
            void* index_ptr = NULL;
            struct stack_s* old_freenode_ids = self->free_node_ids;

            while(NULL != (index_ptr = stack_pop(old_freenode_ids)))
            {
                stack_push(new_freenode_ids, index_ptr);
            }
        }

        if(NULL != self->node_array)
        {
            memcpy(new_node_array, self->node_array, (old_nodenum*sizeof(struct timeaction_node_s)));
        }
        
        i = old_nodenum;
        for(; i < num; ++i)
        {
            timeaction_node_init(new_node_array+i, i);
            stack_push(new_freenode_ids, &i);
        }

        if(NULL != self->compositor_list)
        {
            memcpy(new_compositor_list, self->compositor_list, old_nodenum*sizeof(int*));
        }

        memset(new_compositor_list+old_nodenum, ERROR_COMPOSITOR_INDEX, sizeof(int)*(num-old_nodenum));

        timeaction_mgr_destroy(self);

        self->node_array = new_node_array;
        self->compositor_list = new_compositor_list;
        self->free_node_ids = new_freenode_ids;
        self->node_num = num;

        return true;
    }
    else
    {
        if(NULL != new_node_array)
        {
            free(new_node_array);
            new_node_array = NULL;
        }

        if(NULL != new_freenode_ids)
        {
            stack_delete(new_freenode_ids);
            new_freenode_ids = NULL;
        }

        if(NULL != new_compositor_list)
        {
            free(new_compositor_list);
            new_compositor_list = NULL;
        }
    }

    return false;
}

struct timeaction_mgr_s* timeaction_mgr_new(int num)
{
    struct timeaction_mgr_s* ret = (struct timeaction_mgr_s*)malloc(sizeof(struct timeaction_mgr_s));
    if(NULL != ret)
    {
        memset(ret, 0, sizeof(*ret));

        if(!timeaction_mgr_init(ret, num))
        {
            timeaction_mgr_delete(ret);
            ret = NULL;
        }
    }

    return ret;
}

void timeaction_mgr_delete(struct timeaction_mgr_s* self)
{
    if(NULL != self)
    {
        timeaction_mgr_destroy(self);
        free(self);
        self = NULL;
    }
}

static void timeaction_mgr_erase(struct timeaction_mgr_s* self, struct timeaction_node_s* node)
{
    self->compositor_list[self->compositor_start] = ERROR_COMPOSITOR_INDEX;
    ++(self->compositor_start);
    if(self->compositor_start == self->node_num)
    {
        self->compositor_start = 0;
    }

    --self->compositor_num;
    stack_push(self->free_node_ids, &node->index);
}

void timeaction_mgr_schedule(struct timeaction_mgr_s* self)
{
    unsigned int now = getnowtime();
    struct timeaction_node_s* node = NULL;
    timeaction_handler_pt callback = NULL;
    void* arg = NULL;

    while(true)
    {
        int index = self->compositor_list[self->compositor_start];
        if(ERROR_COMPOSITOR_INDEX == index)
        {
            break;
        }

        node = self->node_array+index;
        callback = node->callback;

        if(NULL == callback)
        {
            timeaction_mgr_erase(self, node);
        }
        else
        {
            if(node->left_time > now)
            {
                break;
            }
            
            arg = node->arg;
            timeaction_mgr_erase(self, node);
            (callback)(arg);
        }
    }
}

static void timeaction_mgr_compositor(struct timeaction_mgr_s* self, int* start_pos, struct timeaction_node_s* node)
{
    int low = 0;
    int high = self->compositor_num-1;
    int insert_pos = 0;

    while(low <= high)
    {
        int middle = (low+high)/2;
        if(node->left_time < self->node_array[start_pos[middle]].left_time)
        {
            high = middle+1;
        }
        else
        {
            low = middle+1;
        }
    }

    insert_pos = high+1;
    memmove(start_pos+insert_pos+1, start_pos+insert_pos, sizeof(int)*(self->compositor_num-insert_pos));
    start_pos[insert_pos] = node->index;
    ++self->compositor_num;
}

int timeaction_mgr_add(struct timeaction_mgr_s* self, timeaction_handler_pt callback, int delay, void* arg)
{
    int id = ERROR_COMPOSITOR_INDEX;
    int* free_id = NULL;
    struct timeaction_node_s* node = NULL;

    if(NULL == callback || delay <= 0)
    {
        return id;
    }
    
    free_id = (int*)stack_pop(self->free_node_ids);
    if(NULL == free_id)
    {
        timeaction_mgr_init(self, 2*self->node_num);
        free_id = (int*)stack_pop(self->free_node_ids);
    }

    if(NULL != free_id)
    {
        id = *free_id;
        node = self->node_array+id;
        node->callback = callback;
        node->arg = arg;
        node->left_time = (delay+getnowtime());

        timeaction_mgr_compositor(self, self->compositor_list+self->compositor_start, node);
        if(self->compositor_num+self->compositor_start == self->node_num)
        {
            memmove(self->compositor_list, self->compositor_list+self->compositor_start, sizeof(int)*self->compositor_num);
            self->compositor_start = 0;
            memset(self->compositor_list+self->compositor_num, ERROR_COMPOSITOR_INDEX, sizeof(int)*(self->node_num-self->compositor_num));
        }    
    }

    return id;
}

void timeaction_mgr_del(struct timeaction_mgr_s* self, int id)
{
    if(id >= 0 && id < self->node_num)
    {
        self->node_array[id].callback = NULL;
    }
}
