#ifndef _FDSET_INCLUDED_H
#define _FDSET_INCLUDED_H

#include <stdbool.h>
#include "socketlibtypes.h"

enum CheckType
{
    ReadCheck = 0x1,
    WriteCheck = 0x2,
    ErrorCheck = 0x4,
};

#ifdef  __cplusplus
extern "C" {
#endif

struct fdset_s;

struct fdset_s* fdset_new();
fd_set* fdset_getresult(struct fdset_s* self, enum CheckType type);
void fdset_delete(struct fdset_s* self);
void fdset_add(struct fdset_s* self, sock fd, int type);
void fdset_del(struct fdset_s* self, sock fd, int type);
int fdset_poll(struct fdset_s* self, long overtime);
bool fdset_check(struct fdset_s* self, sock fd, enum CheckType type);

#ifdef  __cplusplus
}
#endif

#endif
