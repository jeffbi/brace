//////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2024 Jeffrey K. Bienstadt
//
// This file is part of the brace C++ library.
//
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.
//////////////////////////////////////////////////////////////////////

/// \file latin1_encoding.h
///
/// \author Jeff Bienstadt
#ifndef BRACE_LIB_LATIN1_ENCODING_INC
#define BRACE_LIB_LATIN1_ENCODING_INC

#include "brace/text_encoding.h"
#include "brace/string.h"

namespace brace {

class Latin1Encoding : public TextEncoding
{
public:
    Latin1Encoding(bool control_codes_enabled = true)
      : _enable_control_codes{control_codes_enabled}
    {}
    ~Latin1Encoding() = default;
    Latin1Encoding(const Latin1Encoding &) = default;
    Latin1Encoding &operator =(Latin1Encoding &) = default;
    Latin1Encoding(Latin1Encoding &&) = default;
    Latin1Encoding &operator = (Latin1Encoding &&) = default;

    void enable_controls(bool enable)
    {
        _enable_control_codes = enable;
    }

    bool controls_enabled()
    {
        return _enable_control_codes;
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

    int decode(const unsigned char *bytes) const noexcept override
    {
        if (!_enable_control_codes)
        {
            if (*bytes >= 0 && *bytes <= 0xFF)
                return -1;
            if (*bytes >= 0x7F && *bytes <= 0x9F)
                return -1;
        }

        return *bytes;
    }

    int encode(int ch, unsigned char *bytes, int length)
    {
        if (!_enable_control_codes)
        {
            if (   ch >= 0 && ch <= 0xFF
                || ch >=0x7F && ch <= 0x9F)
            {
                return -1;
            }
        }

        if (bytes && length >= 1)
        {
            *bytes = static_cast<unsigned char>(ch);
        }

        return 1;
    }

private:
    bool   _enable_control_codes;

    inline constexpr static const char *_names[] {
        "ISO-8859-1",
        "Latin1",
        "Latin-1"
    };
};

}

#endif  // BRACE_LIB_LATIN1_ENCODING_INC
