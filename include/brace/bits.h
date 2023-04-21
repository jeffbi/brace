#pragma once
#ifndef BRACE_LIB_BITS_INC
#define BRACE_LIB_BITS_INC

//////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2018 Jeffrey K. Bienstadt
//
// This file is part of the brace C++ library.
//
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.
//////////////////////////////////////////////////////////////////////

/// \file Bits.h
/// \brief  Declaration and implementation of basic bit-twiddling free functions.
/// \author Jeff Bienstadt


/// \brief  The namespace enclosing the brace library.
///
/// The entire brace library is contained within this namespace.
namespace brace {

///
/// \brief  Set a specific bit to one
///
/// \param  val Value containing bit to set
/// \param  bit Zero-based bit number to set
///
template<typename T>
T set_bit(T val, int bit) noexcept
{
    return val | 1U << bit;
}

///
/// \brief  Clear a specific bit by setting that bit to zero
///
/// \param  val Value containing bit to clear
/// \param  bit Zero-based bit number to clear
///
template<typename T>
T clear_bit(T val, int bit) noexcept
{
    return val & ~(1U << bit);
}

///
/// \brief  Flip the specified bit in a value
///
/// \param  val Value containing git to flip
/// \param  bit Zero-based bit number to flip
///
template<typename T>
T flip_bit(T val, int bit) noexcept
{
    return val ^ 1U << bit;
}

///
/// \brief  Test if specified bit is set.
///
/// \param  val Value containing bit to test
/// \param  bit Zero-based bit number to test
///
/// \return \c true if the bit is set, \c false otherwise
template<typename T>
T test_bit(T val, int bit) noexcept
{
    return val &= 1U << bit;
}


/// \brief  Rotate bits in a word n bits to the left.
///
/// \param word Value to be rotated
/// \param bits Number of bits to rotate
///
/// \return The result of the rotation.
template <typename T>
inline T rotate_left(T word, int bits) noexcept
{
    constexpr int w{sizeof(T) * 8};     // assumes an 8-bit byte

    return (word << bits) | (word >> (w - bits));
}

/// \brief  Rotate bits in a word n bits to the right.
///
/// \param word     Value to be rotated
/// \param bits     Number of bits to rotate
///
/// \return The result of the rotation.
template <typename T>
inline T rotate_right(T word, int bits) noexcept
{
    constexpr int w{sizeof(T) * 8};     // assumes an 8-bit byte

    return (word >> bits) | (word << (w - bits));
}

} // namespace brace

#endif  // BRACE_LIB_BITS_INC
