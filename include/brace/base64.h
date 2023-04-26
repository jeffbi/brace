//////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2023 Jeffrey K. Bienstadt
//
// This file is part of the brace C++ library.
//
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.
//////////////////////////////////////////////////////////////////////

/// \file base64.h
///
/// \author Jeff Bienstadt
#ifndef BRACE_LIB_BASE64_INC
#define BRACE_LIB_BASE64_INC

#include <algorithm>
#include <array>
#include <cstdint>
#include <iterator>
#include <optional>
#include <ostream>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

#include "binistream.h"
#include "binostream.h"
#include "parseerror.h"


namespace brace {

/// \brief  The Base64 is a base class for the Base64 and Base64Url encoding
///         and decoding classes.
class Base64Base
{
protected:

    /// \brief  Default construct a Base64Base object. This constructor
    ///         is protected.
    Base64Base()
    {}

private:
    /// \brief  Determine if ch is a valid Base64-encoded character.
    /// \param ch       The character to be checked.
    /// \param alphabet Pointer to the alphabet to be used.
    /// \return \c true if the character is valid, \c false otherwise.
    static bool is_valid_character(char ch, const char *alphabet)
    {
        if (   ((ch >= 'A') && (ch <= 'Z'))
            || ((ch >= 'a') && (ch <= 'z'))
            || ((ch >= '0') && (ch <= '9'))
            || (ch == alphabet[62])     // The base64 and base64url alphabets are
            || (ch == alphabet[63]))    // the same except for the last two characters.
            return true;

        return false;
    }

    /// \brief  Determine if a string contains only valid Base64-encoded characters
    /// \param str      The string to be checked.
    /// \param alphabet Pointer to the alphabet to be compared against.
    /// \return \c true if the string contains only valid characters, \c false otherwise.
    static std::optional<brace::BasicParseError>
    validate_str(const std::string_view str, const char *alphabet)
    {
        if (str.size() % 4)
            return BasicParseError{0, str.size(), "Invalid length"};

        size_t  pos{0};
        if (!std::all_of(std::begin(str), std::end(str) - 2,
                            [&pos, alphabet](char ch) { ++pos; return is_valid_character(ch, alphabet); }))
            return BasicParseError{0, pos, "Invalid character"};

        auto const last = std::rbegin(str);
        if (!is_valid_character(*next(last), alphabet))
        {
            if (!((*next(last) == '=') && (*last == '=')))
            {
                pos = *next(last) == '=' ? -1 : -2;
                return BasicParseError(0, str.size() + pos, "Invalid character");
            }
        }

        if (is_valid_character(*last, alphabet) || (*last == '='))
            return std::nullopt;

        return BasicParseError(0, str.size() - 1, "Invalid character");
    }

    /// \brief  Decode four Base64 encoded characters to three output bytes.
    /// \param a    First of the four characters.
    /// \param b    Second of the four characters.
    /// \param c    Third of the four characters.
    /// \param d    Rourth of the four characters.
    /// \param decode_table     Pointer to the decoding table.
    /// \return std::array of three uint8_t objects containing the decoded bytes.
    static std::array<uint8_t, 3> decode_quad(char a, char b, char c, char d, const uint8_t *decode_table)
    {
        const uint32_t  hold{
                            (static_cast<uint32_t>(decode_table[a]) << 18)
                          | (static_cast<uint32_t>(decode_table[b]) << 12)
                          | (static_cast<uint32_t>(decode_table[c]) <<  6)
                          | (decode_table[d])
                        };
        const uint8_t   byte1 = (hold >> 16) & 0xFF;
        const uint8_t   byte2 = (hold >>  8) & 0xFF;
        const uint8_t   byte3 = hold & 0xFF;

        return {byte1, byte2, byte3};
    }

