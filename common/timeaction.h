#ifndef _TIMEACTION_H_INCLUDED_
#define _TIMEACTION_H_INCLUDED_

#ifdef  __cplusplus
extern "C" {
#endif

typedef void (*timeaction_handler_pt)(void* arg);

struct timeaction_mgr_s;

struct timeaction_mgr_s* timeaction_mgr_new(int num);
void timeaction_mgr_delete(struct timeaction_mgr_s* self);

void timeaction_mgr_schedule(struct timeaction_mgr_s* self);
int timeaction_mgr_add(struct timeaction_mgr_s* self, timeaction_handler_pt callback, int delay, void* arg);
void timeaction_mgr_del(struct timeaction_mgr_s* self, int id);

#ifdef  __cplusplus
}
#endif

#endif
