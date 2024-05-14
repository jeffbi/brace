//////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2024 Jeffrey K. Bienstadt
//
// This file is part of the brace C++ library.
//
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.
//////////////////////////////////////////////////////////////////////

/// \file ascii.h
///
/// \author Jeff Bienstadt
#ifndef BRACE_LIB_ASCII_INC
#define BRACE_LIB_ASCII_INC

#include <cstdint>

namespace brace {
class Ascii
{
public:
    enum Properties
    {
        CONTROL     = 0x0001,
        SPACE       = 0x0002,
        PUNCT       = 0x0004,
        DIGIT       = 0x0008,
        HEXDIGIT    = 0x0010,
        ALPHA       = 0x0020,
        LOWER       = 0x0040,
        UPPER       = 0x0080,
        GRAPH       = 0x0100,
        PRINT       = 0x0200,
    };

    /// \brief  Determine whether a valus is within the ASCII range
    /// \param ch   character to check
    /// \return true if the character is within the ASCII range, false otherwise.
    static bool is_ascii(int ch)
    {
        return (static_cast<uint32_t>(ch) & 0xFFFFFF80) == 0;
    }

    /// \brief  Return the properties on an ASCII character.
    /// \param ch   character for which to return properties.
    /// \return The ASCII properties for the given ASCII character
    ///         if ch is within the ASCII range, zero otherwise
    static int properties(int ch)
    {
        if (is_ascii(ch))
            return char_props[ch];
        return 0;
    }

    /// \brief  Determine if the given character is within the ASCII range
    ///         and contains all the properties specified in props
    /// \param ch       character to check
    /// \param props    properties to check for
    /// \return true if ch is within the ASCII range and contains all of the
    ///         specified properties, otherwise return false.
    static bool has_properties(int ch, int props)
    {
        return (properties(ch) & props) == props;
    }

    /// \brief  Determine if the given character is within the ASCII range
    ///         and contains at least on of the specified properties
    /// \param ch       character to check
    /// \param props    property or properties to check for
    /// \return true if the given character is within the ASCII range and
    ///         contains at least on of the specified properties.
    static bool has_property(int ch, int props)
    {
        return (properties(ch) & props) != 0;
    }

    /// \brief Determine if a given character is a space (space, tab, etc).
    /// \param ch   character to check.
    /// \return true if the character is a space, false otherwise
    static bool is_space(int ch)
    {
        return has_properties(ch, SPACE);
    }

    /// \brief  Determine if a given character is a digit (0-9)
    /// \param ch character to check.
    /// \return true if the character is a digit, false otherwise/
    static bool is_digit(int ch)
    {
        return has_properties(ch, DIGIT);
    }

    /// \brief  Determine if a given character is a hexadecimal digit (0-9A-Fa-f)
    /// \param ch character to check.
    /// \return true if the character is a hexadecimal digit, false otherwise.
    static bool is_hexdigit(int ch)
    {
        return has_properties(ch, HEXDIGIT);
    }

    /// \brief  Determine if a given character is a punctuation character.
    /// \param ch character to check
    /// \return true if the character is a punctuation character, false otherwise.
    static bool is_punct(int ch)
    {
        return has_properties(ch, PUNCT);
    }

    /// \brief  Determine if a given character is a letter
    /// \param ch character to check.
    /// \return true if the character is a letter, false otherwise.
    static bool is_alpha(int ch)
    {
        return has_properties(ch, ALPHA);
    }

    /// \brief  Determine if a given character is a letter or a digit.
    /// \param ch character to check.
    /// \return true if the character is a letter or digit, false otherwise.
    static bool is_alphanumeric(int ch)
    {
        return has_property(ch, ALPHA | DIGIT);
    }

    /// \brief  Determine if a given character is a lower-case letter
    /// \param ch character to check
    /// \return true if the character is a lower-case letter, false otherwise.
    static bool is_lower(int ch)
    {
        return has_properties(ch, LOWER);
    }

    /// \brief  Determine if a given character is an upper-case letter.
    /// \param ch character to check
    /// \return true if the character is an upper-case letter, false otherwise.
    static bool is_upper(int ch)
    {
        return has_properties(ch, UPPER);
    }

    /// \brief  Determine if a given character is printable
    /// \param ch character to check.
    /// \return true if the character is printable, false otherwise.
    static bool is_printable(int ch)
    {
        return has_properties(ch, PRINT);
    }

    /// \brief  Convert an upper-case letter to a lower-case letter
    /// \param ch   character to convert. If this is not an upper-case letter,
    ///             no conversion takes place.
    /// \return the lower-case representation of the character, or the original
    ///         character if the character was not an upper-case letter.
    static int to_lower(int ch)
    {
        if (is_upper(ch))
            return ch | 0x20;
        return ch;
    }