    /// \brief  Decode a "pure" string of Base64 encoded data to its original array of bytes.
    /// \param str          Base64 encoded string to decode.
    /// \param decode_table Pointer to the decoding table.
    /// \return std::vector of uint8_t objects containing the decoded bytes.
    /// \details A string is "pure" if its length is a multiple of four and
    /// contains only valid Base64 encoding characters. This function assumes
    /// the input string is "pure", and is generally somewhat faster than the
    /// \c decode_impure function, which assumes that the input string is not
    /// "pure" and performs additional processing on each character.
    static std::vector<uint8_t> decode_pure(const std::string_view str, const uint8_t *decode_table)
    {
        const auto size{str.size()};
        const auto full_quads{size / 4 - 1};
        std::vector<uint8_t>    rv;

        rv.reserve(((full_quads + 2) * 3) / 4);

        for (size_t i=0; i < full_quads; ++i)
        {
            const auto  quad{str.substr(i * 4, 4)};
            const auto  bytes{decode_quad(quad[0], quad[1], quad[2], quad[3], decode_table)};

            std::copy(std::begin(bytes), std::end(bytes), std::back_inserter(rv));
        }

        if (const auto last_quad = str.substr(full_quads * 4, 4); last_quad[2] == '=')
        {
            const auto  bytes{decode_quad(last_quad[0], last_quad[1], 'A', 'A', decode_table)};
            rv.push_back(bytes[0]);
        }
        else if (last_quad[3] == '=')
        {
            const auto  bytes{decode_quad(last_quad[0], last_quad[1], last_quad[2], 'A', decode_table)};
            std::copy_n(std::begin(bytes), 2, std::back_inserter(rv));
        }
        else
        {
            const auto  bytes{decode_quad(last_quad[0], last_quad[1], last_quad[2], last_quad[3], decode_table)};
            std::copy_n(std::begin(bytes), 3, back_inserter(rv));
        }

        return rv;
    }

    static size_t decode_pure(const std::string_view str, brace::BinOStream &outstream, const uint8_t *decode_table)
    {
        const auto  size{str.size()};
        const auto  full_quads{size / 4 - 1};
        size_t      bytes_written{0};

        for (size_t i=0; i < full_quads; ++i)
        {
            const auto  quad{str.substr(i * 4, 4)};
            const auto  bytes{decode_quad(quad[0], quad[1], quad[2], quad[3], decode_table)};
            const auto  bw{outstream.write(bytes.data(), bytes.size())};

            bytes_written += bw;
            if (bw < bytes.size())
                return bytes_written;
        }

        if (const auto last_quad = str.substr(full_quads * 4, 4); last_quad[2] == '=')
        {
            const auto  bytes{decode_quad(last_quad[0], last_quad[1], 'A', 'A', decode_table)};
            const auto  bw{outstream.write(bytes.data(), 1)};

            bytes_written += bw;
            if (bw < 1)
                return bytes_written;
        }
        else if (last_quad[3] == '=')
        {
            const auto  bytes{decode_quad(last_quad[0], last_quad[1], last_quad[2], 'A', decode_table)};
            const auto  bw{outstream.write(bytes.data(), 2)};

            bytes_written += bw;
            if (bw < 2)
                return bytes_written;
        }
        else
        {
            const auto  bytes{decode_quad(last_quad[0], last_quad[1], last_quad[2], last_quad[3], decode_table)};
            const auto  bw{outstream.write(bytes.data(), 3)};

            bytes_written += bw;
            if (bw < 3)
                return bytes_written;
        }

        return bytes_written;
    }

