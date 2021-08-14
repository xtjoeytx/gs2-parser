#include <cassert>
#include <cstdlib>
#include <cstring>

#include "buffer.h"

void Buffer::resize(size_t len)
{
	if (buf)
	{
		auto tmp = buf;
		buflen = (len > buflen ? len : buflen * 2);
		buf = (uint8_t *)realloc(buf, buflen);
		assert(buf);
		
		// silencing msvc warning C6308
		if (!buf)
			free(tmp);
	}
	else
	{
		if (len == 0)
			len = 128;
		
		buf = (uint8_t *)malloc(len);
		buflen = len;
	}
}

void Buffer::read(char *dst, size_t len, size_t pos) const
{
	if (buflen > pos + len) {
		memcpy(dst, buf + pos, len);
	}
}

void Buffer::write(const char *src, size_t len)
{
	if (buflen < writepos + len)
		resize(buflen + writepos + len);

    memcpy(buf + writepos, src, len);
    writepos += len;
}

void Buffer::write(char val)
{
    if (buflen < writepos + 1)
        resize();

    buf[writepos++] = val;
}
