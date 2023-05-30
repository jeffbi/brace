//////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2023 Jeffrey K. Bienstadt
//
// This file is part of the brace C++ library.
//
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.
//////////////////////////////////////////////////////////////////////

/// \file binfstream.h
///
/// \author Jeff Bienstadt
#ifndef _BRACE_LIB_BINFSTREAM_INC
#define _BRACE_LIB_BINFSTREAM_INC

#include <filesystem>

#include "binistream.h"
#include "binostream.h"
#include "file.h"

namespace brace
{

/// \brief  The BinFileStreamImpl class provides an implementation for a binary stream
///         backed by a file.
class BinFileStreamImpl : public BinStreamImpl
{
public:
    using byte_type     = uint8_t;                  ///< Type used to represent a byte
    using traits_type   = std::char_traits<char>;   ///< Type used for character traits
    using int_type      = traits_type::int_type;    ///< Type used for integers
    using pos_type      = std::streampos;           ///< Type used for positioning
    using off_type      = std::streamoff;           ///< Type used for offsets

public:
    //
    // Construction/Destruction/Assignment
    //

    /// \brief  Default-construct a BinFileStreamImpl object. No file is associated with the object.
    BinFileStreamImpl()
    {}

    /// \brief  The copy constructor is deleted. BinFileStreamImpl objects are not copy constructable.
    BinFileStreamImpl(const BinFileStreamImpl &) = delete;
    /// \brief  The copy assignment operator is deleted. BinFileStreamImpl objects are not copy assignable.
    BinFileStreamImpl & operator=(const BinFileStreamImpl &) = delete;

    /// \brief  Construct a BinFileStreamImpl object by moving data from another BinFileStreamImpl object.
    /// \param other    Another BinFileStreamImpl object from which to move.
    BinFileStreamImpl(BinFileStreamImpl &&other) noexcept
        : _file{std::move(other._file)}
    {
    }
    /// \brief  Assign to a BinFileStreamImpl object by moving data from another BinFileStreamImpl object.
    ///         Before moving, the file associated with the moved-to object is closed.
    /// \param rhs  Another BinFileStreamImpl object from which to move.
    /// \return *this;
    BinFileStreamImpl & operator=(BinFileStreamImpl &&rhs) noexcept
    {
        if (this != std::addressof(rhs))
        {
            close();
            _file = std::move(rhs._file);
        }

        return *this;
    }

    /// \brief Destroy a BinFileStreamImpl object. Any associated file is closed.
    ~BinFileStreamImpl()
    {
        close();
    }

    /// \brief  Swap the contents of this BinFileStreamImpl object with another.
    /// \param other    Another BinFileStreamImpl object whose contents is to be swapped.
    void swap(BinFileStreamImpl &other)
    {
        if (this != std::addressof(other))
            std::swap(_file, other._file);
    }

    //
    // File operations
    //

    /// \brief  Open the specified file using the specified mode.
    /// \param filename     A nul-terminated C-style string containing the name of the file to open.
    /// \param mode         The mode in which to open the file. May be a combination of the
    ///                     \c std::ios_base::openmode modes. The std::ios_base::binary mode will
    ///                     always be added.
    /// \return \c true if the file was opened successfully, \c false otherwise.
    bool open(const char *filename, std::ios_base::openmode mode)
    {
        if (is_open())
            return false;

        return _file.open(filename, mode | std::ios_base::binary);
    }

    /// \brief  Open the specified file using the specified mode.
    /// \param filename     A std::string containing the name of the file to open.
    /// \param mode         The mode in which to open the file. May be a combination of the
    ///                     \c std::ios_base::openmode modes. The std::ios_base::binary mode will
    ///                     always be added.
    /// \return \c true if the file was opened successfully, \c false otherwise.
    bool open(const std::string &filename, std::ios_base::openmode mode)
    {
        return open(filename.c_str(), mode);
    }