    /// \brief  Decode an "impure" string of Base64 encoded data to its original array of bytes.
    /// \param str          Base64 encoded string to decode.
    /// \param alphabet     Pointer to the alphabet used to encode the data.
    /// \param decode_table Pointer to the decoding table.
    /// \return On success returns a std::vector of uint8_t objects containing
    /// the decoded bytes. On failure returns std::nullopt. The return type is
    /// std::optional.
    /// \details A string is "pure" if its length is a multiple of four and
    /// contains only valid Base64 encoding characters. This function assumes
    /// the input string is not "pure" and may be of arbitrary length and contain
    /// invalid characters such as newlines anywhere within the encoded data.
    /// This function is generally somewhat slower than the \c decode_pure function
    /// since it must examine each character in turn to determine if it is a valid
    /// Base64 encoded character.
    static std::variant<std::vector<uint8_t>, brace::BasicParseError>
    decode_impure(const std::string_view str, const char *alphabet, const uint8_t *decode_table)
    {
        std::vector<uint8_t>    rv;
        std::array<char, 4>     quads;
        size_t                  quad_pos{0};
        char                    pad1{0}, pad2{0};
        auto                    p{str.begin()};
        size_t                  line{1};
        size_t                  pos{1};

        while (p != str.end())
        {
            if (is_valid_character(*p, alphabet))
            {
                if (pad1 || pad2)
                {
                    return brace::BasicParseError{line, pos - (pad2 ? 2 : 1), "Invalid character"};
                }

                pad1 = pad2 = 0;
                quads[quad_pos++] = *p;

                if (quad_pos == 4)
                {
                    auto    bytes{decode_quad(quads[0], quads[1], quads[2], quads[3], decode_table)};
                    rv.push_back(bytes[0]);
                    rv.push_back(bytes[1]);
                    rv.push_back(bytes[2]);

                    quad_pos = 0;
                }
            }
            else
            {
                if (*p == '\n')
                {
                    ++line;
                    pos = 0;
                }
                else if (*p == '=')
                {
                    if (pad1 == '=')
                    {
                        if (pad2 == 0)
                        {
                            pad2 = '=';
                        }
                        else
                        {
                            return brace::BasicParseError{line, pos, "Invalid character"};
                        }
                    }
                    else
                    {
                        pad1 = '=';
                    }
                }
                else
                {
                    return brace::BasicParseError{line, pos, "Invalid character"};
                }
            }

            ++pos;
            ++p;
        }

        // Check quad_pos and padding for possible last set of quads
        if (quad_pos)   // partial set remaining
        {
            if (quad_pos == 3 && pad1 == '=' && pad2 == 0)
            {
                auto    bytes{decode_quad(quads[0], quads[1], quads[2], 'A', decode_table)};
                rv.push_back(bytes[0]);
                rv.push_back(bytes[1]);
            }
            else if (quad_pos == 2 && pad1 == '=' && pad2 == '=')
            {
                auto    bytes{decode_quad(quads[0], quads[1], 'A', 'A', decode_table)};
                rv.push_back(bytes[0]);
            }
            else
            {
                return brace::BasicParseError{line, pos, "Invalid length or padding"};
            }
        }

        return rv;
    }
    static std::variant<size_t, brace::BasicParseError>
    decode_impure(const std::string_view str, brace::BinOStream &outstream, const char *alphabet, const uint8_t *decode_table)
    {
        std::array<char, 4>     quads;
        size_t                  quad_pos{0};
        char                    pad1{0}, pad2{0};
        auto                    p{str.begin()};
        size_t                  line{1};
        size_t                  pos{1};
        size_t                  bytes_written{0};

        while (p != str.end())
        {
            if (is_valid_character(*p, alphabet))
            {
                if (pad1 || pad2)
                    return brace::BasicParseError{line, pos - (pad2 ? 2 : 1), "Invalid character"};

                pad1 = pad2 = 0;
                quads[quad_pos++] = *p;

                if (quad_pos == 4)
                {
                    auto    bytes{decode_quad(quads[0], quads[1], quads[2], quads[3], decode_table)};
                    auto    bw{outstream.write(bytes.data(), bytes.size())};

                    bytes_written += bw;
                    if (bw != bytes.size())
                        return bytes_written;

                    quad_pos = 0;
                }
            }
            else
            {
                if (*p == '\n')
                {
                    ++line;
                    pos = 0;
                }
                else if (*p == '=')
                {
                    if (pad1 == '=')
                    {
                        if (pad2 == 0)
                            pad2 = '=';
                        else
                            return brace::BasicParseError{line, pos, "Invalid character"};
                    }
                    else
                    {
                        pad1 = '=';
                    }
                }
                else
                {
                    return brace::BasicParseError{line, pos, "Invalid character"};
                }
            }

            ++pos;
            ++p;
        }

        // Check quad_pos and padding for possible last set of quads
        if (quad_pos)   // partial quad remaining
        {
            if (quad_pos == 3 && pad1 == '=' && pad2 == 0)
            {
                auto    bytes{decode_quad(quads[0], quads[1], quads[2], 'A', decode_table)};
                auto    bw{outstream.write(bytes.data(), 2)};

                bytes_written += bw;
                if (bw != 2)
                    return bytes_written;
            }
            else if (quad_pos == 2 && pad1 == '=' && pad2 == '=')
            {
                auto    bytes{decode_quad(quads[0], quads[1], 'A', 'A', decode_table)};
                auto    bw{outstream.write(bytes.data(), 1)};

                bytes_written += bw;
                if (bw != 1)
                    return bytes_written;
            }
            else
            {
                return brace::BasicParseError{line, pos, "Invalid length or padding"};
            }
        }

        return bytes_written;
    }

protected:
    //
    //  Encoding
    //

