#include "platform.h"

#ifdef PLATFORM_LINUX
#include <sys/epoll.h>
#ifndef EPOLLRDHUP
    #define EPOLLRDHUP (0x2000)
    #endif
#endif

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "stack.h"
#include "mutex.h"
#include "socketlibtypes.h"
#include "socketlibfunction.h"
#include "thread.h"
#include "threadpool.h"
#include "server_private.h"
#include "buffer.h"

#define EPOLL_WORKTHREAD_NUM (1)    /*  epoll_wait线程个数; recv线程池的工作线程个数  */

#define MAX_EVENTS (50)

struct epollserver_s;

struct session_s
{
    struct epollserver_s*   epollserver;
    int index;
    sock fd;

    bool    can_send;               /*  是否可写    */
    struct mutex_s*  send_mutex;    /*  TODO::考虑使用自旋锁   */
    struct buffer_s*    send_buffer;
    struct buffer_s*    recv_buffer;
};

struct epollserver_s
{
    struct server_s base;
    int epoll_fd;
    int listen_port;

    int session_recvbuffer_size;
    int session_sendbuffer_size;

    int max_num;
    struct session_s*   sessions;

    struct stack_s* freelist;
    int freelist_num;
    struct mutex_s* freelist_mutex;             /*  TODO::考虑使用自旋锁   */

    struct thread_s*    listen_thread;
    struct thread_s**  epoll_threads;           /*  epoll_wait线程组   */
    struct thread_pool_s*   recv_thread_pool;   /*  recv线程池(处理epoll_wait的可读事件) */
};

#ifdef PLATFORM_LINUX
static void epoll_add_event(struct epollserver_s* epollserver, sock fd, struct session_s* client, uint32_t events)
{
    struct epoll_event ev = { 0, { 0 }};
    ev.events = events;

    ev.data.fd = fd;
    ev.data.ptr = client;
    epoll_ctl(epollserver->epoll_fd, EPOLL_CTL_ADD, fd, &ev);
}
#endif

static void epoll_handle_newclient(struct epollserver_s* epollserver, sock client_fd)
{
    if(epollserver->freelist_num > 0)
    {
        struct session_s*   session = NULL;

        mutex_lock(epollserver->freelist_mutex);

        {
            struct session_s** ppsession = (struct session_s**)stack_pop(epollserver->freelist);
            if(NULL != ppsession)
            {
                session = *ppsession;
                epollserver->freelist_num--;
            }
        }

        mutex_unlock(epollserver->freelist_mutex);

        if(NULL != session)
        {
            struct server_s* server = &epollserver->base;
            session->fd = client_fd;
            buffer_init(session->recv_buffer);
            buffer_init(session->send_buffer);
            socket_nonblock(client_fd);
            session->can_send = true;

            #ifdef PLATFORM_LINUX
            epoll_add_event(epollserver, client_fd, session, EPOLLET | EPOLLIN | EPOLLOUT);
            #endif

            (*(server->logic_on_enter))(server, session->index);
        }
        else
        {
            printf("没有可用资源\n");
        }
    }
}

static void epollserver_handle_sessionclose(struct session_s* session)
{
    struct epollserver_s* epollserver = session->epollserver;

    mutex_lock(epollserver->freelist_mutex);

    if(SOCKET_ERROR != session->fd)
    {
#ifdef PLATFORM_LINUX
        epoll_ctl(epollserver->epoll_fd, EPOLL_CTL_DEL, session->fd, NULL);
#endif
        socket_close(session->fd);
        session->fd = SOCKET_ERROR; 
        stack_push(epollserver->freelist, &session);
        epollserver->freelist_num++;
    }

    mutex_unlock(epollserver->freelist_mutex);

}

/*  TODO::考虑是否根据当前剩余session的个数来进行accept   */
static void epoll_listen_thread(void* arg)
{
    struct epollserver_s* epollserver = (struct epollserver_s*)arg;
    
    sock client_fd = SOCKET_ERROR;
    struct sockaddr_in socketaddress;
    socklen_t size = sizeof(struct sockaddr);

    sock listen_fd = socket_listen(epollserver->listen_port, 25);

    for(;;)
    {
        while((client_fd = accept(listen_fd, (struct sockaddr*)&socketaddress, &size)) < 0)
        {
            if(EINTR == sErrno)
            {
                continue;
            }
        }

        if(SOCKET_ERROR != client_fd)
        {
            epoll_handle_newclient(epollserver, client_fd);
        }
    }
}

