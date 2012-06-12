#ifndef _THREAD_H_INCLUDED_
#define _THREAD_H_INCLUDED_

#ifdef  __cplusplus
extern "C" {
#endif

typedef void (*thread_fun_pt)(void*);
struct thread_s;

struct thread_s* thread_new(thread_fun_pt func, void* param);
void thread_delete(struct thread_s* self);
void thread_wait(struct thread_s* self);
void thread_sleep(int milliseconds);

#ifdef  __cplusplus
}
#endif

#endif