    /// \brief  Perform the Base64 encoding of data from a range of bytes.
    /// \param input        Iterator at the beginning of the input data.
    /// \param input_end    Iterator at one past the end of the input data
    /// \param alphabet     Pointer to the alphabet to be used during encoding.
    /// \return A string containing the encoded data.
    template<typename input_iterator>
    std::string do_encode(input_iterator input, input_iterator input_end,
                          const char *alphabet) const
    {
        const auto  in_size{input_end - input};
        const auto  out_size{((in_size + 2) / 3) * 4};
        std::string rv;
        rv.reserve(out_size);

        size_t  i{0};
        if (in_size > 1)
        {
            for (; i < in_size - 2; i += 3, input += 3)
            {
                rv.push_back(alphabet[(*input >> 2) & 0x3F]);
                rv.push_back(alphabet[((*input & 0x03) << 4) | ((*(input + 1) & 0xF0) >> 4)]);
                rv.push_back(alphabet[((*(input + 1) & 0x0F) << 2) | ((*(input + 2) & 0xC0) >> 6)]);
                rv.push_back(alphabet[(*(input + 2) & 0x3F)]);
            }
        }

        if (i < in_size)
        {
            rv.push_back(alphabet[(*input >> 2) & 0x3F]);
            if (i == (in_size - 1))
            {
                rv.push_back(alphabet[((*input & 0x03) << 4)]);
                rv.push_back('=');
            }
            else
            {
                rv.push_back(alphabet[((*input & 0x03) << 4) | ((*(input + 1) & 0xF0) >> 4)]);
                rv.push_back(alphabet[((*(input + 1) & 0x0F) << 2)]);
            }
            rv.push_back('=');
        }

        return rv;
    }
    /// \brief  Perform the Base64 encoding of data from a range of bytes to a stream.
    /// \param input        Pointer to the beginning of the input data.
    /// \param input_end    Pointer to one past the end of the input data
    /// \param out_stream   std::ostream to receive the encoded characters.
    /// \param alphabet     Pointer to the alphabet to be used during encoding.
    template<typename input_iterator>
    size_t do_encode(input_iterator input, input_iterator input_end,
                     std::ostream &outstream, const char *alphabet) const
    {
        const auto  in_size{input_end - input};
        size_t  chars_written{0};
        size_t  i{0};

        if (in_size > 1)
        {
            for (; i < in_size - 2; i += 3, input += 3)
            {
                if (!outstream.put(alphabet[(*input >> 2) & 0x3F]).good())
                    return chars_written;
                if (!outstream.put(alphabet[((*input & 0x3) << 4) | (((*input + 1) & 0xF0) >> 4)]).good())
                    return chars_written;
                if (!outstream.put(alphabet[((*(input + 1) & 0xF) << 2) | ((*(input + 2) & 0xC0) >> 6)]).good())
                    return chars_written;
                if (!outstream.put(alphabet[*(input + 2) & 0x3F]).good())
                    return chars_written;
            }
        }

        if (i < in_size)
        {
            if (!outstream.put(alphabet[(*input >> 2) & 0x3F]).good())
                return chars_written;
            if (i == (in_size - 1))
            {
                if (!outstream.put(alphabet[((*input & 0x03) << 4)]).good())
                    return chars_written;
                if (!outstream.put('=').good())
                    return chars_written;
            }
            else
            {
                if (!outstream.put(alphabet[((*input & 0x03) << 4) | ((*(input + 1) & 0xF0) >> 4)]).good())
                    return chars_written;
                if (!outstream.put(alphabet[((*(input + 1) & 0x0F) << 2)]).good())
                    return chars_written;
            }
            if (!outstream.put('=').good())
                return chars_written;
        }

        return chars_written;
    }

    /// \brief  Perform the Base64 encoding of data from a binary stream.
    /// \param instream     brace::BinIStream containing the data to be encoded.
    /// \param alphabet     Pointer to the alphabet to be used during encoding.
    /// \return A string containing the encoded data.
    std::string do_encode(BinIStream &instream, const char *alphabet) const
    {
        std::string rv;

        while (instream.good())
        {
            uint8_t bytes[3];
            instream.read(bytes, 3);
            switch (instream.gcount())
            {
                case 3:
                {
                    rv.push_back(alphabet[(bytes[0] >> 2) & 0x3F]);
                    rv.push_back(alphabet[((bytes[0] & 0x3) << 4) | ((bytes[1] & 0xF0) >> 4)]);
                    rv.push_back(alphabet[((bytes[1] & 0xF) << 2) | ((bytes[2] & 0xC0) >> 6)]);
                    rv.push_back(alphabet[bytes[2] & 0x3F]);
                    break;
                }
                case 2:
                {
                    rv.push_back(alphabet[(bytes[0] >> 2) & 0x3F]);
                    rv.push_back(alphabet[((bytes[0] & 0x3) << 4) | ((bytes[1] & 0xF0) >> 4)]);
                    rv.push_back(alphabet[((bytes[1] & 0xF) << 2)]);
                    rv.push_back('=');
                    break;
                }
                case 1:
                {
                    rv.push_back(alphabet[(bytes[0] >> 2) & 0x3F]);
                    rv.push_back(alphabet[((bytes[0] & 0x3) << 4)]);
                    rv.push_back('=');
                    rv.push_back('=');
                    break;
                }
            }
        }

        return rv;
    }

