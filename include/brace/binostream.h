//////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2023 Jeffrey K. Bienstadt
//
// This file is part of the brace C++ library.
//
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.
//////////////////////////////////////////////////////////////////////

/// \file binostream.h
///
/// \author Jeff Bienstadt
#ifndef _BRACE_LIB_BINOSTREAM_INC
#define _BRACE_LIB_BINOSTREAM_INC

#include "binios.h"
#include "binstreamimpl.h"

namespace brace
{

/// \brief  The BinOStream class is the base class for a binary output stream.
class BinOStream : virtual public BinIos
{
public:
    using byte_type     = uint8_t;
    using traits_type   = std::char_traits<char>;
    using int_type      = traits_type::int_type;
    using pos_type      = std::streampos;
    using off_type      = std::streamoff;

protected:
    /// \brief  Construct a BinOStream object with a stream implementation object.
    /// \param impl Pointer to a BinStreamImpl object providing the stream implementation.
    explicit BinOStream(BinStreamImpl *impl)
    {
        init(impl);
    }

    /// \brief  The copy constructor is deleted. BinOStream is not copy constructable.
    BinOStream(const BinOStream &) = delete;
    BinOStream & operator=(const BinOStream &) = delete;

    /// \brief  Construct a BinOStream object by moving data from another BinOStream object.
    /// \param other    Another BinOStream object to be moved from.
    BinOStream(BinOStream &&other) noexcept
        : BinIos(std::move(other))
    {}
    /// \brief  Assign to a BinOStream object by moving data from another BinOStream object.
    /// \param rhs  Another BinOStream object to be moved from.
    /// \return *this
    BinOStream & operator=(BinOStream &&rhs) noexcept
    {
        if (this != std::addressof(rhs))
        {
            BinIos::move(rhs);
        }
        return *this;
    }

    /// \brief Destroy a BinOStream object.
    virtual ~BinOStream()
    {}

    void swap(BinOStream &other)
    {
        if (this != std::addressof(other))
            BinIos::swap(other);
    }

public:
    //
    // Positioning
    //

    /// \brief  Return the current put area's position indicator.
    /// \return The new current position within the put area.
    pos_type tellp()
    {
        return rdimpl()->tellp();
    }

    /// \brief  Set the put area's position indicator relative to the beginning.
    /// \param pos  The new put position indicator relatve to the beginning.
    /// \return The new current position within the put area.
    BinOStream &seekp(pos_type pos)
    {
        clear_state_bit(std::ios_base::eofbit);
        rdimpl()->seekp(pos);
        //TODO: Handle result!!!

        return *this;
    }

    /// \brief  Set the put area's position indicator relative to the specified direction.
    /// \param off  The new ptt position indicator relative to the direction specified by dir
    /// \param dir  The direction from which the new position indicator is to be set. May be
    ///             one of \c std::ios_base::beg, \c std::ios_base:cur, or \c std::ios_base_end.
    /// \return The new current position within the put area.
    BinOStream &seekp(off_type off, std::ios_base::seekdir dir)
    {
        clear_state_bit(std::ios_base::eofbit);
        rdimpl()->seekp(off, dir);
        //TODO: Handle result!!!

        return *this;
    }

    /// \brief      Write a single byte into the put area.
    /// \param b    The byte to write.
    /// \return     On success the byte written, casted to \c int_type, traits_type::eof() otherwise.
    int_type put(byte_type b)
    {
        auto    rv{rdimpl()->put(b)};

        if (traits_type::eq_int_type(traits_type::to_int_type(b), traits_type::eof()))
            set_any_error_bits();

        return rv;
    }

    /// \brief      Write up to count bytes into the put area, from the array pointed to by s.
    /// \param s        A pointer to an array containing the bytes to be written.
    /// \param count    The maximum number of bytes to be written.
    /// \return     On success the byte written, casted to \c int_type, traits_type::eof() otherwise.
    std::streamsize write(const byte_type *s, std::streamsize count)
    {
        auto    rv{rdimpl()->write(s, count)};

        if (rv < count)
            set_any_error_bits();

        return rv;
    }

    //
    // Insertion operators
    //

    /// \brief  Insert a value into a BinOStream.
    /// \tparam T       Type of value to insert/
    /// \param value    Value to insert.
    /// \return *this
    template<typename T, typename std::enable_if<std::is_arithmetic_v<T> || std::is_pointer_v<T>>::type* = nullptr>
    BinOStream &operator<<(const T &value)
    {
        if (*this)
        {
            T v{to_endian(endian(), value)};
            write(reinterpret_cast<const byte_type *>(&v), sizeof(T));
        }

        return *this;
    }

private:
    void set_any_error_bits()
    {
        if (rdimpl()->eof())
            setstate(std::ios_base::eofbit);
        if (rdimpl()->error())
            setstate(std::ios_base::failbit);
    }
};

}   // namespace brace

#endif  // _BRACE_LIB_BINOSTREAM_INC
