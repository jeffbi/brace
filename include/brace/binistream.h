//////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2023 Jeffrey K. Bienstadt
//
// This file is part of the brace C++ library.
//
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.
//////////////////////////////////////////////////////////////////////

/// \file binistream.h
///
/// \author Jeff Bienstadt
#ifndef _BRACE_LIB_BINISTREAM_INC
#define _BRACE_LIB_BINISTREAM_INC

#include "binios.h"
#include "binstreamimpl.h"

#include "binostream.h"

namespace brace
{

/// \brief  The BinIStream class is the base class for a binary input stream.
class BinIStream : virtual public BinIos
{
public:
    using byte_type     = uint8_t;                  ///< Type used to represent a byte
    using traits_type   = std::char_traits<char>;   ///< Type used for character traits
    using int_type      = traits_type::int_type;    ///< Type used for integers
    using pos_type      = std::streampos;           ///< Type used for positioning
    using off_type      = std::streamoff;           ///< Type used for offsets

protected:
    /// \brief  Construct a BinIStream object with a stream implementation object.
    /// \param impl Pointer to a BinStreamImpl object providing the stream implementation.
    explicit BinIStream(BinStreamImpl *impl)
        : _gcount{0}
    {
        init(impl);
    }

    /// \brief  The copy constructor is deleted. BinIStream is not copy constructable.
    BinIStream(const BinIStream &) = delete;
    /// \brief  The copy assignment operator is deleted. BinIStream is not copy assignable.
    BinIStream & operator=(const BinIStream &) = delete;

    /// \brief  Construct a BinIStream object by moving data from another BinIStream object.
    /// \param other    Another BinIStream object to be moved from.
    BinIStream(BinIStream &&other) noexcept
        : BinIos(std::move(other))
        , _gcount{other._gcount}
    {
        other._gcount = 0;
    }

    /// \brief  Assign to a BinIStream object by moving data from another BinIStream object.
    /// \param rhs  Another BinIStream object to be moved from.
    /// \return *this
    BinIStream & operator=(BinIStream &&rhs) noexcept
    {
        if (this != std::addressof(rhs))
        {
            BinIos::move(rhs);
            _gcount = rhs._gcount;
            rhs._gcount = 0;
        }
        return *this;
    }

    /// \brief  Destroy a BinIStream object.
    virtual ~BinIStream()
    {}

public:

    //
    // Unformatted input
    //

    /// \brief  Extract one byte from the stream and return it if available. Otherwise return traits_type::eof()
    ///         and set \c failbit and \c eofbit.
    /// \return The extracted byte or traits_type::eof().
    int_type get()
    {
        int_type    rv{rdimpl()->get()};

        if (traits_type::eq_int_type(rv, traits_type::eof()))
        {
            setstate(std::ios_base::failbit | std::ios_base::eofbit);
            _gcount = 0;
        }
        else
        {
            _gcount = 1;
        }

        return rv;
    }
    /// \brief      Extract one byte and store it into \c b if available. Otherwise leave \c b unmodified and
    ///             set \c failbit and \c eofbit.
    /// \param b    Reference to byte to store result.
    /// \return     *this
    BinIStream &get(byte_type &b)
    {
        int_type    temp{get()};

        if (!traits_type::eq_int_type(temp, traits_type::eof()))
            b = static_cast<byte_type>(temp);

        return *this;
    }
    /// \brief          Extract bytes and store them into to the byte array pointed to by s.
    /// \param s        Pointer to an array of bytes in which to store extracted bytes.
    /// \param count    Length of the array pointed to by s.
    /// \param delim    Delimiting byte to stop the extraction.
    /// \return         *this
    BinIStream &get(byte_type *s, std::streamsize count, byte_type delim)
    {
        std::streamsize     read_count{0};
        while (count--)
        {
            int_type    temp{rdimpl()->peek()};

            if (traits_type::eq_int_type(temp, traits_type::eof()))
            {
                setstate(std::ios_base::eofbit);
                break;
            }

            byte_type   b{static_cast<byte_type>(temp)};

            if (b == delim)
            {
                // don't extract the delim byte.
                //TODO: But do we need to push it back?
                break;
            }

            rdimpl()->get();

            *s++ = b;
            ++read_count;
        }

        _gcount = read_count;
        return *this;
    }

    /// \brief  Return the current byte in the input area, without advancing the input position indicator.
    /// \return On success the current byte casted to \c int_type, otherwise \c traits_type::eof().
    int_type peek()
    {
        _gcount = 0;
        if (good())
            return rdimpl()->peek();
        return traits_type::eof();
    }

    /// \brief  Put a byte back into the input area.
    /// \param b    The byte to put back.
    /// \return On success the byte put back, casted to \c int_type, otherwise \c traits_type::eof().
    BinIStream &putback(byte_type b)
    {
        clear_state_bit(std::ios_base::eofbit);

        auto    rv{rdimpl()->putback(b)};

        _gcount = 0;
        return *this;
    }

    /// \brief  Ignore up to count bytes from the input, stopping when the byte delim is encountered.
    /// \param count    The maximum number of bytes to ignore.
    /// \param delim    The delimiter byte. If this bytes is encountered in the input the operation stops.
    /// \return *this
    BinIStream &ignore(std::streamsize count = 1, int_type delim = traits_type::eof())
    {
        auto read_count{rdimpl()->ignore(count, delim)};
        _gcount = read_count;
        if (rdimpl()->error())
            setstate(std::ios_base::failbit);
        if (rdimpl()->eof())
            setstate(std::ios_base::eofbit);
        return *this;
    }