    /// \brief  Perform the Base64 encoding of data from a binary stream to a standard stream.
    /// \param instream     brace::BinIStream containing the data to be encoded.
    /// \param outstream    std::ostream to receive the encoded characters.
    /// \param alphabet     Pointer to the alphabet to be used during encoding.
    size_t do_encode(BinIStream &instream, std::ostream &outstream, const char *alphabet) const
    {
        size_t      chars_written{0};

        while (instream.good() && outstream.good())
        {
            uint8_t bytes[3];
            instream.read(bytes, 3);
            auto    incount{instream.gcount()};

            if (incount == 3)
            {
                if (!outstream.put(alphabet[(bytes[0] >> 2) & 0x3F]).good())
                    break;
                ++chars_written;
                if (!outstream.put(alphabet[((bytes[0] & 0x3) << 4) | ((bytes[1] & 0xF0) >> 4)]).good())
                    break;
                ++chars_written;
                if (!outstream.put(alphabet[((bytes[1] & 0xF) << 2) | ((bytes[2] & 0xC0) >> 6)]).good())
                    break;
                ++chars_written;
                if (!outstream.put(alphabet[bytes[2] & 0x3F]).good())
                    break;
                ++chars_written;
            }
            else if (incount == 2)
            {
                if (!outstream.put(alphabet[(bytes[0] >> 2) & 0x3F]).good())
                    break;
                ++chars_written;
                if (!outstream.put(alphabet[((bytes[0] & 0x3) << 4) | ((bytes[1] & 0xF0) >> 4)]).good())
                    break;
                ++chars_written;
                if (!outstream.put(alphabet[((bytes[1] & 0xF) << 2)]).good())
                    break;
                ++chars_written;
                if (!outstream.put('=').good())
                    break;
                ++chars_written;
            }
            else if (incount == 1)
            {
                if (!outstream.put(alphabet[(bytes[0] >> 2) & 0x3F]).good())
                    break;
                ++chars_written;
                if (!outstream.put(alphabet[((bytes[0] & 0x3) << 4)]).good())
                    break;
                ++chars_written;
                if (!outstream.put('=').good())
                    break;
                ++chars_written;
                if (!outstream.put('=').good())
                    break;
                ++chars_written;
            }
            else    // must be zero
            {
                break;
            }
        }

        return chars_written;
    }

    //
    //  Decoding
    //

    /// \brief  Decode a Base64 encoded string to its original array of bytes.
    /// \param str              Base64 encoded string data to decode.
    /// \param alphabet         Pointer to the alphabet used to encode the data.
    /// \param decode_table     Pointer to the decoding table.
    /// \param ignore_newline   If \c true the decoder ignores newline characters
    ///                         encountered in the encoded data.
    /// \return On success returns a std::vector of uint8_t objects containing
    /// the decoded bytes.
    /// \exception brace::BasicParseError The data in str is invalid.
    std::variant<std::vector<uint8_t>, brace::BasicParseError>
    do_decode(const std::string_view str,
              const char *alphabet,
              const uint8_t *decode_table,
              bool ignore_newline) const
    {
        const auto size{str.size()};

        if (size == 0)
            return std::vector<uint8_t>{};

        if (!ignore_newline)
        {
            auto result{validate_str(str, alphabet)};

            if (result.has_value())
                return result.value();

            return decode_pure(str, decode_table);
        }
        else
        {
            return decode_impure(str, alphabet, decode_table);
        }
    }

    std::variant<size_t, brace::BasicParseError>
    do_decode(const std::string_view str,
              brace::BinOStream &outstream,
              const char *alphabet,
              const uint8_t *decode_table,
              bool ignore_newline) const
    {
        const auto size{str.size()};

        if (size == 0)
            return size;

        if (!ignore_newline)
        {
            auto result{validate_str(str, alphabet)};

            if (result.has_value())
                return result.value();

            return decode_pure(str, outstream, decode_table);
        }
        else
        {
            return decode_impure(str, outstream, alphabet, decode_table);
        }
    }

