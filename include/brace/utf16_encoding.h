//////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2024 Jeffrey K. Bienstadt
//
// This file is part of the brace C++ library.
//
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.
//////////////////////////////////////////////////////////////////////

/// \file utf16_encoding.h
///
/// \author Jeff Bienstadt
#ifndef BRACE_LIB_UTF16_ENCODING_INC
#define BRACE_LIB_UTF16_ENCODING_INC

#include "brace/byteorder.h"
#include "brace/text_encoding.h"
#include "brace/string.h"

namespace brace {
class UTF16Encoding : TextEncoding
{
    UTF16Encoding(Endian e = Endian::Native)
    {
        endian(e);
    }

    UTF16Encoding(int byte_order_mark)
    {
        endian(byte_order_mark);
    }

    UTF16Encoding(const UTF16Encoding &) = default;
    UTF16Encoding &operator =(const UTF16Encoding &) = default;
    UTF16Encoding(UTF16Encoding &&) = default;
    UTF16Encoding &operator =(UTF16Encoding &&) = default;

    void endian(Endian e)
    {
#if BRACE_BYTE_ORDER == BRACE_BYTE_ORDER_BIG_ENDIAN
        _flip = e == Endian::Little;
#else
        _flip = e == Endian::Big;
#endif
    }

    void endian(int byte_order_mark)
    {
        _flip = byte_order_mark != 0xFEFF;
    }

    Endian endian()
    {
#if BRACE_BYTE_ORDER == BRACE_BYTE_ORDER_BIG_ENDIAN
        return _flip ? Endian::Little : Endian::Big;
#else
        return _flip ? Endian::Big : Endian::Little;
#endif
    }

    /// \brief  Retrieve the cononical name of this encoding.
    /// \return A const pointer to the encoding's canonical namme.
    const char *canonical_name() const noexcept override
    {
        return _names[0];
    }

    /// \brief  Determine if this encoding is known by the given namme.
    /// \param name an encoding name
    /// \return true if the encoding is known by the given name, false otherwise
    /// \details    The encoding "US-ASCII" is known also as "ASCII".
    bool aka(std::string_view name) const noexcept override
    {
        for (auto n : _names)
            if (ci_compare(name, std::string_view(n)) == 0)
                return true;

        return false;
    }

    /// \brief  Decode a multi-byte sequence to a Unicode scalar value
    /// \param bytes pointer to a byte sequence
    /// \return the Unicode scalar value represented by the UTF-8 byte sequence
    ///         if successful, -1 otherwise.
    int decode(const unsigned char *bytes) const noexcept override
    {
        uint16_t        uc;
        unsigned char  *p = (unsigned char *)&uc;
        *p++ = *bytes++;
        *p++ = *bytes++;

        if (_flip)
        {
            uc = brace::byte_swap(uc);
        }

        if (uc >= 0xD800 && uc < 0xDC00)
        {
            uint16_t    uc2;
            p = (unsigned char *)&uc2;
            *p++ = *bytes++;
            *p++ = *bytes++;

            if (_flip)
            {
                uc2 = brace::byte_swap(uc2);
            }

            if (uc2 >= 0xDC00 && uc2 < 0xE000)
            {
                return ((uc & 0x3FF) << 10) + (uc2 & 0x3FF) + 0x10000;
            }
            else
            {
                return -1;
            }
        }
        else
        {
            return uc;
        }
    }

    int encode(int ch, unsigned char *bytes, int length) const noexcept override
    {
        if (ch <= 0xFFFF)
        {
            if (bytes && length >= 2)
            {
                uint16_t    ch1 = _flip ? brace::byte_swap(static_cast<uint16_t>(ch))
                                        : static_cast<uint16_t>(ch);
                unsigned char  *p = reinterpret_cast<unsigned char *>(&ch1);

                *bytes++ = *p++;
                *bytes++ = *p++;
            }

            return 2;
        }
        else
        {
            if (bytes && length >= 4)
            {
                int         ch1 = ch - 0x10000;
                uint16_t    sp1 = 0xD800 + ((ch1 >> 10) & 0x3FF);
                uint16_t    sp2 = 0xDC00 + (ch1 & 0x3FF);

                if (_flip)
                {
                    sp1 = brace::byte_swap(sp1);
                    sp2 = brace::byte_swap(sp2);
                }
                unsigned char  *p = reinterpret_cast<unsigned char *>(&sp1);
                *bytes++ = *p++;
                *bytes++ = *p++;
                p = reinterpret_cast<unsigned char *>(&sp2);
                *bytes++ = *p++;
                *bytes++ = *p++;
            }

            return 4;
        }
    }

    int sequence_length(const unsigned char *bytes, int length) const noexcept override
    {
        if (length <= 1)
            return -1;

        uint32_t        cu;
        unsigned char  *p{reinterpret_cast<unsigned char *>(cu)};
        *p++ = *bytes++;
        *p++ = *bytes++;

        if (_flip)
            cu = byte_swap(cu);
        if (cu >= 0xD800 && cu <= 0xDC00)
            return 4;
        else
            return 2;
    }

private:
    bool    _flip;

    inline constexpr static const char *_names[] {
        "UTF-16",
        "UTF16"
    };
};

}

#endif  // BRACE_LIB_UTF16_ENCODING_INC
