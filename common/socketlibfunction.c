#include "socketlibfunction.h"

#if defined PLATFORM_WINDOWS
static WSADATA g_WSAData;
#endif

void socket_init()
{
    #if defined PLATFORM_WINDOWS
    static bool WinSockIsInit = false;
    if(!WinSockIsInit)
    {
        WSAStartup(MAKEWORD(2,2), &g_WSAData);
        WinSockIsInit = true;
    }
    #endif
}

void socket_destroy()
{
    #if defined PLATFORM_WINDOWS
    WSACleanup();
    #endif
}

bool socket_nonblock(sock fd)
{
    int err;
    unsigned long ul = true;
    #if defined PLATFORM_WINDOWS
    err = ioctlsocket(fd, FIONBIO, &ul);
    #else
    err = ioctl(fd, FIONBIO, &ul);
    #endif

    return err != SOCKET_ERROR;
}

sock socket_connect(const char* server_ip, int port)
{
    struct sockaddr_in server_addr;
    sock clientfd = SOCKET_ERROR;

    socket_init();

    clientfd = socket(AF_INET, SOCK_STREAM, 0);

    if(SOCKET_ERROR != clientfd)
    {
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = inet_addr(server_ip);
        server_addr.sin_port = htons(port);

        while(connect(clientfd, (struct sockaddr*)&server_addr, sizeof(struct sockaddr)) < 0)
        {
            if(EINTR == sErrno)
            {
                continue;
            }
        }
    }

    return clientfd;
}

sock socket_listen(int port, int back_num)
{
    sock socketfd = SOCKET_ERROR;
    struct  sockaddr_in server_addr;
    int reuseaddr_value = 1;

    socket_init();

    socketfd = socket(AF_INET, SOCK_STREAM, 0);

    if(SOCKET_ERROR != socketfd)
    {
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port);
        server_addr.sin_addr.s_addr = INADDR_ANY;

        setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, (const char *)&reuseaddr_value , sizeof(int));

        if(SOCKET_ERROR == bind(socketfd, (struct sockaddr*)&server_addr, sizeof(struct sockaddr)) ||
            SOCKET_ERROR == listen(socketfd, back_num))
        {
            socket_close(socketfd);
            socketfd = SOCKET_ERROR;
        }
    }
    
    
    return socketfd;
}

void socket_close(sock fd)
{
    #if defined PLATFORM_WINDOWS
    closesocket(fd);
    #else
    close(fd);
    #endif
}

const char* socket_getipstr(unsigned int ip)
{
    struct in_addr addr;
    addr.s_addr = htonl(ip);
    return inet_ntoa(addr);
}

int socket_send(sock fd, const char* buffer, int len)
{
    int transnum = send(fd, buffer, len, 0);
    if(transnum < 0)
    {
        if(S_EWOULDBLOCK == sErrno)
        {
            transnum = 0;
        }
    }

    // send error if transnum < 0
    return transnum;
}
