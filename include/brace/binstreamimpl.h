//////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2023 Jeffrey K. Bienstadt
//
// This file is part of the brace C++ library.
//
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.
//////////////////////////////////////////////////////////////////////

/// \file binstreamimpl.h
///
/// \author Jeff Bienstadt
#ifndef _BRACE_LIB_BINSTREAMIMPL_INC
#define _BRACE_LIB_BINSTREAMIMPL_INC

#include "binios.h"
#include "file.h"

namespace brace
{

class BinStreamImpl
{
public:
    using byte_type     = uint8_t;
    using traits_type   = std::char_traits<char>;
    using int_type      = traits_type::int_type;
    using pos_type      = std::streampos;
    using off_type      = std::streamoff;

public:
    virtual ~BinStreamImpl()
    {}

public:
    //
    // Construction
    //

    BinStreamImpl()
    {}

    BinStreamImpl(const BinStreamImpl &) = delete;
    BinStreamImpl &operator=(const BinStreamImpl &) = delete;

    BinStreamImpl(BinStreamImpl &&other)
    {}

    BinStreamImpl &operator=(BinStreamImpl &&rhs)
    {
        return *this;
    }

protected:
    void swap(BinStreamImpl &other)
    {}

public:
    //
    // Error and EOF states.
    //

    /// \brief  Check if errors occurred.
    /// \return \c true if errors occurred, false otherwise.
    ///
    /// The base class version of this function has no effect and simply returns \c true.
    /// Derived classes may override this function to determine if any error condition exists.
    virtual bool error()
    {
        return false;
    }

    /// \brief  Check if a read operation has attempted to read beyond end of file.
    /// \return \c true if end of file has been detected.
    ///
    /// The base class version of this function has no effect and simply returns \c false.
    /// Derived classes may override this function to determine if end of file has been reached.
    virtual bool eof()
    {
        return false;
    }

    /// \brief  Clear any error or end of file conditions
    ///
    /// The base class version of this function has no effect and simply returns \c true.
    /// Derived classes may override this function to clear any error states.
    virtual void clearerr()
    {}

    //
    // Positioning
    //

    /// \brief  Return the current get area's position indicator.
    /// \return The current position indicator within the get area.
    ///
    /// The base class version of this function has no effect and simply returns \c pos_type(off_type(-1)).
    /// Derived classes may override this function to perform a tell operation.
    virtual pos_type tellg()
    {
        return pos_type(off_type(-1));
    }

    /// \brief  Set the get area's position indicator relative to the beginning.
    /// \param pos  The new get position indicator relatve to the beginning.
    /// \return The new current position within the get area.
    ///
    /// The base class version of this function has no effect and simply returns \c pos_type(off_type(-1)).
    /// Derived classes may override this function to perform a seek operation.
    virtual pos_type seekg(pos_type pos)
    {
        return pos_type(off_type(-1));
    }

    /// \brief  Set the get area's position indicator relative to the specified direction.
    /// \param off  The new get position indicator relative to the direction specified by dir
    /// \param dir  The direction from which the new position indicator is to be set. May be
    ///             one of \c std::ios_base::beg, \c std::ios_base:cur, or \c std::ios_base_end.
    /// \return The new current position within the get area.
    ///
    /// The base class version of this function has no effect and simply returns \c pos_type(off_type(-1)).
    /// Derived classes may override this function to perform a seek operation.
    virtual pos_type seekg(off_type off, std::ios_base::seekdir dir)
    {
        return pos_type(off_type(-1));
    }

    /// \brief  Return the current put area's position indicator.
    /// \return The new current position within the put area.
    ///
    /// The base class version of this function has no effect and simply returns \c pos_type(off_type(-1)).
    /// Derived classes may override this function to perform a tell operation.
    virtual pos_type tellp()
    {
        return pos_type(off_type(-1));
    }

    /// \brief  Set the put area's position indicator relative to the beginning.
    /// \param pos  The new put position indicator relatve to the beginning.
    /// \return The new current position within the put area.
    ///
    /// The base class version of this function has no effect and simply returns \c pos_type(off_type(-1)).
    /// Derived classes may override this function to perform a seek operation.
    virtual pos_type seekp(pos_type pos)
    {
        return pos_type(off_type(-1));
    }

