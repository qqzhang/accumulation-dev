#include <stdlib.h>
#include <string.h>

#include "event.h"

struct event_s
{
  event_handler_pt  callback;
  void* data;
};
struct event_handler_s
{
    int num;
    struct event_s    events[MAX_EVENT_HANDLER_NUM];
};

struct object_observer_s 
{
    int num;
    struct object_s*    observer[MAX_EVENT_OBSERVER_NUM];
};

struct object_s
{
    struct event_handler_s events[MAX_OBJECT_EVENT_NUM];            // 自身event处理器
    
    struct object_observer_s  observers[MAX_OBJECT_EVENT_NUM];     // 观察者列表
};

struct object_s* object_create()
{
    struct object_s* ret = (struct object_s*)malloc(sizeof(struct object_s));
    if(NULL != ret)
    {
        memset(ret->events, 0, sizeof(ret->events));
        memset(ret->observers, 0, sizeof(ret->observers));
    }
    
    return ret;
}

void object_insert_observer(struct object_s* root, int event_id, struct object_s* observer_obj)
{
    if(event_id >= 0 && event_id < MAX_OBJECT_EVENT_NUM)
    {
        struct object_observer_s* observer = &root->observers[event_id];
        int num = observer->num;

        if(num < MAX_EVENT_OBSERVER_NUM)
        {
            observer->observer[observer->num] = observer_obj;
            ++(observer->num);
        }
    }
}

void object_insert_handler(struct object_s* root, int event_id, event_handler_pt callback, void* event_data)
{
    if(event_id >= 0 && event_id < MAX_OBJECT_EVENT_NUM)
    {
        int num = root->events[event_id].num;

        if(num < MAX_EVENT_HANDLER_NUM)
        {
            struct event_s* event = &root->events[event_id].events[num];
            event->callback = callback;
            event->data = event_data;
            ++(root->events[event_id].num);
        }
    }
}

void object_handler(struct object_s* root, struct object_s* source, int event_id, void* arg)
{
    if(event_id >= 0 && event_id < MAX_OBJECT_EVENT_NUM)
    {
        struct event_handler_s* handlers = &(root->events[event_id]);
        int num = root->events[event_id].num;

        int i = 0;
        for(; i < num; ++i)
        {
            struct event_s* event = &handlers->events[i];
            event->callback(root, source, event->data, arg);
        }
    }
    
}

void object_nofify(struct object_s* root, int event_id, void* arg)
{
    if(event_id >= 0 && event_id < MAX_OBJECT_EVENT_NUM)
    {
        struct object_observer_s* observer = &root->observers[event_id];
        int num = observer->num;
    
        int i = 0;
        for(; i < num; ++i)
        {
            object_handler(observer->observer[i], root, event_id, arg);
        }
    }
}