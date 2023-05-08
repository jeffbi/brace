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
#include <functional>
#include <istream>
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
    constexpr static char           pad_char{'='};
    constexpr static const char    *bad_char_msg{"Invalid character"};

    /// \brief  Default construct a Base64Base object. This constructor
    ///         is protected.
    Base64Base()
    {}

    virtual const char *alphabet() const = 0;
    virtual const uint8_t *decode_table() const = 0;

private:
    /// \brief  Determine if ch is a valid Base64-encoded character.
    /// \param ch       The character to be checked.
    /// \return \c true if the character is valid, \c false otherwise.
    bool is_valid_character(char ch) const
    {
        if (   ((ch >= 'A') && (ch <= 'Z'))
            || ((ch >= 'a') && (ch <= 'z'))
            || ((ch >= '0') && (ch <= '9'))
            || (ch == alphabet()[62])     // The base64 and base64url alphabets are
            || (ch == alphabet()[63]))    // the same except for the last two characters.
            return true;

        return false;
    }

    /// \brief  Decode four Base64 encoded characters to three output bytes.
    /// \param a    First of the four characters.
    /// \param b    Second of the four characters.
    /// \param c    Third of the four characters.
    /// \param d    Fourth of the four characters.
    /// \return std::array of three uint8_t objects containing the decoded bytes.
    std::array<uint8_t, 3> decode_quad(char a, char b, char c, char d) const
    {
        const auto      table{decode_table()};
        const uint32_t  hold{
                            (static_cast<uint32_t>(table[a]) << 18)
                          | (static_cast<uint32_t>(table[b]) << 12)
                          | (static_cast<uint32_t>(table[c]) <<  6)
                          | (table[d])
                        };
        const uint8_t   byte1 = (hold >> 16) & 0xFF;
        const uint8_t   byte2 = (hold >>  8) & 0xFF;
        const uint8_t   byte3 = hold & 0xFF;

        return {byte1, byte2, byte3};
    }
    std::array<uint8_t, 3> decode_quad(std::string_view str) const
    {
        return decode_quad(str[0], str[1], str[2], str[3]);
    }
    std::array<uint8_t, 3> decode_quad(std::array<char, 4> &arr) const
    {
        return decode_quad(arr[0], arr[1], arr[2], arr[3]);
    }

