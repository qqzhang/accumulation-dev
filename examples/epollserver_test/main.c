#include "thread.h"
#include "epollserver.h"
#include "server.h"
#include <stdio.h>
// ¼àÌý¶Ë¿Ú4000
const static int HTTP_PORT = 4000;

 void my_logic_on_enter_pt(struct server_s* self, int index)
 {
     printf("new client, index:%d\n", index);
 }
 void my_logic_on_close_pt(struct server_s* self, int index)
 {
	printf("client is closed in logic \n");
   
 }
 int my_logic_on_recved_pt(struct server_s* self, int index, const char* buffer, int len)
 {
     printf("recv data is :\n %s\n len:%d\n", buffer, len);
     //server_send(self, index, buffer, len);
    return len;
 }

void test()
{
    struct server_s* server = epollserver_create(4000, 1024, 50, 50);
    server_start(server, my_logic_on_enter_pt, my_logic_on_close_pt, my_logic_on_recved_pt);
    getchar();
}

int main()
{
    test();

    while(true)
    {
        thread_sleep(1);
    }

    return 0;
}
