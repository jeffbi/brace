//////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2023 Jeffrey K. Bienstadt
//
// This file is part of the brace C++ library.
//
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.
//////////////////////////////////////////////////////////////////////

/// \file binastream.h
///
/// \author Jeff Bienstadt
#ifndef _BRACE_LIB_BINASTREAM_INC
#define _BRACE_LIB_BINASTREAM_INC

#include <algorithm>
#include <optional>

#include "binistream.h"

namespace brace
{

/// \brief  The BinArrayStreamImpl classe provides an implementation for a binary stream
///         backed by a fixed-length array of bytes.
class BinArrayStreamImpl : public BinStreamImpl
{
public:
    using byte_type     = uint8_t;
    using traits_type   = std::char_traits<char>;
    using int_type      = traits_type::int_type;
    using pos_type      = std::streampos;
    using off_type      = std::streamoff;

public:
    //
    // Construction/Destruction/Assignment
    //

    /// \brief  Construct a BinArrayStreamImpl object with a byte array of fixed length.
    /// \param a    Pointer to an array of bytes used as data to a binary stream.
    /// \param size Length of the array pointed to by a.
    ///
    /// The BinArrayStreamImpl object does NOT take ownership of the data pointed to by a.
    /// It is the caller's responsibility to maintain the life of the array throughout the
    /// live of the BinArrayStreamImpl object.
    BinArrayStreamImpl(byte_type *a, const size_t size)
        : _start{a}
        , _end{a + size}
        , _read_cur{a}
        , _write_cur{a}
    {}

    /// \brief  Construct a BinArrayStreamImpl object with an array of bytes specified by a range.
    /// \param a    Pointer to the beginning of the array of bytes used as data to a binary stream.
    /// \param aend Pointer to one past the and of the array whose start is pointed to by a.
    ///
    /// The BinArrayStreamImpl object does NOT take ownership of the data pointed to by a.
    /// It is the caller's responsibility to maintain the life of the array throughout the
    /// live of the BinArrayStreamImpl object.
    BinArrayStreamImpl(byte_type *a, byte_type *aend)
        : _start{a}
        , _end{aend}
        , _read_cur{a}
        , _write_cur{a}
    {}

    /// \brief The copy constructor is deleted. BinArrayStreamImpl is not copy constructable.
    BinArrayStreamImpl(const BinArrayStreamImpl &) = delete;
    /// \brief The copy assignment operator is deleted. BinArrayStreamImpl is not copy assignable.
    BinArrayStreamImpl & operator=(const BinArrayStreamImpl &) = delete;

    /// \brief  Construct a BinArrayStreamImpl object by moving data from another BinArrayStreamImpl object.
    /// \param other Another BinArrayStreamImpl object to move data from.
    BinArrayStreamImpl(BinArrayStreamImpl &&other) noexcept
        : _start{std::move(other._start)}
        , _end{std::move(other._end)}
        , _read_cur{std::move(other._read_cur)}
        , _write_cur{std::move(other._write_cur)}
        , _pb{std::move(other._pb)}
        , _state_bits{std::move(other._state_bits)}
    {
            other._start = nullptr;
            other._end = nullptr;
            other._read_cur = nullptr;
            other._write_cur = nullptr;
            other._pb.reset();
            other._state_bits = std::ios_base::goodbit;
    }

    /// \brief  Assign new values to a BinArrayStreamImpl object by moving values from another BinArrayStreamImpl object.
    /// \param rhs Another BinArrayStreamImpl object whose data is to be moved from.
    /// \return
    BinArrayStreamImpl & operator=(BinArrayStreamImpl &&rhs) noexcept
    {
        if (this != std::addressof(rhs))
        {
            BinStreamImpl::operator=(std::move(rhs));

            _start = rhs._start;
            rhs._start = nullptr;
            _end = rhs._end;
            rhs._end = nullptr;
            _read_cur = rhs._read_cur;
            rhs._read_cur = nullptr;
            _write_cur = rhs._write_cur;
            rhs._write_cur = nullptr;
            _pb = rhs._pb;
            rhs._pb.reset();
            _state_bits = rhs._state_bits;
            rhs._state_bits = std::ios_base::goodbit;
        }

        return *this;
    }