protected:
    /// \brief  Encode binary data to a Base64 encoded string.
    /// \param in_func  User-provided function used to supply input data.
    /// \param out_func User-provided function used to write the output string.
    /// \param wrapat   Position at which to wrap lines. If zero, no line wrapping occurs.
    /// \return \c true on success, \c false otherwise. A return value of \c false
    ///         generally means that \c out_func failed.
    /// \details    This is the workhorse function for encoding data. The \c encode
    ///             functions in the public interface call this function, providing
    ///             custom functions (in the form of lamba expressions), passing them
    ///             as the \c in_func and \c out_func parameters.
    bool do_encode(std::function<bool(uint8_t &)> in_func, std::function<bool(char)> out_func, size_t wrapat) const
    {
        const char             *alphabet{this->alphabet()};
        uint8_t                 byte;
        std::array<uint8_t, 3>  bytes;
        size_t                  bytes_pos{0};
        size_t                  pos{0};
        auto                    output = [&pos, &out_func, wrapat](char ch)
                                    {
                                        bool    ok;

                                        if (ok = out_func(ch))
                                        {
                                            if (wrapat && (++pos == wrapat))
                                            {
                                                ok = out_func('\n');
                                                pos = 0;
                                            }
                                        }
                                        return ok;
                                    };

        while (in_func(byte))
        {
            bytes[bytes_pos++] = byte;
            if (bytes_pos == 3)
            {
                bytes_pos = 0;
                if (!output(alphabet[(bytes[0] >> 2) & 0x3F]))
                    return false;
                if (!output(alphabet[((bytes[0] & 0x3) << 4) | ((bytes[1] & 0xF0) >> 4)]))
                    return false;
                if (!output(alphabet[((bytes[1] & 0xF) << 2) | ((bytes[2] & 0xC0) >> 6)]))
                    return false;
                if (!output(alphabet[bytes[2] & 0x3F]))
                    return false;
            }
        }

        switch (bytes_pos)
        {
            case 2:
                if (!output(alphabet[(bytes[0] >> 2) & 0x3F]))
                    return false;
                if (!output(alphabet[((bytes[0] & 0x3) << 4) | ((bytes[1] & 0xF0) >> 4)]))
                    return false;
                if (!output(alphabet[((bytes[1] & 0xF) << 2)]))
                    return false;
                if (!output(pad_char))
                    return false;
                break;
            case 1:
                if (!output(alphabet[(bytes[0] >> 2) & 0x3F]))
                    return false;
                if (!output(alphabet[((bytes[0] & 0x3) << 4)]))
                    return false;
                if (!output(pad_char))
                    return false;
                if (!output(pad_char))
                    return false;
                break;
        }

        return true;
    }

    /// \brief  Decode Base64 encoded data back to its original form.
    /// \param in_func  User-provided function used to supply input data.
    /// \param out_func User-provided function used to write output data.
    /// \param handle_newline   If \c true, newline characters are effectively
    ///                         ignored in the input data. If \c false,
    ///                         newline characters are considered to be invalid.
    /// \return An std::variant containing either a boolean value indicating
    ///         success or failure, or a brace::BasicParseError indicating that
    ///         an error was encountered in the input data. A return value of
    ///         boolean \c false generally indicates a failure in the
    ///         \c out_func output function.
    /// \details    This is the workhorse function for decoding data. The \c decode
    ///             functions in the public interface call this function, providing
    ///             custom functions (in the form of lambda expressions), passing them
    ///             as the \c in_func and \c out_func parameters.
    std::variant<bool, brace::BasicParseError>
    do_decode(std::function<bool(char &)> in_func, std::function<bool(uint8_t)> out_func, bool handle_newline) const
    {
        std::array<char, 4> quads;
        size_t              quads_pos{0};
        size_t              pad_count{0};
        size_t              line{1};
        size_t              pos{1};
        char                ch;

        while (in_func(ch))
        {
            if (is_valid_character(ch))
            {
                if (pad_count)
                    return brace::BasicParseError{line, pos, bad_char_msg};

                quads[quads_pos++] = ch;

                if (quads_pos == 4)
                {
                    auto    bytes{decode_quad(quads)};

                    for (auto b : bytes)
                        if (!out_func(b))
                            return false;
                    quads_pos = 0;
                }
            }
            else
            {
                if (ch == '\n' && handle_newline)
                {
                    ++line;
                    pos = 0;
                }
                else if (ch == pad_char && (++pad_count <= 2))
                {
                    // do nothing more
                }
                else
                {
                    return brace::BasicParseError{line, pos, bad_char_msg};
                }
            }

            ++pos;
        }

        if (quads_pos)
        {
            if (quads_pos == 3 && pad_count == 1)
            {
                auto    bytes{decode_quad(quads[0], quads[1], quads[2], 'A')};

                if (!out_func(bytes[0]))
                    return false;
                if (!out_func(bytes[1]))
                    return false;
            }
            else if (quads_pos == 2 && pad_count == 2)
            {
                auto    bytes{decode_quad(quads[0], quads[1], 'A', 'A')};

                if (!out_func(bytes[0]))
                    return false;
            }
            else
            {
                return brace::BasicParseError(line, pos, "Invalid length or padding");
            }
        }

        return true;
    }