    /// \brief  Open the specified file using the specified mode.
    /// \param filename     std::filesystem::path object containing the name of the file to open.
    /// \param mode         The mode in which to open the file. May be a combination of the
    ///                     \c std::ios_base::openmode modes. The std::ios_base::binary mode will
    ///                     always be added.
    /// \return \c true if the file was opened successfully, \c false otherwise.
    bool open(const std::filesystem::path &filename, std::ios_base::openmode mode)
    {
        return open(filename.c_str(), mode);
    }

#if defined(_WIN32)
    /// \brief  Open the specified file using the specified mode.
    ///         This overload is available only if the \c _WIN32 macro is defined.
    /// \param filename     A nul-terminated wide-character C-style string containing the name of the file to open.
    /// \param mode         The mode in which to open the file. May be a combination of the
    ///                     \c std::ios_base::openmode modes. The std::ios_base::binary mode will
    ///                     always be added.
    /// \return \c true if the file was opened successfully, \c false otherwise.
    bool open(const wchar_t *filename, std::ios_base::openmode mode)
    {
        if (is_open())
            return false;

        return _file.open(filename, mode | std::ios_base::binary);
    }
#endif  // _Win32

    /// \brief  Determine of an open file is associated with this BinFileStreamImpl object.
    /// \return \c true if an open file is associated with this object, \c false otherwise.
    bool is_open() const noexcept
    {
        return _file.is_open();
    }

    /// \brief  Close an open file associated with this BinFileStreamImpl object.
    void close()
    {
        if (is_open())
            _file.close();
    }


    //
    // Error and EOF states.
    //

    /// \brief  Check if errors occurred.
    /// \return \c true if errors occurred, false otherwise.
    bool error() override
    {
        return _file.error();
    }

    /// \brief  Check if a read operation has attempted to read beyond end of file.
    /// \return \c true if end of file has been detected.
    bool eof() override
    {
        return _file.eof();
    }

    /// \brief  Clear any error or end of file conditions
    void clearerr() override
    {
        _file.clearerr();
    }

    //
    // Positioning
    //
    /// \brief  Return the current get area's position indicator.
    /// \return The current position indicator within the get area.
    pos_type tellg() override
    {
        return _file.tell();
    }

    /// \brief  Set the get area's position indicator relative to the beginning.
    /// \param pos  The new get position indicator relatve to the beginning.
    /// \return The new current position within the get area.
    pos_type seekg(pos_type pos) override
    {
        int err = _file.seek(pos, std::ios_base::beg);
        //TODO: Handle errors!
        return pos_type(std::streamoff(_file.tell()));
    }

    /// \brief  Set the get area's position indicator relative to the specified direction.
    /// \param off  The new get position indicator relative to the direction specified by dir
    /// \param dir  The direction from which the new position indicator is to be set. May be
    ///             one of \c std::ios_base::beg, \c std::ios_base:cur, or \c std::ios_base_end.
    /// \return The new current position within the get area.
    pos_type seekg(off_type off, std::ios_base::seekdir dir) override
    {
        int err{_file.seek(off, dir)};
        //TODO: Handle errors!
        auto t = _file.tell();
        return pos_type(std::streamoff(_file.tell()));
    }

    /// \brief  Return the current put area's position indicator.
    /// \return The new current position within the put area.
    pos_type tellp() override
    {
        return _file.tell();
    }

    /// \brief  Set the put area's position indicator relative to the beginning.
    /// \param pos  The new put position indicator relatve to the beginning.
    /// \return The new current position within the put area.
    pos_type seekp(pos_type pos) override
    {
        int err = _file.seek(pos, std::ios_base::beg);
        //TODO: Handle errors!
        return pos_type(std::streamoff(_file.tell()));
    }

    /// \brief  Set the put area's position indicator relative to the specified direction.
    /// \param off  The new ptt position indicator relative to the direction specified by dir
    /// \param dir  The direction from which the new position indicator is to be set. May be
    ///             one of \c std::ios_base::beg, \c std::ios_base:cur, or \c std::ios_base_end.
    /// \return The new current position within the put area.
    pos_type seekp(off_type off, std::ios_base::seekdir dir) override
    {
        int err{_file.seek(off, dir)};
        //TODO: Handle errors!
        return pos_type(std::streamoff(_file.tell()));
    }