    /// \brief  Convert a lower-case letter to an upper-case letter
    /// \param ch   character to convert. If this is not a lower-case letter,
    ///             no conversion takes place.
    /// \return the upper-case representation of the character, or the original
    ///         character if the character was not a lower-case letter.
    static int to_upper(int ch)
    {
        if (is_lower(ch))
            return ch & ~0x20;
        return ch;
    }

private:
    inline constexpr static int char_props[128] = {
    /* 00   */ CONTROL,
    /* 01   */ CONTROL,
    /* 02   */ CONTROL,
    /* 03   */ CONTROL,
    /* 04   */ CONTROL,
    /* 05   */ CONTROL,
    /* 06   */ CONTROL,
    /* 07   */ CONTROL,
    /* 08   */ CONTROL,
    /* 09   */ CONTROL | SPACE,
    /* 0A   */ CONTROL | SPACE,
    /* 0B   */ CONTROL | SPACE,
    /* 0C   */ CONTROL | SPACE,
    /* 0D   */ CONTROL | SPACE,
    /* 0E   */ CONTROL,
    /* 0F   */ CONTROL,
    /* 10   */ CONTROL,
    /* 11   */ CONTROL,
    /* 12   */ CONTROL,
    /* 13   */ CONTROL,
    /* 14   */ CONTROL,
    /* 15   */ CONTROL,
    /* 16   */ CONTROL,
    /* 17   */ CONTROL,
    /* 18   */ CONTROL,
    /* 19   */ CONTROL,
    /* 1A   */ CONTROL,
    /* 1B   */ CONTROL,
    /* 1C   */ CONTROL,
    /* 1D   */ CONTROL,
    /* 1E   */ CONTROL,
    /* 1F   */ CONTROL,
    /* 20   */ SPACE | PRINT,
    /* 21 ! */ PUNCT | GRAPH | PRINT,
    /* 22 " */ PUNCT | GRAPH | PRINT,
    /* 23 # */ PUNCT | GRAPH | PRINT,
    /* 24 $ */ PUNCT | GRAPH | PRINT,
    /* 25 % */ PUNCT | GRAPH | PRINT,
    /* 26 & */ PUNCT | GRAPH | PRINT,
    /* 27 ' */ PUNCT | GRAPH | PRINT,
    /* 28 ( */ PUNCT | GRAPH | PRINT,
    /* 29 ) */ PUNCT | GRAPH | PRINT,
    /* 2A * */ PUNCT | GRAPH | PRINT,
    /* 2B + */ PUNCT | GRAPH | PRINT,
    /* 2C , */ PUNCT | GRAPH | PRINT,
    /* 2D - */ PUNCT | GRAPH | PRINT,
    /* 2E . */ PUNCT | GRAPH | PRINT,
    /* 2F / */ PUNCT | GRAPH | PRINT,
    /* 30 0 */ DIGIT | HEXDIGIT | GRAPH | PRINT,
    /* 31 1 */ DIGIT | HEXDIGIT | GRAPH | PRINT,
    /* 32 2 */ DIGIT | HEXDIGIT | GRAPH | PRINT,
    /* 33 3 */ DIGIT | HEXDIGIT | GRAPH | PRINT,
    /* 34 4 */ DIGIT | HEXDIGIT | GRAPH | PRINT,
    /* 35 5 */ DIGIT | HEXDIGIT | GRAPH | PRINT,
    /* 36 6 */ DIGIT | HEXDIGIT | GRAPH | PRINT,
    /* 37 7 */ DIGIT | HEXDIGIT | GRAPH | PRINT,
    /* 38 8 */ DIGIT | HEXDIGIT | GRAPH | PRINT,
    /* 39 9 */ DIGIT | HEXDIGIT | GRAPH | PRINT,
    /* 3A : */ PUNCT | GRAPH | PRINT,
    /* 3B ; */ PUNCT | GRAPH | PRINT,
    /* 3C < */ PUNCT | GRAPH | PRINT,
    /* 3D = */ PUNCT | GRAPH | PRINT,
    /* 3E > */ PUNCT | GRAPH | PRINT,
    /* 3F ? */ PUNCT | GRAPH | PRINT,
    /* 40 @ */ PUNCT | GRAPH | PRINT,
    /* 41 A */ HEXDIGIT | ALPHA | UPPER | GRAPH | PRINT,
    /* 42 B */ HEXDIGIT | ALPHA | UPPER | GRAPH | PRINT,
    /* 43 C */ HEXDIGIT | ALPHA | UPPER | GRAPH | PRINT,
    /* 44 D */ HEXDIGIT | ALPHA | UPPER | GRAPH | PRINT,
    /* 45 E */ HEXDIGIT | ALPHA | UPPER | GRAPH | PRINT,
    /* 46 F */ HEXDIGIT | ALPHA | UPPER | GRAPH | PRINT,
    /* 47 G */ ALPHA | UPPER | GRAPH | PRINT,
    /* 48 H */ ALPHA | UPPER | GRAPH | PRINT,
    /* 49 I */ ALPHA | UPPER | GRAPH | PRINT,
    /* 4A J */ ALPHA | UPPER | GRAPH | PRINT,
    /* 4B K */ ALPHA | UPPER | GRAPH | PRINT,
    /* 4C L */ ALPHA | UPPER | GRAPH | PRINT,
    /* 4D M */ ALPHA | UPPER | GRAPH | PRINT,
    /* 4E N */ ALPHA | UPPER | GRAPH | PRINT,
    /* 4F O */ ALPHA | UPPER | GRAPH | PRINT,
    /* 50 P */ ALPHA | UPPER | GRAPH | PRINT,
    /* 51 Q */ ALPHA | UPPER | GRAPH | PRINT,
    /* 52 R */ ALPHA | UPPER | GRAPH | PRINT,
    /* 53 S */ ALPHA | UPPER | GRAPH | PRINT,
    /* 54 T */ ALPHA | UPPER | GRAPH | PRINT,
    /* 55 U */ ALPHA | UPPER | GRAPH | PRINT,
    /* 56 V */ ALPHA | UPPER | GRAPH | PRINT,
    /* 57 W */ ALPHA | UPPER | GRAPH | PRINT,
    /* 58 X */ ALPHA | UPPER | GRAPH | PRINT,
    /* 59 Y */ ALPHA | UPPER | GRAPH | PRINT,
    /* 5A Z */ ALPHA | UPPER | GRAPH | PRINT,
    /* 5B [ */ PUNCT | GRAPH | PRINT,
    /* 5C \ */ PUNCT | GRAPH | PRINT,
    /* 5D ] */ PUNCT | GRAPH | PRINT,
    /* 5E ^ */ PUNCT | GRAPH | PRINT,
    /* 5F _ */ PUNCT | GRAPH | PRINT,
    /* 60 ` */ PUNCT | GRAPH | PRINT,
    /* 61 a */ HEXDIGIT | ALPHA | LOWER | GRAPH | PRINT,
    /* 62 b */ HEXDIGIT | ALPHA | LOWER | GRAPH | PRINT,
    /* 63 c */ HEXDIGIT | ALPHA | LOWER | GRAPH | PRINT,
    /* 64 d */ HEXDIGIT | ALPHA | LOWER | GRAPH | PRINT,
    /* 65 e */ HEXDIGIT | ALPHA | LOWER | GRAPH | PRINT,
    /* 66 f */ HEXDIGIT | ALPHA | LOWER | GRAPH | PRINT,
    /* 67 g */ ALPHA | LOWER | GRAPH | PRINT,
    /* 68 h */ ALPHA | LOWER | GRAPH | PRINT,
    /* 69 i */ ALPHA | LOWER | GRAPH | PRINT,
    /* 6A j */ ALPHA | LOWER | GRAPH | PRINT,
    /* 6B k */ ALPHA | LOWER | GRAPH | PRINT,
    /* 6C l */ ALPHA | LOWER | GRAPH | PRINT,
    /* 6D m */ ALPHA | LOWER | GRAPH | PRINT,
    /* 6E n */ ALPHA | LOWER | GRAPH | PRINT,
    /* 6F o */ ALPHA | LOWER | GRAPH | PRINT,
    /* 70 p */ ALPHA | LOWER | GRAPH | PRINT,
    /* 71 q */ ALPHA | LOWER | GRAPH | PRINT,
    /* 72 r */ ALPHA | LOWER | GRAPH | PRINT,
    /* 73 s */ ALPHA | LOWER | GRAPH | PRINT,
    /* 74 t */ ALPHA | LOWER | GRAPH | PRINT,
    /* 75 u */ ALPHA | LOWER | GRAPH | PRINT,
    /* 76 v */ ALPHA | LOWER | GRAPH | PRINT,
    /* 77 w */ ALPHA | LOWER | GRAPH | PRINT,
    /* 78 x */ ALPHA | LOWER | GRAPH | PRINT,
    /* 79 y */ ALPHA | LOWER | GRAPH | PRINT,
    /* 7A z */ ALPHA | LOWER | GRAPH | PRINT,
    /* 7B { */ PUNCT | GRAPH | PRINT,
    /* 7C | */ PUNCT | GRAPH | PRINT,
    /* 7D } */ PUNCT | GRAPH | PRINT,
    /* 7E ~ */ PUNCT | GRAPH | PRINT,
    /* 7F   */ CONTROL
    };
};

}
#endif  // BRACE_LIB_ASCII_INC