    std::variant<size_t, brace::BasicParseError>
    do_decode(std::istream &instream,
              brace::BinOStream &outstream,
              const char *alphabet,
              const uint8_t *decode_table,
              bool ignore_newline) const
    {
        std::array<char, 4>     quads;
        size_t                  quad_pos{0};
        char                    pad1{0}, pad2{0};
        size_t                  line{1};
        size_t                  pos{1};
        size_t                  bytes_written{0};

        while (instream.good() && outstream.good())
        {
            char    ch;

            if (instream.get(ch).good())
            {
                if (is_valid_character(ch, alphabet))
                {
                    if (pad1 || pad2)
                        return brace::BasicParseError{line, pos - (pad2 ? 2 : 1), "Invalid character"};

                    pad1 = pad2 = 0;
                    quads[quad_pos++] = ch;

                    if (quad_pos == 4)
                    {
                        auto    bytes{decode_quad(quads[0], quads[1], quads[2], quads[3], decode_table)};
                        auto    bw{outstream.write(bytes.data(), bytes.size())};

                        bytes_written += bw;
                        if (bw != bytes.size())
                            return bytes_written;

                        quad_pos = 0;
                    }
                }
                else
                {
                    if (ch == '\n')
                    {
                        if (ignore_newline)
                        {
                            ++line;
                            pos = 0;
                        }
                        else
                        {
                            return brace::BasicParseError{line, pos, "Invalid character"};
                        }
                    }
                    else if (ch == '=')
                    {
                        if (pad1 == '=')
                        {
                            if (pad2 == 0)
                                pad2 = '=';
                            else
                                return brace::BasicParseError{line, pos, "Invalid character"};
                        }
                        else
                        {
                            pad1 = '=';
                        }
                    }
                    else
                    {
                        return brace::BasicParseError{line, pos, "Invalid character"};
                    }
                }

                ++pos;
            }
        }

        if (outstream.good())
        {
            if (quad_pos)   // partial quad remaining
            {
                if (quad_pos == 3 && pad1 == '=' && pad2 == 0)
                {
                    auto    bytes{decode_quad(quads[0], quads[1], quads[2], 'A', decode_table)};
                    auto    bw{outstream.write(bytes.data(), 2)};

                    bytes_written += bw;
                    if (bw != 2)
                        return bytes_written;
                }
                else if (quad_pos == 2 && pad1 == '=' && pad2 == '=')
                {
                    auto    bytes{decode_quad(quads[0], quads[1], 'A', 'A', decode_table)};
                    auto    bw{outstream.write(bytes.data(), 1)};

                    bytes_written += bw;
                    if (bw != 1)
                        return bytes_written;
                }
                else
                {
                    return brace::BasicParseError{line, pos, "Invalid length or padding"};
                }
            }
        }

        return bytes_written;
    }

    std::variant<std::vector<uint8_t>, brace::BasicParseError>
    do_decode(std::istream &instream, const char *alphabet, const uint8_t *decode_table, bool ignore_newline) const
    {
        std::array<char, 4>     quads;
        size_t                  quad_pos{0};
        char                    pad1{0}, pad2{0};
        size_t                  line{1};
        size_t                  pos{1};
        std::vector<uint8_t>    rv;

        while (instream.good())
        {
            char    ch;

            if (instream.get(ch).good())
            {
                if (is_valid_character(ch, alphabet))
                {
                    if (pad1 || pad2)
                        return brace::BasicParseError{line, pos - (pad2 ? 2 : 1), "Invalid character"};

                    pad1 = pad2 = 0;
                    quads[quad_pos++] = ch;

                    if (quad_pos == 4)
                    {
                        auto    bytes{decode_quad(quads[0], quads[1], quads[2], quads[3], decode_table)};

                        rv.push_back(bytes[0]);
                        rv.push_back(bytes[1]);
                        rv.push_back(bytes[2]);

                        quad_pos = 0;
                    }
                }
                else
                {
                    if (ch == '\n')
                    {
                        if (ignore_newline)
                        {
                            ++line;
                            pos = 0;
                        }
                        else
                        {
                            return brace::BasicParseError{line, pos, "Invalid character"};
                        }
                    }
                    else if (ch == '=')
                    {
                        if (pad1 == '=')
                        {
                            if (pad2 == 0)
                                pad2 = '=';
                            else
                                return brace::BasicParseError{line, pos, "Invalid character"};
                        }
                        else
                        {
                            pad1 = '=';
                        }
                    }
                    else
                    {
                        return brace::BasicParseError{line, pos, "Invalid character"};
                    }
                }

                ++pos;
            }
        }

        if (quad_pos)   // partial quad remaining
        {
            if (quad_pos == 3 && pad1 == '=' && pad2 == 0)
            {
                auto    bytes{decode_quad(quads[0], quads[1], quads[2], 'A', decode_table)};

                rv.push_back(bytes[0]);
                rv.push_back(bytes[1]);
            }
            else if (quad_pos == 2 && pad1 == '=' && pad2 == '=')
            {
                auto    bytes{decode_quad(quads[0], quads[1], 'A', 'A', decode_table)};

                rv.push_back(bytes[0]);
            }
            else
            {
                return brace::BasicParseError(line, pos, "Invalid length or padding");
            }
        }

        return rv;
    }

