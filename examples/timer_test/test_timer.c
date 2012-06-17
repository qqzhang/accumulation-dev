#include <stdio.h>
#include "../../common/timeaction.h"

static void my_handler(void* arg)
{
    printf("handler\n");
}
int main()
{
    struct timeaction_mgr_s* time_mgr = timeaction_mgr_new(10);
    int id = timeaction_mgr_add(time_mgr, my_handler, 10, NULL);
    
    timeaction_mgr_del(time_mgr, id);
    id = timeaction_mgr_add(time_mgr, my_handler, 10, NULL);
    
    while(1)
    {
        timeaction_mgr_schedule(time_mgr);
    }
    return 0;
}