    /// \brief  Set the put area's position indicator relative to the specified direction.
    /// \param off  The new ptt position indicator relative to the direction specified by dir
    /// \param dir  The direction from which the new position indicator is to be set. May be
    ///             one of \c std::ios_base::beg, \c std::ios_base:cur, or \c std::ios_base_end.
    /// \return The new current position within the put area.
    ///
    /// The base class version of this function has no effect and simply returns \c pos_type(off_type(-1)).
    /// Derived classes may override this function to perform a seek operation.
    virtual pos_type seekp(off_type off, std::ios_base::seekdir dir)
    {
        return pos_type(off_type(-1));
    }

    /// \brief  Write any unwritten data to the underlying "device".
    /// \return \c true if the operation was successful, \c false otherwise.
    ///
    /// The base class version of this function has no effect and simply returns \c true.
    /// Derived classes may override this function to perform a flush operation.
    virtual bool flush()
    {
        return true;
    }

    //
    // Get
    //

    /// \brief  Return the current byte in the input area, advancing the input position indicator.
    /// \return On success the current byte casted to \c int_type, otherwise \c traits_type::eof().
    ///
    /// The base class version of this function has no effect and simply returns \c traits_type::eof().
    /// Derived classes may override the function to provide the current input byte.
    virtual int_type get()
    {
        return traits_type::eof();
    }

    /// \brief  Return the current byte in the input area, without advancing the input position indicator.
    /// \return On success the current byte casted to \c int_type, otherwise \c traits_type::eof().
    ///
    /// The base class version of this function has no effect and simply returns \c traits_type::eof().
    /// Derived classes may override the function to provide the current input byte.
    virtual int_type peek()
    {
        return traits_type::eof();
    }

    /// \brief  Ignore up to count bytes from the input, stopping when the byte delim is encountered.
    /// \param count    The maximum number of bytes to ignore.
    /// \param delim    The delimiter byte. If this bytes is encountered in the input the operation stops.
    /// \return The number of bytes ignored.
    ///
    /// The base class version of this function has no effect and simply returns \c 0.
    /// Derived classes may override the function to ignore input bytes.
    virtual std::streamsize ignore(std::streamsize count, int_type delim)
    {
        return std::streamsize(0);
    }

    /// \brief  Read up to count bytes from the input area into the array pointed to by s.
    /// \param s        A pointer to an array into which the bytes are to be read into.
    /// \param count    The maximum number of bytes to be read.
    /// \return The number of bytes read, which may be less than count if end of file or an error is encountered.
    ///
    /// The base class version of this function has no effect and simply returns \c 0.
    /// Derived classes may override the function to read a sequence of bytes.
    virtual std::streamsize read(byte_type *s, std::streamsize count)
    {
        return std::streamsize(0);
    }

    //
    // Put
    //

    /// \brief      Write a single byte into the put area.
    /// \param b    The byte to write.
    /// \return     On success the byte written, casted to \c int_type, traits_type::eof() otherwise.
    ///
    /// The base class version of this function has no effect and simply returns \c traits_type::eof().
    /// Derived classes may override the function to write a byte.
    virtual int_type put(byte_type b)
    {
        return traits_type::eof();
    }

    /// \brief      Write up to count bytes into the put area, from the array pointed to by s.
    /// \param s        A pointer to an array containing the bytes to be written.
    /// \param count    The maximum number of bytes to be written.
    /// \return     On success the byte written, casted to \c int_type, traits_type::eof() otherwise.
    ///
    /// The base class version of this function has no effect and simply returns \c traits_type::eof().
    /// Derived classes may override the function to write a sequence of bytes.
    virtual std::streamsize write(const byte_type *s, std::streamsize count)
    {
        return traits_type::eof();
    }

    /// \brief  Put a byte back into the input area.
    /// \param b    The byte to put back.
    /// \return On success the byte put back, casted to \c int_type, otherwise \c traits_type::eof().
    ///
    /// The base class version of this function has no effect and simply returns \c traits_type::eof().
    /// Derived classes may override the function to put a byte back into the input area.
    virtual int_type putback(byte_type b)
    {
        return traits_type::eof();
    }
};

}   // namespace brace

#endif  // _BRACE_LIB_BINSTREAMIMPL_INC