    /// \brief  Write any unwritten data to the underlying "device".
    /// \return \c true if the operation was successful, \c false otherwise.
    bool flush() override
    {
        return _file.flush();
    }

    //
    // Get
    //

    /// \brief  Return the current byte in the input area, advancing the input position indicator.
    /// \return On success the current byte casted to \c int_type, otherwise \c traits_type::eof().
    int_type get() override
    {
        return _file.getc();
    }

    /// \brief  Return the current byte in the input area, without advancing the input position indicator.
    /// \return On success the current byte casted to \c int_type, otherwise \c traits_type::eof().
    int_type peek() override
    {
        int_type    rv{_file.getc()};

        if (!traits_type::eq_int_type(rv, traits_type::eof()))
            _file.ungetc(rv);

        return rv;
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

        while (count--)
        {
            auto    c{_file.getc()};

            if (traits_type::eq_int_type(c, traits_type::eof()))
                break;

            ++bytes_read;
            if (traits_type::eq_int_type(c, delim))
                break;
        }

        return bytes_read;
    }

    /// \brief  Read up to count bytes from the input area into the array pointed to by s.
    /// \param s        A pointer to an array into which the bytes are to be read into.
    /// \param count    The maximum number of bytes to be read.
    /// \return The number of bytes read, which may be less than count if end of file or an error is encountered.
    std::streamsize read(byte_type *s, std::streamsize count) override
    {
        std::streamsize bytes_read{0};
        byte_type      *s_part{s};

        while (count > 0)
        {
            const size_t    toread{std::min(static_cast<size_t>(count), std::numeric_limits<size_t>::max())};
            const size_t    didread{_file.read(s_part, 1, toread)};

            if (didread)
            {
                count -= didread;
                s_part += didread;
                bytes_read += didread;
            }
            if (didread < toread)
                break;
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
        return _file.putc(static_cast<int>(b));
    }

    /// \brief      Write up to count bytes into the put area, from the array pointed to by s.
    /// \param s        A pointer to an array containing the bytes to be written.
    /// \param count    The maximum number of bytes to be written.
    /// \return     On success the byte written, casted to \c int_type, traits_type::eof() otherwise.
    std::streamsize write(const byte_type *s, std::streamsize count) override
    {
        std::streamsize bytes_written{0};
        const byte_type *s_part{s};

        while (count > 0)
        {
            const size_t    towrite{std::min(static_cast<size_t>(count), std::numeric_limits<size_t>::max())};
            const size_t    didwrite{_file.write(s_part, 1, towrite)};

            if (didwrite)
            {
                count -= didwrite;
                s_part += didwrite;
                bytes_written += didwrite;
            }
            if (didwrite < towrite)
                break;
        }

        return bytes_written;
    }

    /// \brief  Put a byte back into the input area.
    /// \param b    The byte to put back.
    /// \return On success the byte put back, casted to \c int_type, otherwise \c traits_type::eof().
    int_type putback(byte_type b) override
    {
        return _file.ungetc(b);
    }

private:
    File    _file;
};

/// \brief  The BinIFStream class provides a binary input stream backed by a file.
class BinIFStream : public BinIStream
{
public:
    using byte_type     = uint8_t;                  ///< Type used to represent a byte
    using traits_type   = std::char_traits<char>;   ///< Type used for character traits
    using int_type      = traits_type::int_type;    ///< Type used for integers
    using pos_type      = std::streampos;           ///< Type used for positioning
    using off_type      = std::streamoff;           ///< Type used for offsets

public:
    /// \brief  Default construct a BinIFStream object. No file is associated with the constructed object.
    BinIFStream()
        : BinIStream(std::addressof(_impl))
    {}

