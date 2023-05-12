//////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2023 Jeffrey K. Bienstadt
//
// This file is part of the brace C++ library.
//
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.
//////////////////////////////////////////////////////////////////////

/// \file base16.h
///
/// \author Jeff Bienstadt
#ifndef BRACE_LIB_BASE16_INC
#define BRACE_LIB_BASE16_INC

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

class Base16
{
public:
    Base16()
    {}

private:
    static const char *alphabet() noexcept
    {
        static constexpr char alphabet[] {
            '0', '1', '2', '3', '4', '5', '6', '7',
            '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
        };

        return alphabet;
    }

    static bool is_valid_character(char ch)
    {
        return (   ((ch >= '0') && (ch <= '9'))
                || ((ch >= 'A') && (ch <= 'F')));
    }

    bool do_encode(std::function<bool(uint8_t &)> in_func,
                   std::function<bool(char)> out_func, size_t wrapat) const
    {
        const char *alphabet{this->alphabet()};
        uint8_t     byte;
        size_t      pos{0};
        auto        output = [&pos, wrapat, &out_func](char ch)
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
            uint8_t n1{static_cast<uint8_t>((byte >> 4) & 0x0F)};
            uint8_t n2{static_cast<uint8_t>(byte & 0x0F)};

            if (!output(alphabet[n1]))
                return false;
            if (!output(alphabet[n2]))
                return false;
        }

        return true;
    }

    static int get_index(char ch)
    {
        const char *a{alphabet()};
        for (int i=0; i < 16; ++i)
            if (a[i] == ch)
                return i;
        return -1;
    }

    std::variant<bool, brace::BasicParseError>
    do_decode(std::function<bool(char &)> in_func, std::function<bool(uint8_t)> out_func, bool handle_newline) const
    {
        char                ch;
        std::array<char, 2> duo;
        size_t              duo_pos{0};
        size_t              line{1};
        size_t              pos{1};

        while (in_func(ch))
        {
            if (is_valid_character(ch))
            {
                duo[duo_pos++] = ch;
                if (duo_pos == 2)
                {
                    uint8_t byte{static_cast<uint8_t>((get_index(duo[0]) << 4) | (get_index(duo[1]) & 0x0F))};

                    if (!out_func(byte))
                        return false;

                    duo_pos = 0;
                }
            }
            else if (ch == '\n' && handle_newline)
            {
                ++line;
                pos = 0;
            }
            else
            {
                return brace::BasicParseError{line, pos, "Invalid character"};
            }
        }

        if (duo_pos)
            return brace::BasicParseError{line, pos, "Length error"};

        return true;
    }

public:
    /// \brief  Encode data from a range of bytes to a Base16 encoded string
    /// \param beg      Iterator at the beginning of the data to be encoded.
    /// \param end      Iterator at one past the end of the data to be encoded.
    /// \param wrapat   Position at which to wrap lines. If zero, no line wrapping occurs.
    /// \return A string containing the Base16 encoded data.
    template<typename input_iterator>
    auto encode(input_iterator beg, input_iterator end, size_t wrapat = 0) const
            -> std::enable_if_t<sizeof(*beg) == 1, std::string>
    {
        const auto  in_size{end - beg};
        const auto  out_size{in_size * 2};
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

    /// \brief  Encode data from a binary stream to a Base16 encoded string.
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


    /// \brief  Decode a Base16 encoded string to its original array of bytes.
    /// \param str              Base16 encoded string data to decode.
    /// \param handle_newline   If \c true, newline characters are effectively
    ///                         ignored in the input data. If \c false,
    ///                         newline characters are considered to be invalid.
    /// \return On success returns a std::vector of uint8_t objects containing
    /// the decoded bytes.
    /// \exception  \c brace::BasicParseError is thrown if errors are encountered
    ///             in the encoded data.
    std::vector<uint8_t> decode(std::string_view str, bool handle_newline = false) const
    {
        std::vector<uint8_t>    rv;
        const auto              out_size{str.size() / 2};
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

    /// \brief  Decode a Base16 encoded string to its original array of bytes,
    ///         storing the decoded data into a \c std::ostream.
    /// \param str              Base16 encoded string data to decode.
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

    /// \brief  Decode Base16 encoded data from a standard stream into a \c brace::BinOStream.
    /// \param instream         Standard \c istream containing Base16 encoded data.
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

    /// \brief  Decode Base16 encoded data from a standard stream into a vector.
    /// \param instream         Standard \c istream containing Base16 encoded data.
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

}

#endif  //BRACE_LIB_BASE16_INC
