//////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2023 Jeffrey K. Bienstadt
//
// This file is part of the brace C++ library.
//
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.
//////////////////////////////////////////////////////////////////////

/// \file byteorder.h
///
/// \author Jeff Bienstadt
#ifndef BRACE_LIB_BYTEORDER_INC
#define BRACE_LIB_BTYEORDER_INC

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <type_traits>

namespace brace
{

#if defined(__clang__) || defined(__GNUC__)
#define BRACE_BYTE_ORDER_LITTLE_ENDIAN    __ORDER_LITTLE_ENDIAN__
#define BRACE_BYTE_ORDER_BIG_ENDIAN       __ORDER_BIG_ENDIAN__
#define BRACE_BYTE_ORDER                  __BYTE_ORDER__
#elif defined(_WIN32)
#define BRACE_BYTE_ORDER_LITTLE_ENDIAN    1234
#define BRACE_BYTE_ORDER_BIG_ENDIAN       4321
#define BRACE_BYTE_ORDER                  BRACE_BYTE_ORDER_LITTLE_ENDIAN
#else
#error "Unrecognized compiler!"
#endif

/// @brief  Represents the various byte endian types.
enum class Endian
{
    /// \brief  Indicates little-endian
    Little  = BRACE_BYTE_ORDER_LITTLE_ENDIAN,
    /// \brief  Indicates big-endian
    Big     = BRACE_BYTE_ORDER_BIG_ENDIAN,
    /// \brief  Indicates the native endian type
    Native  = BRACE_BYTE_ORDER,
    /// \brief  Indicates network endian
    Network = Big
};


/// \cond
//
// The byte_swap_sized template functions do their swapping
// in-place. They are generally meant as helpers to the
// by-value byte_swap template function.
template<size_t S>
inline void byte_swap_sized(char *data)
{
    char    buf[S];

    std::memcpy(buf, data, S);
    std::reverse(std::begin(buf), std::end(buf));
    std::memcpy(data, buf, S);
}

// Specialization for one-byte (8-bit) values
template<>
inline void byte_swap_sized<1>(char *)
{
    // do nothing
}

// Specialization for two-byte (16-bit) values
template<>
inline void byte_swap_sized<2>(char *data)
{
    auto   *p = reinterpret_cast<uint16_t *>(data);

    *p =   ((*p >> 8) & 0x00FF)
         | ((*p << 8) & 0xFF00);
}

// Specialization for four-byte (32-bit) values
template<>
inline void byte_swap_sized<4>(char *data)
{
    auto   *p = reinterpret_cast<uint32_t *>(data);

    *p =   ((*p >> 24) & 0x000000FF)
         | ((*p >>  8) & 0x0000FF00)
         | ((*p <<  8) & 0x00FF0000)
         | ((*p << 24) & 0xFF000000);
}

// Specialization for eight-byte (64-bit) values
template<>
inline void byte_swap_sized<8>(char *data)
{
    auto   *p = reinterpret_cast<uint64_t *>(data);
    auto    hi = (static_cast<uint32_t>((*p) >> 32));
    auto    lo = (static_cast<uint32_t>((*p) & 0x00000000FFFFFFFF));

    byte_swap_sized<4>(reinterpret_cast<char *>(&hi));
    byte_swap_sized<4>(reinterpret_cast<char *>(&lo));

    *p =   static_cast<uint64_t>(hi) | (static_cast<uint64_t>(lo) << 32);
}
/// \endcond

/// \brief  Swap the bytes of a given value.
/// \tparam T       The type of value.
/// \param value    The object to byte-swap.
/// \return The result of swapping the bytes of value.
template<typename T>
inline
typename std::enable_if<std::is_arithmetic_v<T> ||
                        std::is_pointer_v<T>, T>::type
byte_swap(T value)
{
    byte_swap_sized<sizeof(T)>(reinterpret_cast<char *>(&value));

    return value;
}


/// \brief  Swap the bytes of a value in-place, modifying the original value
///         rather than returning a new value.
/// \tparam T       The type of value.
/// \param value    The object to be byte-swapped
template<typename T>
inline void byte_swap_in_place(T &value)
{
    byte_swap_sized<sizeof(T)>(reinterpret_cast<char *>(&value));
}

/// \brief

/// \brief  Given a value of type T, return a new value of type T
///         whose bytes are ordered as Big Endian.
/// \tparam T       The type of value.
/// \param value    The value to be byte-ordered as Big Endian.
/// \return The result of value ordered as big-endian.
template<typename T>
inline T to_big_endian(T value)
{
#if BRACE_BYTE_ORDER == BRACE_BYTE_ORDER_BIG_ENDIAN
    return value;
#else
    return byte_swap(value);
#endif
}
/// \brief  Convert a value of type T to Big Endian. The conversion is done
///         in-place, modifying the original value rather than returning a
///         new value.
template<typename T>
inline void convert_to_big_endian(T &value)
{
#if BRACE_BYTE_ORDER == BRACE_BYTE_ORDER_BIG_ENDIAN
    // do nothing
#else
    byte_swap_in_place(value);
#endif
}
/// \brief  Given a value of type T in big-endian byte ordering,
///         return a new value of type T whose bytes are ordered
///         in native byte ordering.
template<typename T>
inline T from_big_endian(T value)
{
#if BRACE_BYTE_ORDER == BRACE_BYTE_ORDER_BIG_ENDIAN
    return value;
#else
    return byte_swap(value);
#endif
}
/// \brief  Convert a value of type T from Big Endian to native endian-ness.
///         The conversion is done in-place, modifying the original value rather
///         than returning a new value.
template<typename T>
inline void convert_from_big_endian(T &value)
{
#if BRACE_BYTE_ORDER == BRACE_BYTE_ORDER_BIG_ENDIAN
    // do nothing
#else
    byte_swap_in_place(value);
#endif
}
/// \brief  Given a value of type T, return a new value of type T
///         whose bytes are ordered as network-endian.
///
/// Network-endian is a synonym for big-endian. This template function
/// invokes to_big_endian.
template<typename T>
inline T to_network_endian(T value)
{
    return to_big_endian(value);
}
/// \brief  Convert a value of type T to Network Endian. The conversion is done
///         in-place, modifying the original value rather than returning a
///         new value.
template<typename T>
inline void convert_to_network_endian(T &value)
{
    convert_to_big_endian(value);
}

/// \brief  Given a value of type T in network-endian byte ordering,
///         return a new value of type T whose bytes are ordered
///         in native byte ordering.
///
/// Network-endian is a synonym for big-endian. This template function
/// invokes from_big_endian.
template<typename T>
inline T from_network_endian(T value)
{
    return from_big_endian(value);
}
/// \brief  Convert a value of type T from Network Endian to native endian-ness.
///         The conversion is done in-place, modifying the original value rather
///         than returning a new value.
template<typename T>
inline void convert_from_network_endian(T &value)
{
    convert_from_big_endian(value);
}

/// \brief  Given a value of type T, return a new value of type T
///         whose bytes are ordered as little-endian.
template<typename T>
inline T to_little_endian(T value)
{
#if BRACE_BYTE_ORDER == BRACE_BYTE_ORDER_LITTLE_ENDIAN
    return value;
#else
    return byte_swap(value);
#endif
}
/// \brief  Convert a value of type T to Little Endian. The conversion is done
///         in-place, modifying the original value rather than returning a
///         new value.
template<typename T>
inline void convert_to_little_endian(T &value)
{
#if BRACE_BYTE_ORDER == BRACE_BYTE_ORDER_LITTLE_ENDIAN
    (void)value;
    // do nothing
#else
    return byte_swap_in_place(value);
#endif
}

/// \brief  Given a value of type T in little-endian byte ordering,
///         return a new value of type T whose bytes are ordered
///         in native byte ordering.
template<typename T>
inline T from_little_endian(T value)
{
#if BRACE_BYTE_ORDER == BRACE_BYTE_ORDER_LITTLE_ENDIAN
    return value;
#else
    return byte_swap(value);
#endif
}
/// \brief  Convert a value of type T from Little Endian to native endian-ness.
///         The conversion is done in-place, modifying the original value rather
///         than returning a new value.
template<typename T>
inline void convert_from_little_endian(T &value)
{
#if BRACE_BYTE_ORDER == BRACE_BYTE_ORDER_LITTLE_ENDIAN
    (void)value;
    // do nothing
#else
    byte_swap_in_place(value);
#endif
}

/// \brief  Given a value of type T, return a new value of type T
///         whose bytes are ordered using the ordering specified
///         in \p endian.
template<typename T>
inline T to_endian(Endian endian, T value)
{
    return endian == Endian::Little ? to_little_endian(value)
                                    : to_big_endian(value);
}
/// \brief  Convert a value of type T to the native endian-ness.
///         The conversion is done in-place, modifying the original value
///         rather than returning a new value.
template<typename T>
inline void convert_to_endian(Endian endian, T &value)
{
    endian == Endian::Little ? convert_to_little_endian(value)
                             : convert_to_big_endian(value);
}
/// \brief  Given a value of type T in the byte ordering specified by endian,
///         return a new value of type T whose bytes are ordered
///         in native byte ordering.
template<typename T>
inline T from_endian(Endian endian, T value)
{
    return endian == Endian::Little ? from_little_endian(value)
                                    : from_big_endian(value);
}
/// \brief  Convert a value of type T from the specified endian-ness to native
///         endian-ness.
///         The conversion is done in-place, modifying the original value rather
///         than returning a new value.
template<typename T>
inline void convert_from_endian(Endian endian, T &value)
{
    endian == Endian::Little ? convert_from_little_endian(value)
                             : convert_from_big_endian(value);
}

}   // namespace brace

#endif  // BRACE_LIB_BTYEORDER_INC