    /// \brief  Construct a BinIFStream object with a filename and an open mode specifier.
    /// \param filename Pointer to a nul-terminated C-style string containing the name of
    ///                 the file to be associated with the constructed object.
    /// \param mode     The mode in which to open the file. May be a combination of the
    ///                 \c std::ios_base::openmode modes. The std::ios_base::binary mode will
    ///                 always be added.
    explicit BinIFStream(const char *filename, std::ios_base::openmode mode = std::ios_base::in)
        : BinIStream(std::addressof(_impl))
    {
        open(filename, mode);
    }

    /// \brief  Construct a BinIFStream object with a filename and an open mode specifier.
    /// \param filename std::string containing the name of
    ///                 the file to be associated with the constructed object.
    /// \param mode     The mode in which to open the file. May be a combination of the
    ///                 \c std::ios_base::openmode modes. The std::ios_base::binary mode will
    ///                 always be added.
#if 0
    explicit BinIFStream(const std::string &filename, std::ios_base::openmode mode = std::ios_base::in)
        : BinIStream(std::addressof(_impl))
    {
        open(filename, mode);
        //TODO: Implement me properly!!!
    }
#else
    explicit BinIFStream(const std::string &filename, std::ios_base::openmode mode = std::ios_base::in)
        : BinIFStream(filename.c_str(), mode)
    {}
#endif

    /// \brief  Construct a BinIFStream object with a filename and an open mode specifier.
    /// \param filename std::filesystem::path object containing the name of
    ///                 the file to be associated with the constructed object.
    /// \param mode     The mode in which to open the file. May be a combination of the
    ///                 \c std::ios_base::openmode modes. The std::ios_base::binary mode will
    ///                 always be added.
#if 0
    explicit BinIFStream(const std::filesystem::path &filename, std::ios_base::openmode mode = std::ios_base::in)
        : BinIStream(std::addressof(_impl))
    {
        open(filename, mode);
        //TODO: Implement me properly!!!
    }
#else
    explicit BinIFStream(const std::filesystem::path &filename, std::ios_base::openmode mode = std::ios_base::in)
        : BinIFStream(filename.c_str(), mode)
    {}
#endif

#if defined(_WIN32)
    /// \brief  Construct a BinIFStream object with a filename and an open mode specifier.
    ///         This overload is available only if the \c _WIN32 macro is defined.
    /// \param filename Pointer to a nul-terminated wide-character C-style string containing the name of
    ///                 the file to be associated with the constructed object.
    /// \param mode     The mode in which to open the file. May be a combination of the
    ///                 \c std::ios_base::openmode modes. The std::ios_base::binary mode will
    ///                 always be added.
    explicit BinIFStream(const wchar_t *filename, std::ios_base::openmode mode = std::ios_base::in)
        : BinIStream(std::addressof(_impl))
    {
        open(filename, mode);
    }
#endif

    /// \brief  Destroy a BinIFStream object. Any associated file is closed.
    ~BinIFStream()
    {
        close();
    }

    /// \brief  Swap the contents of this BinIFStream object with another.
    /// \param other    Another BinIFStream object whose contents is to be swapped.
    void swap(BinIFStream &other)
    {
        if (this != std::addressof(other))
        {
            BinIStream::swap(other);
            _impl.swap(other._impl);
        }
    }

    //
    // File operations
    //

    /// \brief  Open the specified file using the specified mode.
    /// \param filename     A nul-terminated C-style string containing the name of the file to open.
    /// \param mode         The mode in which to open the file. May be a combination of the
    ///                     \c std::ios_base::openmode modes. The std::ios_base::binary mode will
    ///                     always be added.
    void open(const char *filename, std::ios_base::openmode mode = std::ios_base::in)
    {
        if (_impl.open(filename, mode | std::ios_base::binary))
            clear();
        else
            setstate(std::ios_base::failbit);
    }

