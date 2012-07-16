#include "thread.h"
#include "mutex.h"
#include "epollserver.h"
#include "server.h"
#include <stdio.h>
#include <signal.h>
#include <errno.h>

const static int HTTP_PORT = 4002;
int index_array[1024];
int index_num;
static struct mutex_s* m = NULL;

static struct mutex_s* all_recved_lock = NULL;
static int all_recved = 0;

static void add_all_recved(int value)
{
        mutex_lock(all_recved_lock);
        all_recved+=value;
        mutex_unlock(all_recved_lock);
}

void insert_index(int index)
{
        mutex_lock(m);
        index_array[index_num] = index;
        index_num++;
        mutex_unlock(m);
}

void my_logic_on_enter_pt(struct server_s* self, int index)
{
     printf("new client, index:%d\n", index);
     insert_index(index);
}
 
 void my_logic_on_close_pt(struct server_s* self, int index)
 {
	printf("client is closed in logic \n");
   
 }
 
 int my_logic_on_recved_pt(struct server_s* self, int index, const char* buffer, int len)
 {
    if(len > 0)
    {
        int temp_index = 0;

        for(; temp_index < index_num; ++temp_index)
        {
            server_send(self, index_array[temp_index], buffer, len);
        }
         
        add_all_recved(len);
    }
    
    return len;
}

static void show_recved_len(void* arg)
{
    while(true)
    {
        printf("all_recved is %d\n", all_recved);
        thread_sleep(1000);
    }
}
void test()
{
    struct server_s* server = epollserver_create(HTTP_PORT, 1024, 150, 150);
    printf("EAGAIN is %d\n", EAGAIN);
    printf("ECONNRESET is %d\n", ECONNRESET);
    
    getchar();
    server_start(server, my_logic_on_enter_pt, my_logic_on_close_pt, my_logic_on_recved_pt);
    getchar();
}

static void haha()
{
    printf("xxxxx\n");
}

int main()
{
    struct thread_s* ttt = thread_new(show_recved_len, NULL);
    index_num = 0;
    m  = mutex_new();
    all_recved_lock = mutex_new();
    test();

    while(true)
    {
        thread_sleep(1);
    }

    return 0;
}
