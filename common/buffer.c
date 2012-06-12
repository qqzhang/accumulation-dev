#include <stdlib.h>
#include <string.h>

#include "buffer.h"

struct buffer_s
{
    char*   data;
    int data_len;

    int write_pos;
    int read_pos;
};

void buffer_delete(struct buffer_s* self)
{
    if(NULL != self)
    {
        if(NULL != self->data)
        {
            free(self->data);
            self->data = NULL;
        }

        free(self);
        self = NULL;
    }
}

struct buffer_s* buffer_new(int buffer_size)
{
    struct buffer_s* ret = (struct buffer_s*)malloc(sizeof(struct buffer_s));

    if(NULL != ret)
    {
        memset(ret, 0, sizeof(*ret));

        if(NULL != (ret->data = (char*)malloc(sizeof(char)*buffer_size)))
        {
            memset(ret->data, 0, sizeof(char)*buffer_size);
            ret->data_len = buffer_size;
            ret->read_pos = 0;
            ret->write_pos = 0;
        }
        else
        {
            buffer_delete(ret);
            ret = NULL;
        }
    }

    return ret;
}

void buffer_adjustto_head(struct buffer_s* self)
{
    int len = 0;

    if(self->read_pos > 0)
    {
        len = buffer_getreadvalidcount(self);
        if(len > 0 )
        {
            memmove(self->data, self->data+self->read_pos, len);
        }
        self->read_pos = 0;
        self->write_pos = len;
    }
}

void buffer_init(struct buffer_s* self)
{
    self->read_pos = 0;
    self->write_pos = 0;
    memset(self->data, 0, self->data_len);
}

int buffer_getwritepos(struct buffer_s* self)
{
    return self->write_pos;
}

int buffer_getreadpos(struct buffer_s* self)
{
    return self->read_pos;
}

void buffer_addwritepos(struct buffer_s* self, int value)
{
    int temp = self->write_pos + value;
    if(temp <= self->data_len)
    {
        self->write_pos = temp;
    }
}

void buffer_addreadpos(struct buffer_s* self, int value)
{
    int temp = self->read_pos + value;
    if(temp <= self->data_len)
    {
        self->read_pos = temp;
    }
}

int buffer_getreadvalidcount(struct buffer_s* self)
{
    return self->write_pos - self->read_pos;
}

int buffer_getwritevalidcount(struct buffer_s* self)
{
    return self->data_len - self->write_pos;
}

int buffer_getsize(struct buffer_s* self)
{
    return self->data_len;
}

char* buffer_getwriteptr(struct buffer_s* self)
{
    if(self->write_pos < self->data_len)
    {
        return self->data + self->write_pos;
    }
    else
    {
        return NULL;
    }
}

char* buffer_getreadptr(struct buffer_s* self)
{
    if(self->read_pos < self->data_len)
    {
        return self->data + self->read_pos;
    }
    else
    {
        return NULL;
    }
}

bool buffer_write(struct buffer_s* self, const char* data, int len)
{
    bool write_ret = true;

    if(buffer_getwritevalidcount(self) >= len)
    {
        // 不用移动缓冲区即可从后面全部写入
        memcpy(buffer_getwriteptr(self), data, len);
        buffer_addwritepos(self, len);
    }
    else
    {
        int left_len = self->data_len-buffer_getreadvalidcount(self);
        if(left_len >= len)
        {
            // 需要移动已写缓冲区到头部,再写入数据
            buffer_adjustto_head(self);
            buffer_write(self, data, len);
        }
        else
        {
            write_ret = false;
        }
    }

    return write_ret;
}