    /// \brief  Open the specified file using the specified mode.
    /// \param filename     A std::string containing the name of the file to open.
    /// \param mode         The mode in which to open the file. May be a combination of the
    ///                     \c std::ios_base::openmode modes. The std::ios_base::binary mode will
    ///                     always be added.
    void open(const std::string &filename, std::ios_base::openmode mode = std::ios_base::in)
    {
        open(filename.c_str(), mode);
    }

    /// \brief  Open the specified file using the specified mode.
    /// \param filename     std::filesystem::path object containing the name of the file to open.
    /// \param mode         The mode in which to open the file. May be a combination of the
    ///                     \c std::ios_base::openmode modes. The std::ios_base::binary mode will
    ///                     always be added.
    void open(const std::filesystem::path &filename, std::ios_base::openmode mode = std::ios_base::in)
    {
        open(filename.c_str(), mode);
    }

#if defined(_WIN32)
    /// \brief  Open the specified file using the specified mode.
    ///         This overload is available only if the \c _WIN32 macro is defined.
    /// \param filename     A nul-terminated wide-character C-style string containing the name of the file to open.
    /// \param mode         The mode in which to open the file. May be a combination of the
    ///                     \c std::ios_base::openmode modes. The std::ios_base::binary mode will
    ///                     always be added.
    void open(const wchar_t *filename, std::ios_base::openmode mode = std::ios_base::in)
    {
        if (_impl.open(filename, mode | std::ios_base::binary))
            clear();
        else
            setstate(std::ios_base::failbit);
    }
#endif  // _Win32

    /// \brief  Determine of an open file is associated with this BinIFStream object.
    /// \return \c true if an open file is associated with this object, \c false otherwise.
    bool is_open() const noexcept
    {
        return _impl.is_open();
    }

    /// \brief  Close an open file associated with this BinIFStream object.
    void close()
    {
        if (is_open())
        {
            _impl.close();
        }
    }

protected:
    /// \brief  Set the fail and/or eof bits from the current state of the
    ///         underlying FILE object
    void set_fail_or_eof()
    {
        if (_impl.eof())
            setstate(std::ios_base::eofbit);
        if (_impl.error())
            setstate(std::ios_base::failbit);
    }

private:
    BinFileStreamImpl   _impl;
};

/// \brief  The BinOFStream class provides a binary output stream backed by a file.
class BinOFStream : public BinOStream
{
public:
    using byte_type     = uint8_t;                  ///< Type used to represent a byte
    using traits_type   = std::char_traits<char>;   ///< Type used for character traits
    using int_type      = traits_type::int_type;    ///< Type used for integers
    using pos_type      = std::streampos;           ///< Type used for positioning
    using off_type      = std::streamoff;           ///< Type used for offsets

public:
    /// \brief  Default construct a BinOFStream object. No file is associated with the constructed object.
    BinOFStream()
        : BinOStream(std::addressof(_impl))
    {}

    /// \brief  Construct a BinOFStream object with a filename and an open mode specifier.
    /// \param filename Pointer to a nul-terminated C-style string containing the name of
    ///                 the file to be associated with the constructed object.
    /// \param mode     The mode in which to open the file. May be a combination of the
    ///                 \c std::ios_base::openmode modes. The std::ios_base::binary mode will
    ///                 always be added.
    explicit BinOFStream(const char *filename, std::ios_base::openmode mode = std::ios_base::out)
        : BinOFStream()
    {
        open(filename, mode);
    }

    /// \brief  Construct a BinFStream object with a filename and an open mode specifier.
    /// \param filename std::string containing the name of
    ///                 the file to be associated with the constructed object.
    /// \param mode     The mode in which to open the file. May be a combination of the
    ///                 \c std::ios_base::openmode modes. The std::ios_base::binary mode will
    ///                 always be added.
    explicit BinOFStream(const std::string &filename, std::ios_base::openmode mode = std::ios_base::out)
        : BinOFStream()
    {
        open(filename, mode);
    }

