//////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2024 Jeffrey K. Bienstadt
//
// This file is part of the brace C++ library.
//
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.
//////////////////////////////////////////////////////////////////////

/// \file text_encoding.h
///
/// \author Jeff Bienstadt
#ifndef BRACE_LIB_TEXTENCODING_INC
#define BRACE_LIB_TEXTENCODING_INC

#include <string_view>

namespace brace {
class TextEncoding
{
public:
    using CharacterMap  = int[256];

    /// \brief  Destroy a TextEncoding
    virtual ~TextEncoding()
    {}

    /// \brief  Retrieve the cononical name of this encoding.
    /// \return A const pointer to the encoding's canonical namme.
    virtual const char *canonical_name() const noexcept = 0;

    /// \brief  Determine if this encoding is known by the given namme.
    /// \param name an encoding name
    /// \return true if the encoding is known by the given name, false otherwise
    /// \details    The encoding "ISO-8859-1" is known also as "Latin-1".
    virtual bool aka(std::string_view name) const noexcept = 0;

#if 0
    /// \brief  Retrieve the CharacterMap for the encoding.
    /// \return the CharacterMap (pointer to an array of 255 integers)
    ///         for the encoding.
    virtual const CharacterMap &character_map() const noexcept = 0;
#endif

    /// \brief  Decode a multi-byte sequence to a Unicode scalar value
    /// \param bytes pointer to a byte sequence
    /// \return the Unicode scalar value represented by the byte sequence
    ///         if successful, -1 otherwise.
    virtual int decode(const unsigned char *bytes) const noexcept
    {
        return static_cast<int>(*bytes);
    }

    /// \brief  Encode the Unicode character ch into a byte sequence.
    /// \param ch       Unicode character to encode.
    /// \param bytes    Pointer to sequence of bytes to encode into.
    /// \param length   Length of the byte sequence
    /// \return The number of bytes used in the conversion.
    virtual int encode(int ch, unsigned char *bytes, int length) const noexcept
    {
        return 0;
    }

#if 0
    /// \brief
    /// \param bytes
    /// \return
    virtual int query_convert(const unsigned char *bytes) const noexcept
    {
        return static_cast<int>(*bytes);
    }
#endif

    /// \brief  Determine the length of a byte sequence.
    /// \param bytes    Pointer to byte sequence
    /// \param length   Maximum length of byte sequence
    /// \return Length of the byte sequence.
    virtual int sequence_length(const unsigned char *bytes, int length) const noexcept
    {
        return 1;
    }
};

}

#endif  // BRACE_LIB_TEXTENCODING_INC

