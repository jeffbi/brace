//////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2023 Jeffrey K. Bienstadt
//
// This file is part of the brace C++ library.
//
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.
//////////////////////////////////////////////////////////////////////

/// \file base32.h
///
/// \author Jeff Bienstadt
#ifndef BRACE_LIB_BASE32_INC
#define BRACE_LIB_BASE32_INC

#include <array>
#include <cstdint>
#include <functional>
#include <istream>
#include <ostream>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

#include "binistream.h"
#include "binostream.h"
#include "parseerror.h"

namespace brace {

/// \brief  The class Base32Base is an abstract base class for the Base32 and Base32Hex encoding
///         and decoding classes.
class Base32Base
{
protected:
    /// \brief  Padding character.
    constexpr static char           pad_char{'='};
    /// \brief  Text of bad-character error message.
    constexpr static const char    *bad_char_msg = "Invalid character";

    /// \brief  Default construct a Base32Base object. This constructor
    ///         is protected.
    Base32Base()
    {}

    /// \brief  Return a pointer to the alphabet. Override this function in derived
    ///         classes to provide an alphabet for the specific encoding/decoding algorithms.
    /// \return A pointer to the alphabet to be used for encoding.
    virtual const char *alphabet() const = 0;

    /// \brief  Return a pointer to the decoding table. Override this function in
    ///         derived classes to provide a decoding table for the specific decoding algorithm.
    /// \return A pointer to the decoding table to be used for decoding encoded data.
    virtual const uint8_t *decode_table() const = 0;

    /// \brief  Determine if a character is a valid member of the encoding alphabet.
    ///         Override this function in derived classes to determine if the character
    ///         is valid for a specific alphabet.
    /// \param ch   The character to check
    /// \return \c true if ch is a valid character in the encoding alphabet, \c false otherwise.
    virtual bool is_valid_character(char ch) const = 0;

private:
    /// \brief  Decode eight Base32 encoded characters into five output butes.
    /// \param str  A \c std::string_view containing the eight encoded characters.
    /// \return An \c std::array of five uint8_t objects containing the
    ///         the decoded bytes.
    std::array<uint8_t, 5> decode_eights(std::string_view str) const
    {
        const uint8_t          *table{decode_table()};
        u_int64_t               hold{
                                    (static_cast<uint64_t>(table[str[0]]) << 35)
                                  | (static_cast<uint64_t>(table[str[1]]) << 30)
                                  | (static_cast<uint64_t>(table[str[2]]) << 25)
                                  | (static_cast<uint64_t>(table[str[3]]) << 20)
                                  | (static_cast<uint64_t>(table[str[4]]) << 15)
                                  | (static_cast<uint64_t>(table[str[5]]) << 10)
                                  | (static_cast<uint64_t>(table[str[6]]) <<  5)
                                  | (static_cast<uint64_t>(table[str[7]]))
                                };
        const uint8_t   byte4 = (hold >> 32) & 0xFF;
        const uint8_t   byte3 = (hold >> 24) & 0xFF;
        const uint8_t   byte2 = (hold >> 16) & 0xFF;
        const uint8_t   byte1 = (hold >>  8) & 0xFF;
        const uint8_t   byte0 = hold & 0xFF;

        return {byte4, byte3, byte2, byte1, byte0};
    }

protected:
    /// \brief  Encode binary data to a Base32 encoded string.
    /// \param in_func  User-provided function used to supply input data.
    /// \param out_func User-provided function used to write the output string.
    /// \param wrapat   Position at which to wrap lines. If zero, no line wrapping occurs.
    /// \return \c true on success, \c false otherwise. A return value of \c false
    ///         generally means that \c out_func failed.
    /// \details    This is the workhorse function for encoding data. The \c encode
    ///             functions in the public interface call this function, providing
    ///             custom functions (in the form of lamba expressions), passing them
    ///             as the \c in_func and \c out_func parameters.
    bool do_encode(std::function<bool(uint8_t &)> in_func,
                   std::function<bool(char)> out_func, size_t wrapat) const
    {
        const char             *alphabet{this->alphabet()};
        uint8_t                 byte;
        std::array<uint8_t, 5>  bytes;
        size_t                  bytes_pos{0};
        size_t                  pos{0};
        auto                    output = [&pos, wrapat, &out_func](char ch)
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
            if (bytes_pos == 5)
            {
                bytes_pos = 0;
                if (!output(alphabet[(bytes[0] >> 3) & 0x1F]))
                    return false;
                if (!output(alphabet[((bytes[0] & 0x07) << 2) | ((bytes[1] & 0xC0) >> 6)]))
                    return false;
                if (!output(alphabet[(bytes[1] & 0x3E) >> 1]))
                    return false;
                if (!output(alphabet[((bytes[1] & 0x01) << 4) | ((bytes[2] & 0xF0) >> 4)]))
                    return false;
                if (!output(alphabet[((bytes[2] & 0x0F) << 1) | ((bytes[3] & 0x80) >> 7)]))
                    return false;
                if (!output(alphabet[((bytes[3] & 0x7C) >> 2)]))
                    return false;
                if (!output(alphabet[((bytes[3] & 0x03) << 3) | ((bytes[4] & 0xE0) >> 5)]))
                    return false;
                if (!output(alphabet[bytes[4] & 0x1F]))
                    return false;
            }
        }