    /// \brief  Read up to count bytes from the input area into the array pointed to by s.
    /// \param s        A pointer to an array into which the bytes are to be read into.
    /// \param count    The maximum number of bytes to be read.
    /// \return The number of bytes read, which may be less than count if end of file or an error is encountered.
    BinIStream &read(byte_type *s, std::streamsize count)
    {
        if (count <= 0)
        {
            _gcount = 0;
        }
        else
        {
            auto    read_count{rdimpl()->read(s, count)};

            _gcount = read_count;   // Note:    _gcount could be less than count if not enough bytes were read.
            if (rdimpl()->error())
                setstate(std::ios_base::failbit);
            if (rdimpl()->eof())
                setstate(std::ios_base::eofbit | std::ios_base::failbit);
        }

        return *this;
    }

    /// \brief  Return number of bytes read on the last input operation.
    /// \return The number of bytes read.
    std::streamsize gcount()
    {
        return _gcount;
    }

    //
    // Positioning
    //

    /// \brief  Return the current get area's position indicator.
    /// \return The current position indicator within the get area.
    pos_type tellg()
    {
        return rdimpl()->tellg();
    }

    /// \brief  Set the get area's position indicator relative to the beginning.
    /// \param pos  The new get position indicator relatve to the beginning.
    /// \return The new current position within the get area.
    BinIStream &seekg(pos_type pos)
    {
        clear_state_bit(std::ios_base::eofbit);
        rdimpl()->seekg(pos);
        //TODO: Handle result!!!

        return *this;
    }

    /// \brief  Set the get area's position indicator relative to the specified direction.
    /// \param off  The new get position indicator relative to the direction specified by dir
    /// \param dir  The direction from which the new position indicator is to be set. May be
    ///             one of \c std::ios_base::beg, \c std::ios_base:cur, or \c std::ios_base_end.
    /// \return The new current position within the get area.
    BinIStream &seekg(off_type off, std::ios_base::seekdir dir)
    {
        clear_state_bit(std::ios_base::eofbit);
        rdimpl()->seekg(off, dir);
        //TODO: Handle result!!!

        return *this;
    }


    //
    // Extraction operators
    //

    /// \brief  Extract values from an input byte stream.
    /// \tparam T       Type of value to extract.
    /// \param value    Value to extract.
    /// \return *this
    template<typename T, typename std::enable_if<std::is_arithmetic_v<T> || std::is_pointer_v<T>>::type* = nullptr>
    BinIStream &operator>>(T &value)
    {
        if (*this)
        {
            T   v{0};

            if (read(reinterpret_cast<byte_type *>(&v), sizeof(T)))
            {
                value = to_endian(endian(), v);
            }
        }

        return *this;
    }

#if 0
    /// \brief  Extract a single byte from an input byte stream.
    /// \param b    Byte value to be read.
    /// \return *this
    //template <>
    //BinIStream &operator>><byte_type>(byte_type &b)
    //{
    //    return get(b);
    //}
#endif

protected:
    /// @brief  Swap the contents of this BinIStream object with another.
    /// @param other    Another BinIStream object whose contents is to be swapped.
    void swap(BinIStream &other)
    {
        if (this != std::addressof(other))
        {
            BinIos::swap(other);
            std::swap(_gcount, other._gcount);
        }
    }

private:
    std::streamsize _gcount;
};

/// \brief  The BinIOStream class is the base class for binary input/output streams.
class BinIOStream : public BinIStream,  public BinOStream
{
public:
    using byte_type     = uint8_t;                  ///< Type used to represent a byte
    using traits_type   = std::char_traits<char>;   ///< Type used for character traits
    using int_type      = traits_type::int_type;    ///< Type used for integers
    using pos_type      = std::streampos;           ///< Type used for positioning
    using off_type      = std::streamoff;           ///< Type used for offsets

public:
    /// \brief Construct a BinIOStream object with a binary stream implementation object.
    /// \param impl     Pointer to a BinStreamImpl object.
    explicit BinIOStream(BinStreamImpl *impl)
        : BinIStream(impl)
        , BinOStream(impl)
    {}

    /// \brief  The copy constructor is deleted. BinIOStream is not copy constructable.
    BinIOStream(const BinIOStream &) = delete;
    /// \brief  The copy assignment operator is deleted. BinIOStream is not copy assignable.
    BinIOStream & operator=(const BinIOStream &) = delete;

    /// \brief  Construct a BinIOStream object by moving data from another BinIOStream object.
    /// \param other    Another BinIOStream object from which to move.
    BinIOStream(BinIOStream &&other)
        : BinIStream(std::move(other))
        , BinOStream(std::move(other))
    {}

    /// \brief  Assign a BinIOStream object by moving data from another BinIOStream object.
    /// \param rhs  Another BinIOStream object from which to move.
    /// \return *this
    BinIOStream &operator=(BinIOStream &&rhs)
    {
        BinIStream::operator=(std::move(rhs));
        BinOStream::operator=(std::move(rhs));

        return *this;
    }

    /// \brief  Destroy a BinIOStream object.
    ~BinIOStream()
    {}

    /// \brief  Swap the contents of this BinIOStream object with another.
    /// \param other    Another BinIOStream object whose contents is to be swapped.
    void swap(BinIOStream &other)
    {
        if (this != std::addressof(other))
            BinIStream::swap(other);
    }
};

}   // namespace brace

#endif  // _BRACE_LIB_BINISTREAM_INC
