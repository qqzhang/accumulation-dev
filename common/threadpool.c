#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>

#include "mutex.h"
#include "stack.h"
#include "thread.h"

#include "threadpool.h"

struct thread_pool_s
{
    bool    is_run;

    struct stack_s* msg_list;

    struct mutex_s* msg_lock;

    struct thread_s** work_threads;

    struct mutex_s* lock;
    struct thread_cond_s* cv;

    thread_msg_fun_pt   callback;
    int work_thread_num;
};

struct thread_pool_s* thread_pool_new(thread_msg_fun_pt callback, int thread_num, int msg_num)
{
    struct thread_pool_s* ret = (struct thread_pool_s*)malloc(sizeof(*ret));
    ret->msg_list = stack_new(msg_num,sizeof(void*));
    ret->msg_lock = mutex_new();

    ret->lock = mutex_new();
    ret->cv = thread_cond_new();

    ret->callback = callback;
    ret->work_thread_num = thread_num;
    ret->work_threads = (struct thread_s**)malloc(sizeof(struct thread_s*)*ret->work_thread_num);
    memset(ret->work_threads, 0, sizeof(struct thread_s*)*ret->work_thread_num);
    
    ret->is_run = true;

    return ret;
}

void thread_pool_delete(struct thread_pool_s* self)
{
    thread_pool_stop(self);

    stack_delete(self->msg_list);
    self->msg_list = NULL;

    mutex_delete(self->msg_lock);
    self->msg_lock = NULL;

    free(self->work_threads);

    mutex_delete(self->lock);
    self->lock = NULL;
    
    thread_cond_delete(self->cv);
    self->cv = 0;

    free(self);
    self = NULL;
}

// TODO::线程激活时只处理一条消息

// TODO::或许应该为线程池添加一个标志,表示是否一个线程激活时获取当前stack所有消息进行处理
// TODO::还是只处理一条消息

static void thread_pool_proc_onemsg(struct thread_pool_s* self)
{
    thread_msg_fun_pt callback = self->callback;
    void** msg_ptr = 0;

    mutex_lock(self->msg_lock);
    msg_ptr = (void**)stack_pop(self->msg_list);
    mutex_unlock(self->msg_lock);

    if(NULL != msg_ptr)
    {
        (*callback)(self, *msg_ptr);
    }
}

static void thread_pool_work(void* arg)
{
    struct thread_pool_s* self = (struct thread_pool_s*)arg;
    struct stack_s* msg_list = self->msg_list;
    thread_msg_fun_pt callback = self->callback;

    while(true)
    {
        if(stack_top(msg_list) <= 0)
        {
            // 没有消息,则等待条件变量触发
            mutex_lock(self->lock);

            if(!self->is_run)
            {
                mutex_unlock(self->lock);
                break;
            }

            thread_cond_wait(self->cv, self->lock);
            mutex_unlock(self->lock);
        }

        thread_pool_proc_onemsg(self);
    }
}

void thread_pool_start(struct thread_pool_s* self)
{
    if(NULL != self)
    {
        int i = 0;

        for(; i < self->work_thread_num; ++i)
        {
            self->work_threads[i] = thread_new(thread_pool_work, self);
        }
    }
}

void thread_pool_stop(struct thread_pool_s* self)
{
    if(NULL != self)
    {
        int i = 0;
        self->is_run = false;

        for(; i < self->work_thread_num; ++i)
        {
            if(NULL != self->work_threads[i])
            {
                thread_wait(self->work_threads[i]);
            }
        }

        i = 0;
        for(; i < self->work_thread_num; ++i)
        {
            if(NULL != self->work_threads[i])
            {
                thread_delete(self->work_threads[i]);
            }
        }

        memset(self->work_threads, 0, sizeof(struct thread_s*)*self->work_thread_num);
    }
}

void thread_pool_pushmsg(struct thread_pool_s* self, void* data)
{
    // 添加消息,并触发条件变量

    if(self->is_run)
    {
        mutex_lock(self->msg_lock);
        stack_push(self->msg_list, &data);
        mutex_unlock(self->msg_lock);

        thread_cond_signal(self->cv);
    }
}
