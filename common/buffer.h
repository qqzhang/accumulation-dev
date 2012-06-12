#ifndef _BUFFER_H_INCLUDED
#define _BUFFER_H_INCLUDED

#ifdef  __cplusplus
extern "C" {
#endif

#include <stdbool.h>

struct buffer_s;

struct buffer_s* buffer_new(int buffer_size);
void buffer_delete(struct buffer_s* self);
void buffer_adjustto_head(struct buffer_s* self);
void buffer_init(struct buffer_s* self);

int buffer_getwritepos(struct buffer_s* self);
int buffer_getreadpos(struct buffer_s* self);

void buffer_addwritepos(struct buffer_s* self, int value);
void buffer_addreadpos(struct buffer_s* self, int value);

int buffer_getreadvalidcount(struct buffer_s* self);
int buffer_getwritevalidcount(struct buffer_s* self);
int buffer_getsize(struct buffer_s* self);

char* buffer_getwriteptr(struct buffer_s* self);
char* buffer_getreadptr(struct buffer_s* self);

bool buffer_write(struct buffer_s* self, const char* data, int len);

#ifdef  __cplusplus
}
#endif

#endif
