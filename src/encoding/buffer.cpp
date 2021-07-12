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
}

/*
int main(int argc, char *argv[])
{
    Buffer buf;

    // const char *test = "test str";
    // buf.write(test, strlen(test));

    // char *tmp = new char[30];
    // buf.read(tmp, strlen(test), 0);
    // tmp[strlen(test)+1] = '\0';

    // printf("data: %s\n", tmp);

    uint16_t t = 17954;
    buf.Write<GraalShort>(t);

    auto out = buf.Read<GraalByte>(0);
    printf("Val: %d\n", out);
    out = buf.Read<GraalByte>(1);
    printf("Val: %d\n", out);

    auto out2 = buf.Read<GraalShort>(0);
    printf("Val: %d\n", out2);

    buf.Write<GraalString>("test str");

    auto eee = buf.Read<GraalString>(2);
    printf("string: %s\n", eee.c_str());

//////////////

    Buffer strBuffer;
    strBuffer.Write<GraalString>("hello");

    buf.write(strBuffer);

    eee = buf.Read<GraalString>(2);
    printf("string: %s\n", eee.c_str());

    eee = buf.Read<GraalString>(10);
    printf("string: %s\n", eee.c_str());


    return 0;
}
*/