/*  发送内置缓冲区未发送的数据 */
static void epollserver_send_olddata(struct session_s* session)
{
    int send_len = 0;
    struct buffer_s*    send_buffer = session->send_buffer;
    int oldlen = buffer_getreadvalidcount(send_buffer);
        
    if(oldlen > 0)
    {
        send_len = socket_send(session->fd, buffer_getreadptr(send_buffer), oldlen);

        if(send_len > 0)
        {
            buffer_addwritepos(send_buffer, send_len);
        }
        
        session->can_send = (oldlen == oldlen);
    }
    
    return;
}

/*  优先发送内置缓冲区未发送的数据,然后发送指定数据 */
static int epollserver_senddata(struct session_s* session, const char* data, int len)
{
    int send_len = 0;
    int oldlen = 0;
    int oldsendlen = 0;
    struct buffer_s*    send_buffer = session->send_buffer;
    bool can_send = session->can_send;
    
    mutex_lock(session->send_mutex);
    
    if(can_send)    /*  如果可写则发送内置缓冲区数据,然后仍然可写则继续发送指定数据  */
    {
        epollserver_send_olddata(session);
        
        if(session->can_send)
        {
            send_len = socket_send(session->fd, data, len);
        }
        
        if(send_len < len)
        {
            session->can_send = false;  /*  设置为不可写  */
        }
    }
    
    if(send_len < len)
    {
        if(buffer_write(send_buffer, data+send_len, len-send_len))
        {
            send_len = len;
        }
    }
    
    mutex_unlock(session->send_mutex);

    return send_len;
}

static void epoll_handle_onoutevent(struct session_s* session)
{
    session->can_send = true;
    
    if(buffer_getreadvalidcount(session->send_buffer) > 0)
    {
        mutex_lock(session->send_mutex);
        epollserver_send_olddata(session);
        mutex_unlock(session->send_mutex);
    }
}

static void epoll_work_thread(void* arg)
{
    #ifdef PLATFORM_LINUX

    struct epollserver_s* epollserver = (struct epollserver_s*)arg;
    struct server_s* server = &epollserver->base;
    int nfds = 0;
    int i = 0;
    int epollfd = epollserver->epoll_fd;
    struct epoll_event events[MAX_EVENTS];
    uint32_t event_data = 0;

    for(;;)
    {
        nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);

        printf("recv %d event\n", nfds);
        if(-1 == nfds)
        {
            break;
        }

        for(i = 0; i < nfds; ++i)
        {
            struct session_s*   session = (struct session_s*)(events[i].data.ptr);
            event_data = events[i].events;

            if(event_data & EPOLLIN)
            {
                thread_pool_pushmsg(epollserver->recv_thread_pool, session);
            }

            if(event_data & EPOLLOUT)
            {
                epoll_handle_onoutevent(session);
            }
        }
    }

    #endif
}



static void epoll_recvdata_callback(struct thread_pool_s* self, void* msg)
{
    struct session_s*   session = (struct session_s*)msg;
    struct buffer_s*    recv_buffer = session->recv_buffer;
    struct server_s*    server = &(session->epollserver->base);

    bool is_close = false;

    for(;;)
    {
        int can_recvlen = 0;
        int recv_len = 0;

        if(buffer_getwritevalidcount(recv_buffer) <= 0)
        {
            buffer_adjustto_head(recv_buffer);
        }

        can_recvlen = buffer_getwritevalidcount(recv_buffer);

        if(can_recvlen <= 0)
        {
            break;
        }

        recv_len = recv(session->fd, buffer_getwriteptr(recv_buffer), can_recvlen, 0);
        if(0 == recv_len)
        {
            is_close = true;
            break;
        }
        else if(SOCKET_ERROR == recv_len)
        {
            is_close = (S_EWOULDBLOCK != sErrno);
            break;
        }
        else
        {
            buffer_addwritepos(recv_buffer, recv_len);
        }
    }

    if(is_close)
    {
        epollserver_handle_sessionclose(session);
        (*server->logic_on_close)(server, session->index);
    }
    else
    {
        int proc_len = (*server->logic_on_recved)(server, session->index, buffer_getreadptr(recv_buffer), buffer_getreadvalidcount(recv_buffer));
        buffer_addreadpos(recv_buffer, proc_len);
    }
}

