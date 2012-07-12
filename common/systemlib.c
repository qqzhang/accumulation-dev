#include "systemlib.h"
#include "platform.h"
// thanks eathena

#if defined PLATFORM_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <windows.h> // GetTickCount()
#else
#include <unistd.h>
#include <sys/time.h> // struct timeval, gettimeofday()
#endif

#include <time.h>
#include <stdio.h>
#include <stdlib.h>

unsigned int getnowtime()
{
#if defined PLATFORM_WINDOWS
    return GetTickCount();
#elif defined(ENABLE_RDTSC)
    return (unsigned int)((_rdtsc() - RDTSC_BEGINTICK) / RDTSC_CLOCK);
#elif (defined(_POSIX_TIMERS) && _POSIX_TIMERS > 0 && defined(_POSIX_MONOTONIC_CLOCK) /* posix compliant */) || (defined(__FreeBSD_cc_version) && __FreeBSD_cc_version >= 500005 /* FreeBSD >= 5.1.0 */)
    struct timespec tval;
    clock_gettime(CLOCK_MONOTONIC, &tval);
    return tval.tv_sec * 1000 + tval.tv_nsec / 1000000;
#else
    struct timeval tval;
    gettimeofday(&tval, NULL);
    return tval.tv_sec * 1000 + tval.tv_usec / 1000;
#endif
}

int getCpuNum()
{
    int processorNum = 1;
#if defined PLATFORM_WINDOWS
    SYSTEM_INFO    si;
    GetSystemInfo( &si );
    processorNum = si.dwNumberOfProcessors;
#else

    FILE * pFile = popen("cat /proc/cpuinfo | grep processor | wc -l", "r");
    if(NULL != pFile)
    {
        char buff[1024];
        fgets(buff, sizeof(buff), pFile);
        pclose(pFile);
        processorNum = atoi(buff);
    }
#endif

    return processorNum;
}

