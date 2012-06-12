#ifndef _MUTEX_H_INCLUDED_
#define _MUTEX_H_INCLUDED_

#ifdef  __cplusplus
extern "C" {
#endif

struct mutex_s;

struct mutex_s* mutex_new();
void mutex_delete(struct mutex_s* self);
void mutex_lock(struct mutex_s* self);
void mutex_unlock(struct mutex_s* self);

#ifdef  __cplusplus
}
#endif

#endif