    /// \brief  Construct a BinOFStream object with a filename and an open mode specifier.
    /// \param filename std::filesystem::path object containing the name of
    ///                 the file to be associated with the constructed object.
    /// \param mode     The mode in which to open the file. May be a combination of the
    ///                 \c std::ios_base::openmode modes. The std::ios_base::binary mode will
    ///                 always be added.
    explicit BinOFStream(const std::filesystem::path &filename, std::ios_base::openmode mode = std::ios_base::out)
        : BinOFStream()
    {
        open(filename, mode);
    }

#if defined(_WIN32)
    /// \brief  Construct a BinOFStream object with a filename and an open mode specifier.
    ///         This overload is available only if the \c _WIN32 macro is defined.
    /// \param filename Pointer to a nul-terminated wide-character C-style string containing the name of
    ///                 the file to be associated with the constructed object.
    /// \param mode     The mode in which to open the file. May be a combination of the
    ///                 \c std::ios_base::openmode modes. The std::ios_base::binary mode will
    ///                 always be added.
    explicit BinOFStream(const wchar_t *filename, std::ios_base::openmode mode = std::ios_base::out)
        : BinOFStream()
    {
        open(filename, mode);
    }
#endif

    /// \brief  Destroy a BinOFStream object. Any associated file is closed.
    ~BinOFStream()
    {
        close();
    }

    /// \brief  Swap the contents of this BinOFStream object with another.
    /// \param other    Another BinOFStream object whose contents is to be swapped.
    void swap(BinOFStream &other)
    {
        if (this != std::addressof(other))
        {
            BinOStream::swap(other);
            _impl.swap(other._impl);
        }
    }

    //
    // File operations
    //

    /// \brief  Open the specified file using the specified mode.
    /// \param filename     A nul-terminated C-style string containing the name of the file to open.
    /// \param mode         The mode in which to open the file. May be a combination of the
    ///                     \c std::ios_base::openmode modes. The std::ios_base::binary mode will
    ///                     always be added.
    void open(const char *filename, std::ios_base::openmode mode = std::ios_base::out)
    {
        if (_impl.open(filename, mode | std::ios_base::binary))
            clear();
        else
            setstate(std::ios_base::failbit);
    }

    /// \brief  Open the specified file using the specified mode.
    /// \param filename     A std::string containing the name of the file to open.
    /// \param mode         The mode in which to open the file. May be a combination of the
    ///                     \c std::ios_base::openmode modes. The std::ios_base::binary mode will
    ///                     always be added.
    void open(const std::string &filename, std::ios_base::openmode mode = std::ios_base::out)
    {
        open(filename.c_str(), mode);
    }

    /// \brief  Open the specified file using the specified mode.
    /// \param filename     std::filesystem::path object containing the name of the file to open.
    /// \param mode         The mode in which to open the file. May be a combination of the
    ///                     \c std::ios_base::openmode modes. The std::ios_base::binary mode will
    ///                     always be added.
    void open(const std::filesystem::path &filename, std::ios_base::openmode mode = std::ios_base::out)
    {
        open(filename.c_str(), mode);
    }

#if defined(_WIN32)
    /// \brief  Open the specified file using the specified mode.
    ///         This overload is available only if the \c _WIN32 macro is defined.
    /// \param filename     A nul-terminated wide-character C-style string containing the name of the file to open.
    /// \param mode         The mode in which to open the file. May be a combination of the
    ///                     \c std::ios_base::openmode modes. The std::ios_base::binary mode will
    ///                     always be added.
    void open(const wchar_t *filename, std::ios_base::openmode mode = std::ios_base::out)
    {
        if (_impl.open(filename, mode | std::ios_base::binary))
            clear();
        else
            setstate(std::ios_base::failbit);
    }
#endif  // _Win32

    /// \brief  Determine of an open file is associated with this BinOFStream object.
    /// \return \c true if an open file is associated with this object, \c false otherwise.
    bool is_open() const noexcept
    {
        return _impl.is_open();
    }

