//////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2024 Jeffrey K. Bienstadt
//
// This file is part of the brace C++ library.
//
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.
//////////////////////////////////////////////////////////////////////

/// \file string.h
///
/// \author Jeff Bienstadt
#ifndef BRACE_LIB_STRING_INC
#define BRACE_LIB_STRING_INC

#include <string>
#include <string_view>

#include <brace/ascii.h>

namespace brace {

/// \brief  Return a copy of str containing all upper-case characters.
/// \tparam S   type of string
/// \param str  original string
/// \return a new string containing all upper-case characters.
template <typename S>
S to_upper(const S &str)
{
    S   rv{str};

    auto    it{rv.begin()};
    auto    end{rv.end()};

    while (it != end)
    {
        int     ch{static_cast<unsigned char>(*it)};

        *it = static_cast<typename S::value_type>(Ascii::to_upper(ch));
        ++it;
    }

    return rv;
}


/// \brief  Return a copy of str containing all lower-case characters.
/// \tparam S   type of string
/// \param str  original string
/// \return a new string containing all lower-case characters.
template <typename S>
S to_lower(const S &str)
{
    S   rv{str};

    auto    it{rv.begin()};
    auto    end{rv.end()};

    while (it != end)
    {
        int     ch{static_cast<unsigned char>(*it)};

        *it = static_cast<typename S::value_type>(Ascii::to_lower(ch));
        ++it;
    }

    return rv;
}

/// \brief  Compare two string_view objects without regard to case.
/// \tparam CharT   Character type for string_view
/// \tparam Traits  Traits for string_view
/// \param sv1 First string_view to compare.
/// \param sv2 Second string_view to compare.
/// \return An integer value with the results of the compare. If the
///         string_views compare equal (regardless of case) the function
///         returns 0. If the first string_view compares less than the second
///         string_view, the function returns a value < 0. If the first string_view
///         compares greater than the second string_view, the function returns
///         a value > 0.
template <typename CharT,
          typename Traits = std::char_traits<CharT>>
inline int ci_compare(std::basic_string_view<CharT, Traits> sv1,
               std::basic_string_view<CharT, Traits> sv2)
{
    using value_type = typename std::basic_string_view<CharT, Traits>::value_type;

    auto    it1{sv1.cbegin()};
    auto    end1{sv1.cend()};
    auto    it2{sv2.cbegin()};
    auto    end2{sv2.cend()};

    while (it1 != end1 && it2 != end2)
    {
        auto    c1{static_cast<value_type>(Ascii::to_lower(*it1))};
        auto    c2{static_cast<value_type>(Ascii::to_lower(*it2))};

        if (c1 < c2)
            return -1;
        else if (c1 > c2)
            return 1;

        ++it1;
        ++it2;
    }

    if (it1 == end1)
        return it2 == end2 ? 0 : -1;
    else
        return 1;
}

/// \brief  Compare two string_view objects without regard to case.
/// \param sv1 First string_view to compare.
/// \param sv2 Second string_view to compare.
/// \return An integer value with the results of the compare. If the
///         string_views compare equal (regardless of case) the function
///         returns 0. If the first string_view compares less than the second
///         string_view, the function returns a value < 0. If the first string_view
///         compares greater than the second string_view, the function returns
///         a value > 0.
inline int ci_compare(std::string_view sv1, std::string_view sv2)
{
    return ci_compare<char>(sv1, sv2);
}
/// \brief  Compare two wstring_view objects without regard to case.
/// \param sv1 First wstring_view to compare.
/// \param sv2 Second wstring_view to compare.
/// \return An integer value with the results of the compare. If the
///         string_views compare equal (regardless of case) the function
///         returns 0. If the first string_view compares less than the second
///         string_view, the function returns a value < 0. If the first string_view
///         compares greater than the second string_view, the function returns
///         a value > 0.
inline int ci_compare(std::wstring_view sv1, std::wstring_view sv2)
{
    return ci_compare<wchar_t>(sv1, sv2);
}
/// \brief  Compare two u16string_view objects without regard to case.
/// \param sv1 First u16string_view to compare.
/// \param sv2 Second u16string_view to compare.
/// \return An integer value with the results of the compare. If the
///         string_views compare equal (regardless of case) the function
///         returns 0. If the first string_view compares less than the second
///         string_view, the function returns a value < 0. If the first string_view
///         compares greater than the second string_view, the function returns
///         a value > 0.
inline int ci_compare(std::u16string_view sv1, std::u16string_view sv2)
{
    return ci_compare<char16_t>(sv1, sv2);
}
/// \brief  Compare two u32string_view objects without regard to case.
/// \param sv1 First u32string_view to compare.
/// \param sv2 Second u32string_view to compare.
/// \return An integer value with the results of the compare. If the
///         string_views compare equal (regardless of case) the function
///         returns 0. If the first string_view compares less than the second
///         string_view, the function returns a value < 0. If the first string_view
///         compares greater than the second string_view, the function returns
///         a value > 0.
inline int ci_compare(std::u32string_view sv1, std::u32string_view sv2)
{
    return ci_compare<char32_t>(sv1, sv2);
}

}

#endif  // BRACE_LIB_STRING_INC
