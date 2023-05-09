//////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2023 Jeffrey K. Bienstadt
//
// This file is part of the brace C++ library.
//
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.
//////////////////////////////////////////////////////////////////////

/// \file parseerror.h
///
/// \author Jeff Bienstadt
#ifndef BRACE_LIB_PARSEERROR_INC
#define BRACE_LIB_PARSEERROR_INC

#include <stdexcept>
#include <string>

namespace brace {

/// \brief  The BasicParseError class is a rudimentary parser-error
///         reporting class, providing a line number and position
///         where an error occurred along with a simple text message.
///         Derive from this class to provide additional parser error
///         information.
class BasicParseError : public std::runtime_error
{
public:
    /// \brief  Construct a BasicParseError object.
    /// \param line The line number where the error occurred.
    /// \param pos  The position (column) in the line where the error occurred.
    /// \param msg  The message text produced by the \c what() function.
    BasicParseError(size_t line, size_t pos, const char *msg)
        : std::runtime_error(msg)
        , _line{line}
        , _pos{pos}
    {}

    /// \brief  Construct a BasicParseError object.
    /// \param line The line number where the error occurred.
    /// \param pos  The position (column) in the line where the error occurred.
    /// \param msg  The message text produced by the \c what() function.
    BasicParseError(size_t line, size_t pos, const std::string &msg)
        : std::runtime_error(msg)
        , _line(line)
        , _pos{pos}
    {}

    BasicParseError(const BasicParseError &) noexcept = default;
    BasicParseError & operator=(const BasicParseError &) noexcept = default;

    /// \brief  Retrieve the error line number.
    /// \return The line number where the error occurred.
    size_t line() const noexcept
    {
        return _line;
    }

    /// \brief  Retrieve the error position on the line.
    /// \return The position on the line where the error occurred.
    size_t position() const noexcept
    {
        return _pos;
    }

private:
    size_t  _line;
    size_t  _pos;
};

}

#endif  // BRACE_LIB_PARSEERROR_INC