    /// \brief  Destroy a BinArrayStreamImpl object. This destructor has no effect as no resources are owned by the object.
    ~BinArrayStreamImpl() noexcept
    {}

    void swap(BinArrayStreamImpl &other)
    {
        if (this != std::addressof(other))
        {
            BinStreamImpl::swap(other);
            std::swap(_start, other._start);
            std::swap(_end, other._end);
            std::swap(_read_cur, other._read_cur);
            std::swap(_write_cur, other._write_cur);
            std::swap(_pb, other._pb);
            std::swap(_state_bits, other._state_bits);
        }
    }
public:
    //
    // Error and EOF states.
    //

    /// \brief  Check if errors occurred.
    /// \return \c true if errors occurred, false otherwise.
    bool error() override
    {
        return _state_bits & std::ios_base::failbit;
    }

    /// \brief  Check if a read operation has attempted to read beyond end of file.
    /// \return \c true if end of file has been detected.
    bool eof() override
    {
        return _state_bits & std::ios_base::eofbit;
    }

    /// \brief  Clear any error or end of file conditions
    void clearerr() override
    {
        _state_bits = std::ios_base::goodbit;
    }

    //
    //  Positioning
    //

    /// \brief  Return the current get area's position indicator.
    /// \return The current position indicator within the get area.
    pos_type tellg() override
    {
        return pos_type(_read_cur - _start);
    }

    /// \brief  Set the get area's position indicator relative to the beginning.
    /// \param pos  The new get position indicator relatve to the beginning.
    /// \return The new current position within the get area.
    pos_type seekg(pos_type pos) override
    {
        return seek(pos, _read_cur);
    }

    /// \brief  Set the get area's position indicator relative to the specified direction.
    /// \param off  The new get position indicator relative to the direction specified by dir
    /// \param dir  The direction from which the new position indicator is to be set. May be
    ///             one of \c std::ios_base::beg, \c std::ios_base:cur, or \c std::ios_base_end.
    /// \return The new current position within the get area.
    pos_type seekg(off_type off, std::ios_base::seekdir dir) override
    {
        return seek(off, dir, _read_cur);
    }

    /// \brief  Return the current put area's position indicator.
    /// \return The new current position within the put area.
    pos_type tellp() override
    {
        return pos_type(_write_cur - _start);
    }

    /// \brief  Set the put area's position indicator relative to the beginning.
    /// \param pos  The new put position indicator relatve to the beginning.
    /// \return The new current position within the put area.
    pos_type seekp(pos_type pos) override
    {
        return seek(pos, _write_cur);
    }

    /// \brief  Set the put area's position indicator relative to the specified direction.
    /// \param off  The new ptt position indicator relative to the direction specified by dir
    /// \param dir  The direction from which the new position indicator is to be set. May be
    ///             one of \c std::ios_base::beg, \c std::ios_base:cur, or \c std::ios_base_end.
    /// \return The new current position within the put area.
    pos_type seekp(off_type off, std::ios_base::seekdir dir) override
    {
        return seek(off, dir, _write_cur);
    }


    //
    // Get
    //

    /// \brief  Return the current byte in the input area, advancing the input position indicator.
    /// \return On success the current byte casted to \c int_type, otherwise \c traits_type::eof().
    int_type get() override
    {
        int_type    rv{traits_type::eof()};

        if (_pb.has_value())
        {
            rv = traits_type::to_int_type(_pb.value());
            _pb.reset();
        }
        else if (_read_cur < _end)
        {
            rv = traits_type::to_int_type(*_read_cur++);
        }
        else
        {
            set_eof();
        }

        return rv;
    }

