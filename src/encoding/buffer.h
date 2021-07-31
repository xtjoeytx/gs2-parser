#pragma once

#ifndef BUFFER_H
#define BUFFER_H

#include <cstdint>

class Buffer
{
    public:
        Buffer(size_t len = 128);
        Buffer(Buffer&& o) noexcept;
        Buffer(const Buffer&) = delete;
        ~Buffer();
        
        Buffer& operator=(const Buffer&) = delete;
        
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

        void read(char *dst, size_t len, size_t pos = 0);
        void write(const char *src, size_t len);
        void write(char val);
        void write(Buffer& o);

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

inline void Buffer::write(Buffer& o)
{
    write((char *)o.buf, o.length());
}

#endif
