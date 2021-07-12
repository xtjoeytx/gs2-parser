#include <cassert>
#include <cstdio>
#include "buffer.h"

Buffer::Buffer(size_t len)
    : buflen(len), readpos(0), writepos(0)
{
    buf = (uint8_t *)malloc(len);
}

Buffer::Buffer(Buffer&& o) noexcept
    : buf(std::move(o.buf)), buflen(o.buflen), readpos(o.readpos), writepos(o.writepos)
{
    o.buf = nullptr;
    //o.buflen = 0;
    //o.readpos = 0;
    //o.writepos = 0;
}

Buffer::~Buffer()
{
    free(buf);
}

void Buffer::resize(size_t len)
{
    buflen = (len > buflen ? len : buflen * 2);
    buf = (uint8_t *)realloc(buf, buflen);
    assert(buf);
}

void Buffer::read(char *dst, size_t len, size_t pos)
{
    memcpy(dst, buf + pos, len);
}

void Buffer::write(const char *src, size_t len)
{
    if (buflen < writepos + len)
        resize();

    memcpy(buf + writepos, src, len);
    writepos += len;
}

void Buffer::write(char val)
{
    if (buflen < writepos + 1)
        resize();

    buf[writepos++] = val;

    if (val == 0x0d) {
        printf("written\n");
    }
}