    virtual const char *alphabet() const = 0;
    virtual const uint8_t *decode_table() const = 0;

public:
    /// \brief  Encode data from a range of bytes to a Base64 encoded string
    /// \param beg  Iterator at the beginning of the data to be encoded.
    /// \param end  Iterator at one past the end of the data to be encoded.
    /// \return A string containing the Base64 encoded data.
    template<typename input_iterator>
    auto encode(input_iterator beg, input_iterator end) const
            -> std::enable_if_t<sizeof(*beg) == 1, std::string>
    {
        return do_encode(beg, end, alphabet());
    }

    /// \brief  Encode data from a binary stream to a Base64 encoded string.
    /// \param instream     brace::BinIStream containing the data to be encoded.
    /// \return A string containing the encoded data.
    std::string encode(brace::BinIStream &instream) const
    {
        return do_encode(instream, alphabet());
    }

    /// \brief  Encode data from a range of bytes to a stream.
    /// \param beg  Iterator at the beginning of the input data.
    /// \param end  Iterator at one past the end of the input data
    /// \param outstream    std::ostream to receive the encoded data.
    /// \return The number of characters written to outstream.
    template<typename input_iterator>
    auto encode(input_iterator beg, input_iterator end, std::ostream &outstream) const
            -> std::enable_if_t<sizeof(*beg) == 1, size_t>
    {
        return do_encode(beg, end, outstream, alphabet());
    }

    /// \brief  Encode data from a binary stream to a standard stream.
    /// \param instream     brace::BinIStream containing the data to be encoded.
    /// \param outstream    std::ostream to receive the encoded characters.
    size_t encode(brace::BinIStream &instream, std::ostream &outstream) const
    {
        return do_encode(instream, outstream, alphabet());
    }

    /// \brief  Decode a Base64 encoded string to its original array of bytes.
    /// \param str              Base64 encoded string data to decode.
    /// \param ignore_invalid   If \c true the decoder ignores newlines
    ///                         encountered in the encoded data.
    /// \return On success returns a std::vector of uint8_t objects containing
    /// the decoded bytes.
    /// \exception \c brace::BasicParseError on failure.
    std::vector<uint8_t> decode(const std::string_view str, bool ignore_newline = false) const
    {
        auto rv{do_decode(str, alphabet(), decode_table(), ignore_newline)};

        if (std::holds_alternative<brace::BasicParseError>(rv))
            throw std::get<brace::BasicParseError>(rv);

        return std::get<std::vector<uint8_t>>(rv);
    }

    /// \brief  Decode a Base64 encoded string to its original array of bytes.
    /// \param str              Base64 encoded string data to decode.
    /// \param outstream        brace::BinOStream to contain the decoded bytes.
    /// \param ignore_newline   if \c true the decoder ignores newlines
    ///                         encountered in the encoded data.
    /// \return The number of bytes written to the output stream
    /// \exception  \c brace::BasicParseError on failure.
    /// \details    If there are errors in the input string the function throws.
    ///             If there are errors when writing to the output stream the function
    ///             returns prematurely with the number of bytes successfully written.
    ///             The state of the output stream can be checked for errors.
    size_t decode(const std::string_view str, brace::BinOStream &outstream, bool ignore_newline = false) const
    {
        auto rv{do_decode(str, outstream, alphabet(), decode_table(), ignore_newline)};

        if (std::holds_alternative<brace::BasicParseError>(rv))
            throw std::get<brace::BasicParseError>(rv);

        return std::get<size_t>(rv);
    }