static void epollserver_start_callback(
    struct server_s* self,
    logic_on_enter_pt enter_pt,
    logic_on_close_pt close_pt,
    logic_on_recved_pt   recved_pt
    )
{
    struct epollserver_s* epollserver = (struct epollserver_s*)self;

    self->logic_on_enter = enter_pt;
    self->logic_on_close = close_pt;
    self->logic_on_recved = recved_pt;

    epollserver->freelist = stack_new(epollserver->max_num, sizeof(struct session_s*));
    epollserver->freelist_num = epollserver->max_num;
    epollserver->freelist_mutex = mutex_new();

    epollserver->sessions = (struct session_s*)malloc(sizeof(struct session_s)*epollserver->max_num);
    
    {
        int i = 0;
        for(; i < epollserver->max_num; ++i)
        {
            struct session_s* session = epollserver->sessions+i;
            session->index = i;
            session->epollserver = epollserver;
            session->fd = SOCKET_ERROR;

            session->send_mutex = mutex_new();
            session->send_buffer = buffer_new(epollserver->session_sendbuffer_size);
            session->recv_buffer = buffer_new(epollserver->session_recvbuffer_size);

            stack_push(epollserver->freelist, &session);
        }
    }

    /*  开启epoll_wait线程组 */
    epollserver->epoll_threads = (struct thread_s**)malloc(sizeof(struct thread_s*)*EPOLL_WORKTHREAD_NUM);

    {
        int i = 0;
        for(; i < EPOLL_WORKTHREAD_NUM; ++i)
        {
            epollserver->epoll_threads[i] = thread_new(epoll_work_thread, epollserver);
        }
    }

    /*  开启recv线程池   */
    epollserver->recv_thread_pool = thread_pool_new(epoll_recvdata_callback, EPOLL_WORKTHREAD_NUM, 1024);
    thread_pool_start(epollserver->recv_thread_pool);

    /*  开启监听线程  */
    epollserver->listen_thread = thread_new(epoll_listen_thread, epollserver);
}

static void epollserver_stop_callback(struct server_s* self)
{
}

static void epollserver_closesession_callback(struct server_s* self, int index)
{
    struct epollserver_s* epollserver = (struct epollserver_s*)self;
    
    if(index >= 0 && index < epollserver->max_num)
    {
        struct session_s* session = epollserver->sessions+index;
        epollserver_handle_sessionclose(session);
    }
}

static int epollserver_send_callback(struct server_s* self, int index, const char* data, int len)
{
    struct epollserver_s* epollserver = (struct epollserver_s*)self;
    int send_len = -1;

    if(index >= 0 && index < epollserver->max_num)
    {
        struct session_s* session = epollserver->sessions+index;
        send_len = epollserver_senddata(session, data, len);
    }
    
    return send_len;
}

struct server_s* epollserver_create(
    int port, 
    int max_num,
    int session_recvbuffer_size,
    int session_sendbuffer_size)
{
    struct epollserver_s* epollserver = (struct epollserver_s*)malloc(sizeof(*epollserver));
    memset(epollserver, 0, sizeof(*epollserver));

    epollserver->sessions = NULL;
    epollserver->max_num = max_num;
    epollserver->listen_port = port;

    epollserver->base.start_pt = epollserver_start_callback;
    epollserver->base.stop_pt = epollserver_stop_callback;
    epollserver->base.closesession_pt = epollserver_closesession_callback;
    epollserver->base.send_pt = epollserver_send_callback;

    epollserver->session_recvbuffer_size = session_recvbuffer_size;
    epollserver->session_sendbuffer_size = session_sendbuffer_size;

#ifdef PLATFORM_LINUX
    epollserver->epoll_fd = epoll_create(1);
#endif

    return &epollserver->base;
}