        switch (bytes_pos)
        {
            case 4:
                if (!output(alphabet[(bytes[0] >> 3) & 0x1F]))
                    return false;
                if (!output(alphabet[((bytes[0] & 0x07) << 2) | ((bytes[1] & 0xC0) >> 6)]))
                    return false;
                if (!output(alphabet[(bytes[1] & 0x3E) >> 1]))
                    return false;
                if (!output(alphabet[((bytes[1] & 0x01) << 4) | ((bytes[2] & 0xF0) >> 4)]))
                    return false;
                if (!output(alphabet[((bytes[2] & 0x0F) << 1) | ((bytes[3] & 0x80) >> 7)]))
                    return false;
                if (!output(alphabet[((bytes[3] & 0x7C) >> 2)]))
                    return false;
                if (!output(alphabet[((bytes[3] & 0x03) << 3)]))
                    return false;
                if (!output(pad_char))
                    return false;
                break;
            case 3:
                if (!output(alphabet[(bytes[0] >> 3) & 0x1F]))
                    return false;
                if (!output(alphabet[((bytes[0] & 0x07) << 2) | ((bytes[1] & 0xC0) >> 6)]))
                    return false;
                if (!output(alphabet[(bytes[1] & 0x3E) >> 1]))
                    return false;
                if (!output(alphabet[((bytes[1] & 0x01) << 4) | ((bytes[2] & 0xF0) >> 4)]))
                    return false;
                if (!output(alphabet[((bytes[2] & 0x0F) << 1)]))
                    return false;
                if (!output(pad_char))
                    return false;
                if (!output(pad_char))
                    return false;
                if (!output(pad_char))
                    return false;
                break;
            case 2:
                if (!output(alphabet[(bytes[0] >> 3) & 0x1F]))
                    return false;
                if (!output(alphabet[((bytes[0] & 0x07) << 2) | ((bytes[1] & 0xC0) >> 6)]))
                    return false;
                if (!output(alphabet[(bytes[1] & 0x3E) >> 1]))
                    return false;
                if (!output(alphabet[((bytes[1] & 0x01) << 4)]))
                    return false;
                if (!output(pad_char))
                    return false;
                if (!output(pad_char))
                    return false;
                if (!output(pad_char))
                    return false;
                if (!output(pad_char))
                    return false;
                break;
            case 1:
                if (!output(alphabet[(bytes[0] >> 3) & 0x1F]))
                    return false;
                if (!output(alphabet[((bytes[0] & 0x07) << 2)]))
                    return false;
                if (!output(pad_char))
                    return false;
                if (!output(pad_char))
                    return false;
                if (!output(pad_char))
                    return false;
                if (!output(pad_char))
                    return false;
                if (!output(pad_char))
                    return false;
                if (!output(pad_char))
                    return false;
                break;
        }

        return true;
    }

    /// \brief  Decode Base32 encoded data back to its original form.
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
        std::array<char, 8> eights;
        size_t              eights_pos{0};
        char                ch;
        size_t              line{1};
        size_t              pos{1};
        size_t              pad_count{0};

