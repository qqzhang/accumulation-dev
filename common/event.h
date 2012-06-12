#ifndef _EVENT_H_INCLUDED_
#define _EVENT_H_INCLUDED_

#define MAX_OBJECT_EVENT_NUM 20         // 对象拥有事件上限
#define MAX_EVENT_OBSERVER_NUM    20    // 每个事件最多拥有的观察者数量
#define MAX_EVENT_HANDLER_NUM 20        // 每个事件最大回调处理数

#ifdef  __cplusplus
extern "C" {
#endif

    struct object_s;
    struct event_handler_s;

    typedef void (*event_handler_pt)(struct object_s* root, struct object_s* source, void* data, void* arg);

    struct object_s* object_create();

    void object_insert_observer(struct object_s* root, int event_id, struct object_s* observer);
    void object_insert_handler(struct object_s* root, int event_id, event_handler_pt callback, void* event_data);

    void object_handler(struct object_s* root, struct object_s* source, int event_id, void* arg);
    void object_nofify(struct object_s* root, int event_id, void* arg);

#ifdef  __cplusplus
}
#endif


#endif