    /// \brief  Return the current byte in the input area, without advancing the input position indicator.
    /// \return On success the current byte casted to \c int_type, otherwise \c traits_type::eof().
    int_type peek() override
    {
        int_type    rv{traits_type::eof()};

        if (_pb.has_value())
            rv = traits_type::to_int_type(_pb.value());
        else if (_read_cur < _end)
            rv = traits_type::to_int_type(*_read_cur);
        else
            set_eof();

        return rv;
    }

    /// \brief  Put a byte back into the input area.
    /// \param b    The byte to put back.
    /// \return On success the byte put back, casted to \c int_type, otherwise \c traits_type::eof().
    int_type putback(byte_type b) override
    {
        _pb = b;
        return traits_type::to_int_type(b);
    }

    /// \brief  Ignore up to count bytes from the input, stopping when the byte delim is encountered.
    /// \param count    The maximum number of bytes to ignore.
    /// \param delim    The delimiter byte. If this bytes is encountered in the input the operation stops.
    /// \return The number of bytes ignored.
    std::streamsize ignore(std::streamsize count, int_type delim) override
    {
        if (count <= 0)
            return 0;

        std::streamsize bytes_read{0};

        if (_pb.has_value())
        {
            _pb.reset();
            --count;
            ++bytes_read;
        }

        while ((_read_cur < _end) && count--)
        {
            byte_type   b{*_read_cur++};

            ++bytes_read;
            if (traits_type::eq_int_type(traits_type::to_int_type(b), delim))
                return bytes_read;
        }

        if ((_read_cur >= _end) && count)
            set_eof();

        return bytes_read;
    }

    /// \brief  Read up to count bytes from the input area into the array pointed to by s.
    /// \param s        A pointer to an array into which the bytes are to be read into.
    /// \param count    The maximum number of bytes to be read.
    /// \return The number of bytes read, which may be less than count if end of file or an error is encountered.
    std::streamsize read(byte_type *s, std::streamsize count) override
    {
        if (count <= 0)
            return 0;

        std::streamsize bytes_read{0};

        if (_pb.has_value())
        {
            *s++ = _pb.value();
            _pb.reset();
            --count;
            ++bytes_read;
        }

        if (count)
        {
            byte_type *p{_read_cur + count};

            if (p > _end)
            {
                p = _end;
                set_eof();
            }

            std::copy(_read_cur, p, s);
            bytes_read += p - _read_cur;
            _read_cur = p;
        }

        return bytes_read;
    }

    //
    // Put
    //

    /// \brief      Write a single byte into the put area.
    /// \param b    The byte to write.
    /// \return     On success the byte written, casted to \c int_type, traits_type::eof() otherwise.
    int_type put(byte_type b) override
    {
        if (_write_cur < _end)
        {
            *_write_cur++ = b;
            return traits_type::to_int_type(b);
        }

        set_eof();
        return traits_type::eof();
    }

    /// \brief      Write up to count bytes into the put area, from the array pointed to by s.
    /// \param s        A pointer to an array containing the bytes to be written.
    /// \param count    The maximum number of bytes to be written.
    /// \return     On success the byte written, casted to \c int_type, traits_type::eof() otherwise.
    std::streamsize write(const byte_type *s, std::streamsize count) override
    {
        if (count <= 0)
            return 0;

        if (_write_cur + count >= _end)
        {
            set_eof();
            count = _end - _write_cur;
        }

        const byte_type      *ends{s + count};
        std::copy(s, ends, _write_cur);
        _write_cur += count;

        return count;
    }

private:
    pos_type seek(pos_type pos, byte_type *&cur)
    {
        _pb.reset();
        cur = _start + pos;

        if (cur > _end)
            cur = _end;
        if (cur < _start)
            cur = _start;

        return cur - _start;
    }

    pos_type seek(off_type off, std::ios_base::seekdir dir, byte_type *&cur)
    {
        _pb.reset();

        byte_type *p{cur};

        switch (dir)
        {
            case std::ios_base::beg:
                p = _start + off;
                break;

            case std::ios_base::cur:
                p = cur + off;
                break;

            case std::ios_base::end:
                p = _end + off;
                break;
        }

        if (p < _start)
            p = _start;
        if (p > _end)
            p = _end;
        cur = p;

        return cur - _start;
    }