    /// \brief  Decode Base64 encoded data from a standard stream into a \c brace::BinOStream.
    /// \param instream         Standard \c istream containing Base64 encoded data.
    /// \param outstream        \c brace::BinOStream to receive the decoded bytes.
    /// \param ignore_newline   if \c true the decoder ignores newlines
    ///                         encountered in the encoded data.
    /// \return The number of bytes written to the output stream
    /// \exception  \c brace::BasicParseError on failure.
    /// \details    If there are errors in the input string the function throws.
    ///             If there are errors when writing to the output stream the function
    ///             returns prematurely with the number of bytes successfully written.
    ///             The state of the input and output streams can be checked for errors.
    size_t decode(std::istream &instream, brace::BinOStream &outstream, bool ignore_newline = false) const
    {
        auto rv{do_decode(instream, outstream, alphabet(), decode_table(), ignore_newline)};

        if (std::holds_alternative<brace::BasicParseError>(rv))
            throw std::get<brace::BasicParseError>(rv);

        return std::get<size_t>(rv);
    }

    /// \brief  Decode Base64 encoded data from a standard stream into a vector.
    /// \param instream         Standard \c istream containing Base64 encoded data.
    /// \param ignore_newline   if \c true the decoder ignores newlines
    ///                         encountered in the encoded data.
    /// \return A \c std::vector<uint8_t> containing the decoded data.
    /// \exception  \c brace::BasicParseError
    std::vector<uint8_t>
    decode(std::istream &instream, bool ignore_newline = false) const
    {
        auto rv{do_decode(instream, alphabet(), decode_table(), ignore_newline)};

        if (std::holds_alternative<brace::BasicParseError>(rv))
            throw std::get<brace::BasicParseError>(rv);

        return std::get<std::vector<uint8_t>>(rv);
    }
};

/// \brief  The Base64 class provides functions for encoding and decoding data
///         to and from Base64, as described in RFC 4648
///         (https://www.rfc-editor.org/rfc/rfc4648), section 4.
///
/// \details    The only difference between Base64 and Base64Url is the alphabet used
///             for encoding. Base64Url uses a "URL and Filename safe" alphabet.
class Base64 : public Base64Base
{
private:
    /// \brief  Return a pointer to the basic alphabet used for encoding to Base64.
    /// \return A pointer to the alphabet.
    const char *alphabet() const override
    {
        static constexpr char alphabet[]{
            'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
            'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
            'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
            'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'};

        return alphabet;
    }

    /// \brief  Return a pointer to the decoding table used for decoding from Base64.
    /// \return A pointer to the decoding table.
    const uint8_t *decode_table() const override
    {
        static constexpr uint8_t decode_table[]{
            0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64,
            0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64,
            0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x3E, 0x64, 0x64, 0x64, 0x3F,
            0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64,
            0x64, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E,
            0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x64, 0x64, 0x64, 0x64, 0x64,
            0x64, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28,
            0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31, 0x32, 0x33, 0x64, 0x64, 0x64, 0x64, 0x64};

        return decode_table;
    }
};

/// \brief  The Base64Url class provides functions for encoding and decoding data
///         to and from Base64, as described in RFC 4648
///         (https://www.rfc-editor.org/rfc/rfc4648), section 5.
///
/// \details    The only difference between Base64 and Base64Url is the alphabet used
///             for encoding. Base64Url uses a "URL and Filename safe" alphabet.
class Base64Url : Base64Base
{
private:

    /// \brief  Return a pointer to the "URL and Filename safe" alphabet used for encoding to Base64Url
    /// \return A pointer to the alphabet.
    const char *alphabet() const override
    {
        static constexpr char alphabet[]{
            'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
            'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
            'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
            'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '-', '_'};

        return alphabet;
    }

    /// \brief  Return a pointer to the decoding table used for decoding from Base64Url
    /// \return A pointer to the decoding table.
    const uint8_t *decode_table() const override
    {
        static constexpr uint8_t decode_table[]{
            0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64,
            0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64,
            0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x3E, 0x64, 0x64,
            0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64,
            0x64, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E,
            0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x64, 0x64, 0x64, 0x64, 0x3F,
            0x64, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28,
            0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31, 0x32, 0x33, 0x64, 0x64, 0x64, 0x64, 0x64};

        return decode_table;
    }
};

}
#endif  // BRACE_LIB_BASE64_INC
