#include <stdio.h>
#include "socketlibfunction.h"
#include "thread.h"

void send_thread(void* arg)
{
    sock s = socket_connect("127.0.0.1", 4002);
    int len = 0;
    printf("connect sucesss\n");
    while(true)
    {
        printf("开发发送\n");
        len = socket_send(s, "dzwdzwdzwdzwdzwdzwdzwdzwdzwdzwdzw", 33);
        printf("send len is %d\n", len);
        thread_sleep(10);
    }
}

int main()
{
    int i = 0;
    struct thread_s* thread = NULL;
    for(; i < 100; ++i)
    {
        thread = thread_new(send_thread, NULL);
    }
    
    getchar();
    return 0;
}