    void set_eof()
    {
        _state_bits |= std::ios_base::eofbit | std::ios_base::failbit;
    }

    void set_error()
    {
        _state_bits |= std::ios_base::failbit;
    }

private:
    byte_type                  *_start;     // Start of the byte array providing data
    byte_type                  *_end;       // One past the end of the array providing data
    byte_type                  *_read_cur;  // Pointer to current byte in the get area
    byte_type                  *_write_cur; // Pointer to current byte in the put area
    std::optional<byte_type>    _pb;        // result of putback
    std::ios_base::iostate      _state_bits{std::ios_base::goodbit};    // error/eof bits
};


/// \brief  The BinIArrayStream class provides a binary data input stream backed by a fixed-length byte array.
class BinIArrayStream : public BinIStream
{
public:
    using byte_type     = uint8_t;
    using traits_type   = std::char_traits<char>;
    using int_type      = traits_type::int_type;
    using pos_type      = std::streampos;
    using off_type      = std::streamoff;

public:
    /// \brief  Construct a BinIArrayStream object from a fixed-length array of bytes.
    /// \param a    Pointer to an array of bytes to be the backing data of the stream.
    /// \param size Size in bytes of the byte array pointed to by a.
    BinIArrayStream(byte_type *a, size_t size)
        : BinIStream(std::addressof(_impl))
        , _impl{a, size}
    {}

    /// \brief  Construct a BinIArrayStream object from a fixed-length array of bytes.
    /// \param a    Pointer to an array of bytes to be the backing data of the stream.
    /// \param aend Pointer to one past the last byte in the array pointed to by a.
    BinIArrayStream(byte_type *a, byte_type *aend)
        : BinIStream(&_impl)
        , _impl{a, aend}
    {}

    /// \brief  The copy constructor is deleted. The BinIArrayStream is not copy constructable.
    BinIArrayStream(const BinIArrayStream &) = delete;
    /// \brief  The copy assignment operator is deleted. The BinIArrayStream is not copy assignable.
    BinIArrayStream &operator=(const BinIArrayStream &) = delete;

    /// \brief  Construct a BinIArrayStream by moving data from another BinIArrayStream object.
    /// \param other    Another BinIArrayStream to move data from.
    BinIArrayStream(BinIArrayStream &&other) noexcept
        : BinIStream(std::move(other))
        , _impl{std::move(other._impl)}
    {}

    /// \brief  Assign to a BinIArrayStream object by moving data from another BinIArrayStream object
    BinIArrayStream &operator=(BinIArrayStream &&rhs) noexcept
    {
        if (this != std::addressof(rhs))
        {
            BinIStream::swap(rhs);
        }

        return *this;
    }

    void swap(BinIArrayStream &other)
    {
        if (this != std::addressof(other))
        {
            BinIStream::swap(other);
            _impl.swap(other._impl);
        }
    }

private:
    BinArrayStreamImpl  _impl;  // Implementation object.
};


/// \brief  The BinOArrayStream class provides a binary data output stream backed by a fixed-length byte array.
class BinOArrayStream : public BinOStream
{
public:
    using byte_type     = uint8_t;
    using traits_type   = std::char_traits<char>;
    using int_type      = traits_type::int_type;
    using pos_type      = std::streampos;
    using off_type      = std::streamoff;

public:
    /// \brief  Construct a BinOArrayStream object from a fixed-length array of bytes.
    /// \param a    Pointer to an array of bytes to be the backing data of the stream.
    /// \param size Size in bytes of the byte array pointed to by a.
    BinOArrayStream(byte_type *s, size_t size)
        : BinOStream(std::addressof(_impl))
        , _impl{s, size}
    {}

