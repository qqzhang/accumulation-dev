#ifndef _SOCKETLIBFUNCTION_H_INCLUDED_
#define _SOCKETLIBFUNCTION_H_INCLUDED_

#include <stdbool.h>
#include "socketlibtypes.h"

#ifdef  __cplusplus
extern "C" {
#endif

void socket_init();
void socket_destroy();
bool socket_nonblock(sock fd);
sock socket_connect(const char* server_ip, int port);
sock socket_listen(int port, int back_num);
void socket_close(sock fd);
const char* socket_getipstr(unsigned int ip);
int socket_send(sock fd, const char* buffer, int len);

#ifdef  __cplusplus
}
#endif

#endif
