#include <stdio.h>
#include <Windows.h>

#include "threadpool.h"
#include "mutex.h"
#include "thread.h"

static struct mutex_s* count_mutex = 0;
static int counter = 0;

static void showcounter()
{
    printf("############### ThreadID : %d\n", GetCurrentThreadId());
    printf("wait counter lock\n");
    mutex_lock(count_mutex);
    counter++;

    printf("current count : %d\n", counter);
    mutex_unlock(count_mutex);

    printf("un counter lock\n");
}

static void my_msg_fun(struct thread_pool_s* self, void* msg)
{
    //thread_sleep(1);
    showcounter();
}

int main()
{
    int i= 0;
    struct thread_pool_s* p = thread_pool_new(my_msg_fun, 3, 4000);
    count_mutex = mutex_new();
    thread_pool_start(p);
    
    thread_sleep(1000);

    while(1)
    {
        //thread_sleep(1);      why

        if(i < 3001)
        {
            thread_pool_pushmsg(p, (void*)3);
            i++;
        }
    }
    return 0;
}