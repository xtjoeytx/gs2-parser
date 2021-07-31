#pragma once

#ifndef GRAALENCODING_H
#define GRAALENCODING_H

#include <string>
#include "buffer.h"

namespace encoding
{
    struct Int16
    {
        using Val_Type = uint16_t;

        static void Write(Buffer& buf, Val_Type data)
        {
            uint8_t out[2];
            out[0] = (data >> 8) & 0xFF;
            out[1] = data & 0xFF;
            buf.write((char *)&out, 2);
        }

        static Val_Type Read(Buffer& buf, size_t pos)
        {
            char dst[2];
            buf.read(dst, 2, pos);
            return (dst[0] << 8) + dst[1];
        }
    };

    struct Int32
    {
        using Val_Type = uint32_t;

        static void Write(Buffer& buf, Val_Type data)
        {
            uint8_t out[4];
            out[0] = (data >> 24) & 0xFF;
            out[1] = (data >> 16) & 0xFF;
            out[2] = (data >> 8) & 0xFF;
            out[3] = data & 0xFF;
            buf.write((char *)&out, 4);
        }

        static Val_Type Read(Buffer& buf, size_t pos)
        {
            char dst[4];
            buf.read(dst, 4, pos);
            return (dst[0] << 24) + (dst[1] << 16) + (dst[2] << 8) + dst[3];
        }
    };
}

struct GraalByte
{
    using Val_Type = uint8_t;

    static void Write(Buffer& buf, Val_Type data)
    {
        if (data < 223)
            data += 32;
        buf.write((char *)&data, 1);
    }

    static Val_Type Read(Buffer& buf, size_t pos)
    {
        char dst[1];
        buf.read(dst, 1, pos);
        return dst[0] - 32;
    }
};

struct GraalShort
{
    using Val_Type = uint16_t;

    static void Write(Buffer& buf, Val_Type data)
    {
        Val_Type tmp = data;
        if (tmp > 28767)
            tmp = 28767;
        
        uint8_t out[2];
        out[0] = tmp >> 7;
        if (out[0] > 223)
            out[0] = 223;

        out[1] = (tmp - (out[0] << 7)) + 32;
        out[0] += 32;
        buf.write((char *)out, 2);
    }

    static Val_Type Read(Buffer& buf, size_t pos)
    {
        uint8_t dst[2];
        buf.read((char *)dst, 2, pos);
	    return (dst[0] << 7) + dst[1] - 0x1020;
    }
};

struct GraalInt24
{
    using Val_Type = uint32_t;

    static void Write(Buffer& buf, Val_Type data)
    {
        Val_Type tmp = data;
        if (tmp > 3682399)
            tmp = 3682399;
        
        uint8_t out[3];
        out[0] = tmp >> 14;
        if (out[0] > 223)
            out[0] = 223;
        tmp -= (out[0] << 14);

        out[1] = tmp >> 7;
        if (out[1] > 223)
            out[1] = 223;

        out[2] = (tmp - (out[1] << 7)) + 32;
        out[1] += 32;
        out[0] += 32;
        buf.write((char *)out, 3);
    }

    static Val_Type Read(Buffer& buf, size_t pos)
    {
        uint8_t dst[3];
        buf.read((char *)dst, 3, pos);
	    return (((dst[0] << 7) + dst[1]) << 7) + dst[2] - 0x81020;
    }
};

struct GraalInt32
{
    using Val_Type = uint32_t;

    static void Write(Buffer& buf, Val_Type data)
    {
        Val_Type tmp = data;
        if (tmp > 471347295)
            tmp = 471347295;
        
        uint8_t out[4];
        out[0] = tmp >> 21;
        if (out[0] > 223)
            out[0] = 223;
        tmp -= (out[0] << 21);

        out[1] = tmp >> 14;
        if (out[1] > 223)
            out[1] = 223;

        out[2] = tmp >> 7;
        if (out[2] > 223)
            out[2] = 223;

        out[3] = (tmp - (out[2] << 7)) + 32;
        out[2] += 32;
        out[1] += 32;
        out[0] += 32;
        buf.write((char *)out, 4);
    }

    static Val_Type Read(Buffer& buf, size_t pos)
    {
        uint8_t dst[4];
        buf.read((char *)dst, 4, pos);
	    return (((((dst[0] << 7) + dst[1]) << 7) + dst[2]) << 7) + dst[3] - 0x4081020;
    }
};

struct GraalString
{
    using Val_Type = std::string;

    static void Write(Buffer& buf, Val_Type data)
    {
        auto len = data.length();
        if (len > 223)
            len = 223;

        buf.Write<GraalByte>((uint8_t)len);
        buf.write(data.c_str(), len);
    }

    static Val_Type Read(Buffer& buf, size_t pos)
    {
        uint8_t len = buf.Read<GraalByte>(pos);

        std::string str;
        str.reserve(len);
        buf.read(str.data(), len, pos+1);
        return str;
    }
};

#endif
