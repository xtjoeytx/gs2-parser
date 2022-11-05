#pragma once

#ifndef BUFFER_H
#define BUFFER_H

#include <cstdint>
#include <cstring>
#include <cstddef>
#include <cstdlib>
#include <utility>

class Buffer
{
    public:
		Buffer();
		Buffer(size_t len);
        Buffer(Buffer&& o) noexcept;
        Buffer(const Buffer&) = delete;
        ~Buffer();

        Buffer& operator=(const Buffer&) = delete;
        Buffer& operator= (Buffer&& o) noexcept;

        const uint8_t * buffer() const {
            return buf;
        }

        size_t length() const {
            return writepos;
        }

        size_t size() const {
            return buflen;
        }

        void setWritePos(size_t pos) {
            writepos = pos;
        }

        void read(char *dst, size_t len, size_t pos = 0) const;
        void write(const char *src, size_t len);
        void write(char val);
        void write(const Buffer& o);

        template<typename T>
        typename T::Val_Type Read(size_t pos) {
            return T::Read(*this, pos);
        }

        template<typename T>
        void Write(typename T::Val_Type val) {
            T::Write(*this, val);
        }

        template<typename T>
        void Write(typename T::Val_Type val, size_t pos) {
            auto tmp = writepos;
            writepos = pos;
            T::Write(*this, val);
            writepos = tmp;
        }

    private:
        void resize(size_t len = 0);

        uint8_t *buf;
        size_t buflen;
        size_t readpos, writepos;
};

inline Buffer::Buffer()
    : buf(nullptr), buflen(0), readpos(0), writepos(0)
{

}

inline Buffer::Buffer(size_t len)
    : Buffer()
{
    resize(len);
}

inline Buffer::Buffer(Buffer&& o) noexcept
{
    *this = std::move(o);
}

inline Buffer::~Buffer()
{
    if (buf)
    {
        free(buf);
    }
}

inline Buffer& Buffer::operator=(Buffer&& o) noexcept
{
	buf = o.buf;
    buflen = o.buflen;
    readpos = o.readpos;
    writepos = o.writepos;

    o.buf = nullptr;
    o.buflen = 0;
    o.readpos = 0;
    o.writepos = 0;
    return *this;
}

inline void Buffer::write(const Buffer& o)
{
    write((char *)o.buf, o.length());
}

#endif
