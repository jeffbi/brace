//////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2024 Jeffrey K. Bienstadt
//
// This file is part of the brace C++ library.
//
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.
//////////////////////////////////////////////////////////////////////

/// \file ascii_encoding.h
///
/// \author Jeff Bienstadt
#ifndef BRACE_LIB_ASCII_ENCODING_INC
#define BRACE_LIB_ASCII_ENCODING_INC

#include "brace/text_encoding.h"
#include "brace/string.h"

namespace brace {
class ASCIIEncoding : public TextEncoding
{
public:
    /// \brief  Construct an ASCIIEncoding object
    ASCIIEncoding()
    {}

    /// \brief  Destroy an ASCIIEncoding object.
    ~ASCIIEncoding()
    {}

    ASCIIEncoding(const ASCIIEncoding &) = default;
    ASCIIEncoding &operator =(const ASCIIEncoding &) = default;
    ASCIIEncoding(ASCIIEncoding &&) = default;
    ASCIIEncoding &operator =(ASCIIEncoding &&) = default;


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

#if 0
    /// \brief  Retrieve the CharacterMap for the encoding.
    /// \return the CharacterMap (pointer to an array of 255 integers)
    ///         for the encoding.
    const CharacterMap &character_map() const noexcept override
    {
        return _map;
    }
#endif

    /// \brief  Decode a multi-byte sequence to a Unicode scalar value
    /// \param bytes pointer to a byte sequence
    /// \return the Unicode scalar value represented by the byte sequence
    ///         if successful, -1 otherwise.
    int decode(const unsigned char *bytes) const noexcept override
    {
        return _map[*bytes];
    }

    /// \brief  Encode the Unicode character ch into a byte sequence.
    /// \param ch       Unicode character to encode.
    /// \param bytes    Pointer to sequence of bytes to encode into.
    /// \param length   Length of the byte sequence
    /// \return The number of bytes used in the conversion.
    int encode(int ch, unsigned char *bytes, int length) const noexcept override
    {
        if (ch >= 0 && ch <= 127)
        {
            if (bytes && length >= 1)
                *bytes = static_cast<unsigned char>(ch);
            return 1;
        }

        return 0;
    }

private:
    inline constexpr static const char *_names[] {
        "US-ASCII",
        "ASCII"
    };

    inline constexpr static CharacterMap _map {
    /* 00 */    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    /* 10 */    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
    /* 20 */    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
    /* 30 */    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
    /* 40 */    0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,
    /* 50 */    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F,
    /* 60 */    0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
    /* 70 */    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F,
    /* 80 */      -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
    /* 90 */      -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
    /* A0 */      -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
    /* B0 */      -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
    /* C0 */      -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
    /* D0 */      -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
    /* E0 */      -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
    /* F0 */      -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
    };
};

}

#endif  // BRACE_LIB_ASCII_ENCODING_INC
