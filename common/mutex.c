#include <stdlib.h>
#include "platform.h"

#if defined PLATFORM_WINDOWS
#include <windows.h>
#else
#include <pthread.h>
#endif

#include "mutex.h"

struct mutex_s
{
#if defined PLATFORM_WINDOWS
    CRITICAL_SECTION    mutex;
#else
    pthread_mutex_t    mutex;
#endif
};

struct mutex_s* mutex_new()
{
    struct mutex_s* ret = (struct mutex_s*)malloc(sizeof(struct mutex_s));
    if(NULL != ret)
    {
        #if defined PLATFORM_WINDOWS
        InitializeCriticalSection(&ret->mutex);
        #else
        pthread_mutex_init(&ret->mutex, 0);
        #endif
    }

    return ret;
}

void mutex_delete(struct mutex_s* self)
{
    if(NULL != self)
    {
#if defined PLATFORM_WINDOWS
        DeleteCriticalSection(&self->mutex);
#else
        pthread_mutex_destroy(&self->mutex);
#endif

        free(self);
        self = NULL;
    }
}

void mutex_lock(struct mutex_s* self)
{
    #if defined PLATFORM_WINDOWS
    EnterCriticalSection(&self->mutex);
    #else
    pthread_mutex_lock(&self->mutex);
    #endif
}

void mutex_unlock(struct mutex_s* self)
{
    #if defined PLATFORM_WINDOWS
    LeaveCriticalSection(&self->mutex);
    #else
    pthread_mutex_unlock(&self->mutex);
    #endif
}