        while (in_func(ch))
        {
            if (is_valid_character(ch))
            {
                if (pad_count)  // already seen a padding character, so something's broken.
                    return brace::BasicParseError{line, pos, bad_char_msg};

                eights[eights_pos++] = ch;
                if (eights_pos == 8)
                {
                    auto    bytes{decode_eights({eights.data(), eights.size()})};

                    for (auto b : bytes)
                        if (!out_func(b))
                            return false;

                    eights_pos = 0;
                }
            }
            else
            {
                if (ch == '\n' && handle_newline)
                {
                    ++line;
                    pos = 0;
                }
                else if (ch == pad_char && (++pad_count <= 6))
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

        if (eights_pos) // partial set remaining
        {
            std::string s{eights.data(), eights.size()};
            for (int i=eights_pos; i < 8; ++i)
                s[i] = 'A';
            auto    bytes{decode_eights(s)};
            switch (eights_pos)
            {
                case 2:
                    return out_func(bytes[0]);
                case 3:
                case 4:
                    for (size_t i=0; i < 2; ++i)
                        if (!out_func(bytes[i]))
                            return false;
                    break;
                case 5:
                    for (size_t i=0; i < 3; ++i)
                        if (!out_func(bytes[i]))
                            return false;
                    break;
                case 6:
                case 7:
                    for (size_t i=0; i < 4; ++i)
                        if (!out_func(bytes[i]))
                            return false;
            }
        }

        return true;
    }

public:
    /// \brief  Encode data from a range of bytes to a Base32 encoded string
    /// \param beg      Iterator at the beginning of the data to be encoded.
    /// \param end      Iterator at one past the end of the data to be encoded.
    /// \param wrapat   Position at which to wrap lines. If zero, no line wrapping occurs.
    /// \return A string containing the Base32 encoded data.
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

    /// \brief  Encode data from a binary stream to a Base32 encoded string.
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
    /// \return The number of characters written to the output stream,
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


    /// \brief  Decode a Base32 encoded string to its original array of bytes.
    /// \param str              Base32 encoded string data to decode.
    /// \param handle_newline   If \c true, newline characters are effectively
    ///                         ignored in the input data. If \c false,
    ///                         newline characters are considered to be invalid.
    /// \return On success returns a std::vector of uint8_t objects containing
    /// the decoded bytes.
    /// \exception  brace::BasicParseError if errors are encountered in the encoded data.
    std::vector<uint8_t> decode(std::string_view str, bool handle_newline = false) const
    {
        std::vector<uint8_t>    rv;
        const auto              out_size{((str.size() + 4) / 5) * 8};
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

    /// \brief  Decode a Base32 encoded string to its original array of bytes,
    ///         storing the decoded data into a \c std::ostream.
    /// \param str              Base32 encoded string data to decode.
    /// \param outstream        brace::BinOStream to contain the decoded bytes.
    /// \param handle_newline   If \c true, newline characters are effectively
    ///                         ignored in the input data. If \c false,
    ///                         newline characters are considered to be invalid.
    /// \return The number of bytes written to the output stream
    /// \exception  brace::BasicParseError if errors are encountered in the encoded data.
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

    /// \brief  Decode Base32 encoded data from a standard stream into a \c brace::BinOStream.
    /// \param instream         Standard \c istream containing Base32 encoded data.
    /// \param outstream        \c brace::BinOStream to receive the decoded bytes.
    /// \param handle_newline   If \c true, newline characters are effectively
    ///                         ignored in the input data. If \c false,
    ///                         newline characters are considered to be invalid.
    /// \return The number of bytes written to the output stream
    /// \exception  brace::BasicParseError if errors are encountered in the encoded data.
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

    /// \brief  Decode Base32 encoded data from a standard stream into a vector.
    /// \param instream         Standard \c istream containing Base32 encoded data.
    /// \param handle_newline   If \c true, newline characters are effectively
    ///                         ignored in the input data. If \c false,
    ///                         newline characters are considered to be invalid.
    /// \return A std::vector<uint8_t> containing the decoded data.
    /// \exception  brace::BasicParseError if errors are encountered in the encoded data.
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

/// \brief  The Base32 class provides functions for encoding and decoding data
///         to and from Base32, as described in RFC 4648, section 6
///         (https://www.rfc-editor.org/rfc/rfc4648#section-6).
///
/// \details    The only difference between Base32 and Base32Hex is the alphabet used
///             for encoding.
class Base32 : public Base32Base
{
private:
    [[nodiscard]] const char *alphabet() const override
    {
        static constexpr char alphabet[] {
            'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
            'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '2', '3', '4', '5', '6', '7'
        };

        return alphabet;
    }

    [[nodiscard]] const uint8_t *decode_table() const override
    {
        static constexpr uint8_t decode_table[] {
            0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64,
            0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64,
            0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64,
            0x64, 0x64, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64,
            0x64, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E,
            0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x64, 0x64, 0x64, 0x64, 0x64,
            0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64,
            0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64
        };

        return decode_table;
    }

    [[nodiscard]] bool is_valid_character(char ch) const override
    {
        return (   ((ch >= 'A') && (ch <= 'Z'))
                || ((ch >= '2') && (ch <= '7')));
    }
};

/// \brief  The Base32Hex class provides functions for encoding and decoding data
///         to and from Base32Hex, as described in RFC 4648, section 7
///         (https://www.rfc-editor.org/rfc/rfc4648#section-7).
///
/// \details    The only difference between Base32 and Base32Hex is the alphabet used
///             for encoding.
class Base32Hex : public Base32Base
{
private:
    [[nodiscard]] const char *alphabet() const override
    {
        static constexpr char alphabet[] {
            '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
            'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V'
        };

        return alphabet;
    }

    [[nodiscard]] const uint8_t *decode_table() const override
    {
        static constexpr uint8_t decode_table[] {
            0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64,
            0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64,
            0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64,
            0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64,
            0x64, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18,
            0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64,
            0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64,
            0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64
        };

        return decode_table;
    }

    [[nodiscard]] bool is_valid_character(char ch) const override
    {
        return (   ((ch >= '0') && (ch <= '9'))
                || ((ch >= 'A') && (ch <= 'V')));
    }

};

}
#endif  // BRACE_LIB_BASE32_INC
