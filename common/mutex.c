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

struct thread_cond_s
{
#if defined PLATFORM_WINDOWS
    HANDLE  cond;
#else
    pthread_cond_t  cond;
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

struct thread_cond_s* thread_cond_new()
{
    struct thread_cond_s* ret = (struct thread_cond_s*)malloc(sizeof(*ret));
    if(NULL != ret)
    {
#if defined PLATFORM_WINDOWS
        ret->cond = CreateEvent(NULL, FALSE, FALSE, NULL );
#else
        pthread_cond_init(&ret->cond, 0);
#endif
    }

    return ret;
}

void thread_cond_delete(struct thread_cond_s* self)
{
    if(NULL != self)
    {
#if defined PLATFORM_WINDOWS
        CloseHandle(self->cond);
#else
        pthread_cond_destroy(&self->cond);
#endif

        free(self);
        self = NULL;
    }
}

void thread_cond_wait(struct thread_cond_s* self, struct mutex_s* mutex)
{
#if defined PLATFORM_WINDOWS
    WaitForSingleObject(self->cond, INFINITE);
#else
    pthread_cond_wait(&self->cond, &mutex->mutex);
#endif
}

void thread_cond_signal(struct thread_cond_s* self)
{
#if defined PLATFORM_WINDOWS
    SetEvent(self->cond);
#else
    pthread_cond_signal(&self->cond);
#endif
}