    /// \brief  Close an open file associated with this BinOFStream object.
    void close()
    {
        if (is_open())
        {
            _impl.close();
        }
    }

protected:
    /// \brief  Set the fail and/or eof bits from the current state of the
    ///         underlying FILE object
    void set_fail_or_eof()
    {
        if (_impl.eof())
            setstate(std::ios_base::eofbit);
        if (_impl.error())
            setstate(std::ios_base::failbit);
    }

private:
    BinFileStreamImpl   _impl;
};


/// \brief  The BinFStream class provides a binary input/output stream backed by a file.
class BinFStream : public BinIOStream
{
public:
    using byte_type     = uint8_t;                  ///< Type used to represent a byte
    using traits_type   = std::char_traits<char>;   ///< Type used for character traits
    using int_type      = traits_type::int_type;    ///< Type used for integers
    using pos_type      = std::streampos;           ///< Type used for positioning
    using off_type      = std::streamoff;           ///< Type used for offsets


public:
    /// \brief  Default construct a BinFStream object. No file is associated with the constructed object.
    BinFStream()
        : BinIOStream(std::addressof(_impl))
    {}

    /// \brief  Construct a BinOFStream object with a filename and an open mode specifier.
    /// \param filename Pointer to a nul-terminated C-style string containing the name of
    ///                 the file to be associated with the constructed object.
    /// \param mode     The mode in which to open the file. May be a combination of the
    ///                 \c std::ios_base::openmode modes. The std::ios_base::binary mode will
    ///                 always be added.
    explicit BinFStream(const char *filename, std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out)
        : BinIOStream(std::addressof(_impl))
    {
        open(filename, mode);
    }

    /// \brief  Construct a BinFStream object with a filename and an open mode specifier.
    /// \param filename std::string containing the name of
    ///                 the file to be associated with the constructed object.
    /// \param mode     The mode in which to open the file. May be a combination of the
    ///                 \c std::ios_base::openmode modes. The std::ios_base::binary mode will
    ///                 always be added.
#if 0
    explicit BinFStream(const std::string &filename, std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out)
        : BinIOStream(std::addressof(_impl))
    {
        open(filename, mode);
    }
#else
    explicit BinFStream(const std::string &filename, std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out)
        : BinFStream(filename.c_str(), mode)
    {}
#endif

    /// \brief  Construct a BinFStream object with a filename and an open mode specifier.
    /// \param filename std::filesystem::path object containing the name of
    ///                 the file to be associated with the constructed object.
    /// \param mode     The mode in which to open the file. May be a combination of the
    ///                 \c std::ios_base::openmode modes. The std::ios_base::binary mode will
    ///                 always be added.
#if 0
    explicit BinFStream(const std::filesystem::path &filename, std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out)
        : BinIOStream(std::addressof(_impl))
    {
        open(filename, mode);
        //TODO: Implement me properly!!!
    }
#else
    explicit BinFStream(const std::filesystem::path &filename, std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out)
        : BinFStream(filename.c_str(), mode)
    {}
#endif

#if defined(_WIN32)
    /// \brief  Construct a BinFStream object with a filename and an open mode specifier.
    ///         This overload is available only if the \c _WIN32 macro is defined.
    /// \param filename Pointer to a nul-terminated wide-character C-style string containing the name of
    ///                 the file to be associated with the constructed object.
    /// \param mode     The mode in which to open the file. May be a combination of the
    ///                 \c std::ios_base::openmode modes. The std::ios_base::binary mode will
    ///                 always be added.
    explicit BinFStream(const wchar_t *filename, std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out)
        : BinIOStream(std::addressof(_impl))
    {
        open(filename, mode);
    }
#endif

    /// \brief  The copy constructor is deleted. The BinFStream is not copy constructable.
    BinFStream(const BinFStream &) = delete;
    /// \brief  The copy assignment operator is deleted. The BinFStream is not copy assignable.
    BinFStream &operator=(BinFStream &) = delete;

