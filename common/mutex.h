#ifndef _MUTEX_H_INCLUDED_
#define _MUTEX_H_INCLUDED_

#ifdef  __cplusplus
extern "C" {
#endif

struct mutex_s;
struct thread_cond_s;

struct mutex_s* mutex_new();
void mutex_delete(struct mutex_s* self);
void mutex_lock(struct mutex_s* self);
void mutex_unlock(struct mutex_s* self);

struct thread_cond_s* thread_cond_new();
void thread_cond_delete(struct thread_cond_s* self);
void thread_cond_wait(struct thread_cond_s* self, struct mutex_s* mutex);
void thread_cond_signal(struct thread_cond_s* self);

#ifdef  __cplusplus
}
#endif

#endif
