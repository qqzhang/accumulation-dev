#include "platform.h"
#if defined PLATFORM_WINDOWS
#include <windows.h>
#else
#include <pthread.h>
#include <unistd.h>
#endif

#include <stdlib.h>

#include "thread.h"

#if defined PLATFORM_WINDOWS
typedef DWORD thread_id;
#else
typedef pthread_t thread_id;
#endif

struct thread_s
{
    thread_fun_pt   func;
    void*   param;
    thread_id id;
#if defined PLATFORM_WINDOWS
    HANDLE  fd;
#else
#endif
};

#if defined PLATFORM_WINDOWS
DWORD static WINAPI fummy_run( void* param)
#else
void* fummy_run( void* param)
#endif
{
    struct thread_s* data = (struct thread_s*)param;
    data->func(data->param);
    return 0;
}

struct thread_s* thread_new(thread_fun_pt func, void* param)
{
    struct thread_s* data = (struct thread_s*)malloc(sizeof(struct thread_s));
    if(NULL != data)
    {
        data->func = func;
        data->param = param;

#if defined PLATFORM_WINDOWS
        data->fd = CreateThread( NULL, 0, fummy_run, data, 0, (PDWORD)&(data->id));
#else
        pthread_create( &(data->id), 0, fummy_run, data);
#endif
    }

    return data;
}

void thread_delete(struct thread_s* self)
{
    thread_wait(self);
    free(self);
    self = NULL;
}

void thread_wait(struct thread_s* self)
{
    if(0 != self->fd)
    {
#if defined PLATFORM_WINDOWS
        WaitForSingleObject(self->fd, INFINITE);
        CloseHandle(self->fd);
#else
        pthread_join(self->id, NULL);
#endif

        self->fd = 0;
    }
}

void thread_sleep(int milliseconds)
{
#if defined PLATFORM_WINDOWS
    Sleep(milliseconds);
#else
    usleep(milliseconds * 1000 );
#endif
}