    /// \brief  Construct a BinFStream by moving data from another BinFStream object.
    /// \param other    Another BinFStream to move data from.
    BinFStream(BinFStream &&other) noexcept
        : BinIOStream(std::move(other))
        , _impl{std::move(other._impl)}
    {
        BinIOStream::rdimpl(&_impl);
    }

    /// \brief  Assign to a BinFStream object by moving data from another BinFStream object
    BinFStream &operator=(BinFStream &&rhs) noexcept
    {
        if (this != std::addressof(rhs))
        {
            BinIOStream::operator=(std::move(rhs));
            _impl = std::move(rhs._impl);
            BinIOStream::rdimpl(&_impl);
        }

        return *this;
    }

    /// \brief  Destroy a BinOFStream object. Any associated file is closed.
    ~BinFStream()
    {
        close();
    }

    /// \brief  Swap the contents of this BinFStream object with another.
    /// \param other    Another BinFStream object whose contents is to be swapped.
    void swap(BinFStream &other)
    {
        if (this != std::addressof(other))
        {
            BinIStream::swap(other);
            //BinOStream::swap(other);
            _impl.swap(other._impl);
        }
    }


    //
    // File operations
    //

    /// \brief  Open the specified file using the specified mode.
    /// \param filename     A nul-terminated C-style string containing the name of the file to open.
    /// \param mode         The mode in which to open the file. May be a combination of the
    ///                     \c std::ios_base::openmode modes. The std::ios_base::binary mode will
    ///                     always be added.
    void open(const char *filename, std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out)
    {
        if (_impl.open(filename, mode | std::ios_base::binary))
            clear();
        else
            setstate(std::ios_base::failbit);
    }

    /// \brief  Open the specified file using the specified mode.
    /// \param filename     A std::string containing the name of the file to open.
    /// \param mode         The mode in which to open the file. May be a combination of the
    ///                     \c std::ios_base::openmode modes. The std::ios_base::binary mode will
    ///                     always be added.
    void open(const std::string &filename, std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out)
    {
        open(filename.c_str(), mode);
    }

    /// \brief  Open the specified file using the specified mode.
    /// \param filename     std::filesystem::path object containing the name of the file to open.
    /// \param mode         The mode in which to open the file. May be a combination of the
    ///                     \c std::ios_base::openmode modes. The std::ios_base::binary mode will
    ///                     always be added.
    void open(const std::filesystem::path &filename, std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out)
    {
        open(filename.c_str(), mode);
    }

#if defined(_WIN32)
    /// \brief  Open the specified file using the specified mode.
    ///         This overload is available only if the \c _WIN32 macro is defined.
    /// \param filename     A nul-terminated wide-character C-style string containing the name of the file to open.
    /// \param mode         The mode in which to open the file. May be a combination of the
    ///                     \c std::ios_base::openmode modes. The std::ios_base::binary mode will
    ///                     always be added.
    bool open(const wchar_t *filename, std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out)
    {
        if (_impl.open(filename, mode | std::ios_base::binary))
            clear();
        else
            setstate(std::ios_base::failbit);
    }
#endif  // _Win32

    /// \brief  Determine of an open file is associated with this BinOFStream object.
    /// \return \c true if an open file is associated with this object, \c false otherwise.
    bool is_open() const noexcept
    {
        return _impl.is_open();
    }

    /// \brief  Close an open file associated with this BinOFStream object.
    void close()
    {
        if (is_open())
        {
            _impl.close();
        }
    }

protected:
    /// \brief  Set the fail and/or eof bits from the current state of the
    ///         underlying FILE object
    void set_fail_or_eof()
    {
        if (_impl.eof())
            BinIos::setstate(std::ios_base::eofbit | std::ios_base::failbit);
        if (_impl.error())
            BinIos::setstate(std::ios_base::failbit);
    }

private:
    BinFileStreamImpl   _impl;
};


}   // namespace brace

#endif  // _BRACE_LIB_BINFSTREAM_INC
