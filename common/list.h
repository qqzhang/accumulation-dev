#ifndef _LIST_H_INCLUDED_
#define _LIST_H_INCLUDED_

#ifdef  __cplusplus
extern "C" {
#endif

struct list_node_s;
struct list_s;

struct list_node_s
{
    void*   data;
    struct list_node_s* prior;
    struct list_node_s* next;
};

struct list_s* list_new(int node_num, int element_size);
void list_delete(struct list_s* self);
void list_push_back(struct list_s* self, const void* data);
struct list_node_s* list_erase(struct list_s* self, struct list_node_s* node);
struct list_node_s* list_begin(struct list_s* self);
const struct list_node_s* list_end(struct list_s* self);

#ifdef  __cplusplus
}
#endif

#endif