public:
    /// \brief  Encode data from a range of bytes to a Base64 encoded string
    /// \param beg      Iterator at the beginning of the data to be encoded.
    /// \param end      Iterator at one past the end of the data to be encoded.
    /// \param wrapat   Position at which to wrap lines. If zero, no line wrapping occurs.
    /// \return A string containing the Base64 encoded data.
    template<typename input_iterator>
    auto encode(input_iterator beg, input_iterator end, size_t wrapat = 0) const
            -> std::enable_if_t<sizeof(*beg) == 1, std::string>
    {
        const auto  in_size{end - beg};
        const auto  out_size{((in_size + 2) / 3) * 4};
        std::string rv;

        auto    in_func = [&beg, &end, wrapat](uint8_t &b)
                {
                    if (beg == end)
                        return false;
                    b = *beg++;
                    return true;
                };
        auto    out_func = [&rv](char ch)
                {
                    rv.push_back(ch);
                    return true;
                };

        rv.reserve(out_size);

        do_encode(in_func, out_func, wrapat);
        return rv;
    }

    /// \brief  Encode data from a binary stream to a Base64 encoded string.
    /// \param instream brace::BinIStream containing the data to be encoded.
    /// \param wrapat   Position at which to wrap lines. If zero, no line wrapping occurs.
    /// \return A string containing the encoded data.
    std::string encode(brace::BinIStream &instream, size_t wrapat = 0) const
    {
        std::string rv;

        auto    in_func = [&instream](uint8_t &b)
                {
                    instream.get(b);
                    return instream.good();
                };
        auto    out_func = [&rv](char ch)
                {
                    rv.push_back(ch);
                    return true;
                };

        do_encode(in_func, out_func, wrapat);
        return rv;
    }

    /// \brief  Encode data from a range of bytes to a stream.
    /// \param beg  Iterator at the beginning of the input data.
    /// \param end  Iterator at one past the end of the input data
    /// \param outstream    std::ostream to receive the encoded data.
    /// \param wrapat   Position at which to wrap lines. If zero, no line wrapping occurs.
    /// \return The number of characters written to outstream.
    template<typename input_iterator>
    auto encode(input_iterator beg, input_iterator end, std::ostream &outstream, size_t wrapat = 0) const
            -> std::enable_if_t<sizeof(*beg) == 1, size_t>
    {
        auto    in_func = [&beg, &end, wrapat](uint8_t &b)
                {
                    if (beg == end)
                        return false;
                    b = *beg++;
                    return true;
                };
        size_t  chars_written{0};
        auto    out_func = [&outstream, &chars_written](char ch)
                {
                    if (outstream.put(ch).good())
                        ++chars_written;
                    return outstream.good();
                };

        do_encode(in_func, out_func, wrapat);

        return chars_written;
    }

    /// \brief  Encode data from a binary stream to a standard stream.
    /// \param instream     brace::BinIStream containing the data to be encoded.
    /// \param outstream    std::ostream to receive the encoded characters.
    /// \param wrapat   Position at which to wrap lines. If zero, no line wrapping occurs.
    size_t encode(brace::BinIStream &instream, std::ostream &outstream, size_t wrapat = 0) const
    {
        auto    in_func = [&instream](uint8_t &b)
                {
                    instream.get(b);
                    return instream.good();
                };
        size_t  chars_written{0};
        auto    out_func = [&outstream, &chars_written](char ch)
                {
                    if (outstream.put(ch).good())
                        ++chars_written;
                    return outstream.good();
                };

        do_encode(in_func, out_func, wrapat);

        return chars_written;
    }


    /// \brief  Decode a Base64 encoded string to its original array of bytes.
    /// \param str              Base64 encoded string data to decode.
    /// \param handle_newline   If \c true, newline characters are effectively
    ///                         ignored in the input data. If \c false,
    ///                         newline characters are considered to be invalid.
    /// \return On success returns a std::vector of uint8_t objects containing
    /// the decoded bytes.
    /// \exception  \c brace::BasicParseError is thrown if errors are encountered
    ///             in the encoded data.
    std::vector<uint8_t> decode(const std::string_view str, bool handle_newline = false) const
    {
        std::vector<uint8_t>    rv;
        const auto              out_size{((str.size() + 2) / 3) * 4};
        rv.reserve(out_size);

        auto    in_it{str.begin()};
        auto    in_func = [&in_it, &str](char &ch)
                {
                    if (in_it != str.end())
                    {
                        ch = *in_it++;
                        return true;
                    }
                    return false;
                };
        auto    out_func = [&rv](uint8_t b)
                {
                    rv.push_back(b);
                    return true;
                };

        auto    result{do_decode(in_func, out_func, handle_newline)};

        if (std::holds_alternative<brace::BasicParseError>(result))
            throw std::get<brace::BasicParseError>(result);

        return rv;
    }

    /// \brief  Decode a Base64 encoded string to its original array of bytes.
    /// \param str              Base64 encoded string data to decode.
    /// \param outstream        brace::BinOStream to contain the decoded bytes.
    /// \param handle_newline   If \c true, newline characters are effectively
    ///                         ignored in the input data. If \c false,
    ///                         newline characters are considered to be invalid.
    /// \return The number of bytes written to the output stream
    /// \exception  \c brace::BasicParseError is thrown if errors are encountered
    ///             in the encoded data.
    /// \details    If there are errors in the input string the function throws.
    ///             If there are errors when writing to the output stream the function
    ///             returns prematurely with the number of bytes successfully written.
    ///             The state of the output stream can be checked for errors.
    size_t decode(std::string_view str, brace::BinOStream &outstream, bool handle_newline = false) const
    {
        auto    in_it{str.begin()};
        auto    in_func = [&in_it, &str](char &ch)
                {
                    if (in_it != str.end())
                    {
                        ch = *in_it++;
                        return true;
                    }
                    return false;
                };
        size_t  bytes_written{0};
        auto    out_func = [&outstream, &bytes_written](uint8_t b)
                {
                    if (outstream.put(b) == b)
                        ++bytes_written;
                    return outstream.good();
                };

        auto    rv{do_decode(in_func, out_func, handle_newline)};

        if (std::holds_alternative<brace::BasicParseError>(rv))
            throw std::get<brace::BasicParseError>(rv);

        return bytes_written;
    }

    /// \brief  Decode Base64 encoded data from a standard stream into a \c brace::BinOStream.
    /// \param instream         Standard \c istream containing Base64 encoded data.
    /// \param outstream        \c brace::BinOStream to receive the decoded bytes.
    /// \param handle_newline   If \c true, newline characters are effectively
    ///                         ignored in the input data. If \c false,
    ///                         newline characters are considered to be invalid.
    /// \return The number of bytes written to the output stream
    /// \exception  \c brace::BasicParseError is thrown if errors are encountered
    ///             in the encoded data.
    /// \details    If there are errors in the input string the function throws.
    ///             If there are errors when writing to the output stream the function
    ///             returns prematurely with the number of bytes successfully written.
    ///             The state of the input and output streams can be checked for errors.
    size_t decode(std::istream &instream, brace::BinOStream &outstream, bool handle_newline = false) const
    {
        auto    in_func = [&instream](char &ch)
                {
                    return instream.get(ch).good();
                };
        size_t  bytes_written{0};
        auto    out_func = [&outstream, &bytes_written](uint8_t b)
                {
                    if (outstream.put(b) == b)
                        ++bytes_written;
                    return outstream.good();
                };

        auto    rv{do_decode(in_func, out_func, handle_newline)};

        if (std::holds_alternative<brace::BasicParseError>(rv))
            throw std::get<brace::BasicParseError>(rv);

        return bytes_written;
    }

    /// \brief  Decode Base64 encoded data from a standard stream into a vector.
    /// \param instream         Standard \c istream containing Base64 encoded data.
    /// \param handle_newline   If \c true, newline characters are effectively
    ///                         ignored in the input data. If \c false,
    ///                         newline characters are considered to be invalid.
    /// \return A \c std::vector<uint8_t> containing the decoded data.
    /// \exception  \c brace::BasicParseError is thrown if errors are encountered
    ///             in the encoded data.
    std::vector<uint8_t>
    decode(std::istream &instream, bool handle_newline = false) const
    {
        std::vector<uint8_t>    rv;
        auto    in_func = [&instream](char &ch)
                {
                    return instream.get(ch).good();
                };
        auto    out_func = [&rv](uint8_t b)
                {
                    rv.push_back(b);
                    return true;
                };
        auto result{do_decode(in_func, out_func, handle_newline)};

        if (std::holds_alternative<brace::BasicParseError>(result))
            throw std::get<brace::BasicParseError>(result);

        return rv;
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
            'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'
        };

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
            0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31, 0x32, 0x33, 0x64, 0x64, 0x64, 0x64, 0x64
        };

        return decode_table;
    }
};

/// \brief  The Base64Url class provides functions for encoding and decoding data
///         to and from Base64, as described in RFC 4648
///         (https://www.rfc-editor.org/rfc/rfc4648), section 5.
///
/// \details    The only difference between Base64 and Base64Url is the alphabet used
///             for encoding. Base64Url uses a "URL and Filename safe" alphabet.
class Base64Url : public Base64Base
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
