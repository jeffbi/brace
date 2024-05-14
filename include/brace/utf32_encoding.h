//////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2024 Jeffrey K. Bienstadt
//
// This file is part of the brace C++ library.
//
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.
//////////////////////////////////////////////////////////////////////

/// \file utf32_encoding.h
///
/// \author Jeff Bienstadt
#ifndef BRACE_LIB_UTF32_ENCODING_INC
#define BRACE_LIB_UTF32_ENCODING_INC

#include "brace/byteorder.h"
#include "brace/text_encoding.h"
#include "brace/string.h"

namespace brace {

class UTF32Encoding : TextEncoding
{
public:
    UTF32Encoding(Endian e = Endian::Native)
    {
        endian(e);
    }

    UTF32Encoding(int byte_order_mark)
    {
        endian(byte_order_mark);
    }

    UTF32Encoding(const UTF32Encoding &) = default;
    UTF32Encoding &operator =(const UTF32Encoding &) = default;
    UTF32Encoding(UTF32Encoding &&) = default;
    UTF32Encoding &operator =(UTF32Encoding &&) = default;

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
        uint32_t        uc;
        unsigned char  *p = reinterpret_cast<unsigned char *>(&uc);

        *p++ = *bytes++;
        *p++ = *bytes++;
        *p++ = *bytes++;
        *p++ = *bytes++;

        if (_flip)
            uc = byte_swap(uc);

        if (uc <= 0x10FFFF)
            return static_cast<int>(uc);
        else
            return -1;
    }

    /// \brief  Encode the Unicode character ch into a byte sequence.
    /// \param ch       Unicode character to encode.
    /// \param bytes    Pointer to sequence of bytes to  into.
    /// \param length   Length of the byte sequence
    /// \return The number of bytes used in the conversion.
    int encode(int ch, unsigned char *bytes, int length) const noexcept override
    {
        if (bytes && length >= 4)
        {
            uint32_t        ch1{_flip ? byte_swap(static_cast<uint32_t>(ch))
                                      : static_cast<uint32_t>(ch)};

            unsigned char  *p = reinterpret_cast<unsigned char*>(&ch1);
            *bytes++ = *p++;
            *bytes++ = *p++;
            *bytes++ = *p++;
            *bytes++ = *p++;
        }

        return 4;
    }

    int sequence_length(const unsigned char *bytes, int length) const noexcept override
    {
        return 4;
    }

private:
    bool    _flip;

    inline constexpr static const char *_names[] {
        "UTF-32",
        "UTF32"
    };

#if 0
    inline constexpr static CharacterMap _map {
    /* 00 */    -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4,
    /* 10 */    -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4,
    /* 20 */    -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4,
    /* 30 */    -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4,
    /* 40 */    -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4,
    /* 50 */    -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4,
    /* 60 */    -4, -4, -4, -4, -4, -4, -4, -4, -4, -, 0x01, 0x03, 0x484, -4, -4, -4, -4, -4, -4,
    /* 70 */    -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4,
    /* 80 */    -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4,
    /* 90 */    -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4,
    /* A0 */    -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4,
    /* B0 */    -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4,
    /* C0 */    -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4,
    /* D0 */    -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4,
    /* E0 */    -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4,
    /* F0 */    -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4,
    };
#endif
};

}

#endif  // BRACE_LIB_UTF32_ENCODING_INC