    /// \brief  Construct a BinOArrayStream object from a fixed-length array of bytes.
    /// \param a    Pointer to an array of bytes to be the backing data of the stream.
    /// \param aend Pointer to one past the last byte in the array pointed to by a.
    BinOArrayStream(byte_type *s, byte_type *send)
        : BinOStream(&_impl)
        , _impl{s, send}
    {}

    /// \brief  The copy constructor is deleted. The BinOArrayStream is not copy constructable.
    BinOArrayStream(const BinOArrayStream &) = delete;
    /// \brief  The copy assignment operator is deleted. The BinOArrayStream is not copy assignable.
    BinOArrayStream &operator=(const BinOArrayStream &) = delete;

    /// \brief  Construct a BinOArrayStream by moving data from another BinOArrayStream object.
    /// \param other    Another BinOArrayStream to move data from.
    BinOArrayStream(BinOArrayStream &&other) noexcept
        : BinOStream(std::move(other))
        , _impl{std::move(other._impl)}
    {}
    /// \brief  Assign to a BinOArrayStream object by moving data from another BinOArrayStream object
    BinOArrayStream &operator=(BinOArrayStream &&rhs) noexcept
    {
        if (this != std::addressof(rhs))
        {
            BinOStream::swap(rhs);
            _impl.swap(rhs._impl);
        }

        return *this;
    }

    void swap(BinOArrayStream &other)
    {
        if (this != std::addressof(other))
        {
            BinOStream::swap(other);
            _impl.swap(other._impl);
        }
    }

private:
    BinArrayStreamImpl  _impl;  // Implementation object.
};

/// \brief  The BinArrayStream class provides a binary data input/output stream backed by a fixed-length byte array.
class BinArrayStream : public BinIOStream
{
public:
    using byte_type     = uint8_t;
    using traits_type   = std::char_traits<char>;
    using int_type      = traits_type::int_type;
    using pos_type      = std::streampos;
    using off_type      = std::streamoff;

public:
    /// \brief  Construct a BinArrayStream object from a fixed-length array of bytes.
    /// \param a    Pointer to an array of bytes to be the backing data of the stream.
    /// \param size Size in bytes of the byte array pointed to by a.
    BinArrayStream(byte_type *a, size_t size)
        : BinIOStream(std::addressof(_impl))
        , _impl{a, size}
    {}

    /// \brief  Construct a BinArrayStream object from a fixed-length array of bytes.
    /// \param a    Pointer to an array of bytes to be the backing data of the stream.
    /// \param aend Pointer to one past the last byte in the array pointed to by a.
    BinArrayStream(byte_type *a, byte_type *aend)
        : BinIOStream(std::addressof(_impl))
        , _impl{a, aend}
    {}

    /// \brief  The copy constructor is deleted. The BinArrayStream is not copy constructable.
    BinArrayStream(const BinArrayStream &) = delete;
    /// \brief  The copy assignment operator is deleted. The BinArrayStream is not copy assignable.
    BinArrayStream &operator=(const BinArrayStream &) = delete;


    /// \brief  Construct a BinArrayStream by moving data from another BinArrayStream object.
    /// \param other    Another BinArrayStream to move data from.
    BinArrayStream(BinArrayStream &&other) noexcept
        : BinIOStream(std::move(other))
        , _impl{std::move(other._impl)}
    {
        BinIOStream::rdimpl(&_impl);
    }

    /// \brief  Assign to a BinArrayStream object by moving data from another BinArrayStream object
    BinArrayStream &operator=(BinArrayStream &&rhs) noexcept
    {
        if (this != std::addressof(rhs))
        {
            BinIOStream::operator=(std::move(rhs));
            _impl = std::move(rhs._impl);
            BinIOStream::rdimpl(&_impl);
        }

        return *this;
    }

    ~BinArrayStream()
    {}

    void swap(BinArrayStream &other)
    {
        if (this != std::addressof(other))
        {
            BinIStream::swap(other);
            _impl.swap(other._impl);
        }
    }
private:
    BinArrayStreamImpl  _impl;  // Implementation object.
};

}   // namespace brace

#endif  // _BRACE_LIB_BINASTREAM_INC
