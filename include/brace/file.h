//////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2023 Jeffrey K. Bienstadt
//
// This file is part of the brace C++ library.
//
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.
//////////////////////////////////////////////////////////////////////

/// \file file.h
///
/// \author Jeff Bienstadt
#ifndef _BRACE_LIB_FILE_INC
#define _BRACE_LIB_FILE_INC

#include <cstdio>
#include <filesystem>
#include <string>
#include <unordered_map>

namespace brace
{

/// \brief      Create a mode string for the std::fopen function from std::ios_base::openmode bits.
/// \param mode A combination of std::ios_base::openmode bits specifying the mode with which to open a file.
/// \return     On success, a string containing open modes suitable for the std::fopen function, otherwise an empty string.
inline const char *make_fopen_mode(std::ios_base::openmode mode)
{
    static const std::unordered_map<std::ios_base::openmode, const char *> open_modes = {
        {std::ios_base::in, "r"},
        {std::ios_base::in | std::ios_base::binary, "rb"},
        {std::ios_base::in | std::ios_base::out, "r+"},
        {std::ios_base::in | std::ios_base::out | std::ios_base::binary, "r+b"},
        {std::ios_base::out, "w"},
        {std::ios_base::out | std::ios_base::trunc, "w"},
        {std::ios_base::out | std::ios::binary, "wb"},
        {std::ios_base::out | std::ios_base::trunc | std::ios::binary, "wb"},
        {std::ios_base::in | std::ios_base::out | std::ios::binary, "w+"},
        {std::ios_base::in | std::ios_base::out | std::ios_base::trunc | std::ios::binary, "w+b"},
        {std::ios_base::out | std::ios_base::app, "a"},
        {std::ios_base::app, "a"},
        {std::ios_base::binary | std::ios_base::out | std::ios_base::app, "ab"},
        {std::ios_base::binary | std::ios_base::app, "ab"},
        {std::ios_base::in | std::ios_base::out | std::ios_base::app, "a+"},
        {std::ios_base::in | std::ios_base::app, "a+"},
        {std::ios_base::binary | std::ios_base::in | std::ios_base::out | std::ios_base::app, "a+b"},
        {std::ios_base::binary | std::ios_base::in | std::ios_base::app, "a+b"},
    };

    if (auto it = open_modes.find(mode & ~std::ios_base::ate); it != open_modes.end())
        return it->second;

    return "";
}

#if defined(_WIN32)
/// \brief      Create a mode string for the std::fopen function from std::ios_base::openmode bits.
/// \param mode A combination of std::ios_base::openmode bits specifying the mode with which to open a file.
/// \return     On success, a string containing open modes suitable for the std::fopen function, otherwise an empty string.
/// \details    This is the wide character string version of this function.
inline const wchar_t *make_wfopen_mode(std::ios_base::openmode mode)
{
    static const std::unordered_map<std::ios_base::openmode, const wchar_t *> open_modes = {
        {std::ios_base::in, L"r"},
        {std::ios_base::in | std::ios_base::binary, L"rb"},
        {std::ios_base::in | std::ios_base::out, L"r+"},
        {std::ios_base::in | std::ios_base::out | std::ios_base::binary, L"r+b"},
        {std::ios_base::out, L"w"},
        {std::ios_base::out | std::ios_base::trunc, L"w"},
        {std::ios_base::out | std::ios::binary, L"wb"},
        {std::ios_base::out | std::ios_base::trunc | std::ios::binary, L"wb"},
        {std::ios_base::in | std::ios_base::out | std::ios::binary, L"w+"},
        {std::ios_base::in | std::ios_base::out | std::ios_base::trunc | std::ios::binary, L"w+b"},
        {std::ios_base::out | std::ios_base::app, L"a"},
        {std::ios_base::app, L"a"},
        {std::ios_base::binary | std::ios_base::out | std::ios_base::app, L"ab"},
        {std::ios_base::binary | std::ios_base::app, L"ab"},
        {std::ios_base::in | std::ios_base::out | std::ios_base::app, L"a+"},
        {std::ios_base::in | std::ios_base::app, L"a+"},
        {std::ios_base::binary | std::ios_base::in | std::ios_base::out | std::ios_base::app, L"a+b"},
        {std::ios_base::binary | std::ios_base::in | std::ios_base::app, L"a+b"},
    };

    if (auto it = open_modes.find(mode & ~std::ios_base::ate); it != open_modes.end())
        return it->second;

    return L"";
}
#endif  // _WIN32

/// \brief Abstraction around std::FILE. Provides functions for most std::FILE related operations.
class File
{
public:
    /// \brief Default construct a File object. No file is associated with the object.
    File() noexcept
    {}

    /// \brief  Construct a File object and attempt to open the specified file using the specified mode.
    /// \param filename A pointer to a C-style string containing the name of the file to be opened.
    /// \param mode     Combination of std::ios_base::openmode bits specifying the mode to be used to open the file.
    File(const char *filename, std::ios_base::openmode mode)
    {
        open(filename, mode);
    }
    /// \brief  Construct a File object and attempt to open the specified file using the specified mode.
    /// \param filename A std::string containing the name of the file to be opened.
    /// \param mode     Combination of std::ios_base::openmode bits specifying the mode to be used to open the file.
    File(const std::string &filename, std::ios_base::openmode mode)
    {
        open(filename, mode);
    }
    /// \brief  Construct a File object and attempt to open the specified file using the specified mode.
    /// \param filename A std::filesystem::path object containing the name of the file to be opened.
    /// \param mode     Combination of std::ios_base::openmode bits specifying the mode to be used to open the file.
    File(const std::filesystem::path &filename, std::ios_base::openmode mode)
    {
        open(filename.c_str(), mode);
    }
#if defined(_WIN32)
    /// \brief  Construct a File object and attempt to open the specified file using the specified mode.
    /// \param filename A pointer to a wide character C-style string containing the name of the file to be opened.
    /// \param mode     Combination of std::ios_base::openmode bits specifying the mode to be used to open the file.
    File(const wchar_t *filename, std::ios_base::openmode mode)
    {
        open(filename, mode);
    }
#endif

    /// \brief  Construct a File object from a pointer to a std::FILE structure.
    /// \param f    A pointer to a std::FILE structure.
    /// \details    The f parameter should either be associated with a file or
    ///             should be \c nullptr.
    ///             The File object assumes ownership of the associated file,
    ///             and will close the file upon object destruction.
    ///             Passing nullptr is essentially the same as invoking the
    ///             default constructor.
    explicit File(std::FILE *f) noexcept
        : _file{f}
    {}

    /// \brief  The copy constructor is deleted. File objects are not copyable.
    File(const File &) = delete;
    /// \brief  The copy assignment operator is deleted. File objects are not copy assignable.
    File & operator=(const File &) = delete;

    /// \brief  Construct a File object by moving the contents of another File object.
    /// \param other    Another File object to be moved from
    File(File &&other) noexcept
        : _file{other._file}
    {
        other._file = nullptr;
    }
    /// \brief  Move assign the File object rhs to *this.
    /// \param rhs  The File object to be moved.
    /// \return *this.
    File &operator=(File &&rhs) noexcept
    {
        if (this != std::addressof(rhs))
        {
            _file = rhs._file;
            rhs._file = nullptr;
        }

        return *this;
    }

    /// \brief Destroys the File object, closing any associated file.
    ~File()
    {
        close();
    }



    /// \brief  Open a file with the specified name, using the specified open mode.
    /// \param filename A pointer to a C-style string containing the name of the file to be opened.
    /// \param mode     A pointer to a C-style string specifying the mode to be used to open the file.
    /// \return \c true on success, \c false otherwise.
    bool open(const char *filename, const char *mode)
    {
        if (is_open())
            return false;

#if defined(_WIN32)
        std::FILE  *f{nullptr};
        auto        err = fopen_s(&f, filename, mode);

        if (err != 0)
            return false;
#else
        std::FILE  *f{std::fopen(filename, mode)};
#endif

        if (f)
        {
            _file = f;
            return true;
        }

        return false;
    }
    /// \brief  Open a file with the specified name, using the specified open mode.
    /// \param filename A std::string containing the name of the file to be opened.
    /// \param mode     A pointer to a C-style string specifying the mode to be used to open the file.
    /// \return \c true on success, \c false otherwise.
    bool open(const std::string &filename, const char *mode)
    {
        return open(filename.c_str(), mode);
    }
    /// \brief  Open a file with the specified name, using the specified open mode.
    /// \param filename A std::filesystem::path object containing the name of the file to be opened.
    /// \param mode     A pointer to a C-style string specifying the mode to be used to open the file.
    /// \return \c true on success, \c false otherwise.
    bool open(const std::filesystem::path &filename, const char *mode)
    {
        return open(filename.c_str(), mode);
    }
#if defined(_WIN32)
    /// \brief  Open a file with the specified name, using the specified open mode.
    /// \param filename A pointer to a wide character C-style string containing the name of the file to be opened.
    /// \param mode     A pointer to a wide character C-style string specifying the mode to be used to open the file.
    /// \return \c true on success, \c false otherwise.
    bool open(const wchar_t *filename, const wchar_t *mode)
    {
        if (is_open())
            return false;

        std::FILE  *f{nullptr};
        auto        err = _wfopen_s(&f, filename, mode);

        if (err != 0)
            return false;

        if (f)
        {
            _file = f;
            return true;
        }

        return false;
    }
#endif  // _Win32

    /// \brief  Open a file with the specified name, using the specified open mode.
    /// \param filename A pointer to a C-style string containing the name of the file to be opened.
    /// \param mode     A combination of std::ios_base::openmode bits specifying the mode to be used to open the file.
    /// \return \c true on success, \c false otherwise.
    bool open(const char *filename, std::ios_base::openmode mode)
    {
        return open(filename, make_fopen_mode(mode));
    }
    /// \brief  Open a file with the specified name, using the specified open mode.
    /// \param filename A std::string containing the name of the file to be opened.
    /// \param mode     A combination of std::ios_base::openmode bits specifying the mode to be used to open the file.
    /// \return \c true on success, \c false otherwise.
    bool open(const std::string &filename, std::ios_base::openmode mode)
    {
        return open(filename.c_str(), mode);
    }
    /// \brief  Open a file with the specified name, using the specified open mode.
    /// \param filename A std::filesystem::path object containing the name of the file to be opened.
    /// \param mode     A Combination of std::ios_base::openmode bits specifying the mode to be used to open the file.
    /// \return \c true on success, \c false otherwise.
    bool open(const std::filesystem::path &filename, std::ios_base::openmode mode)
    {
        return open(filename.c_str(), mode);
    }
#if defined(_WIN32)
    /// \brief  Open a file with the specified name, using the specified open mode.
    /// \param filename A pointer to a wide character C-style string containing the name of the file to be opened.
    /// \param mode     A combination of std::ios_base::openmode bits specifying the mode to be used to open the file.
    /// \return \c true on success, \c false otherwise.
    bool open(const wchar_t *filename, std::ios_base::openmode mode)
    {
        return open(filename, make_wfopen_mode(mode));
    }
#endif  // _Win32

    /// \brief Determine if the File object refers to an open file.
    /// \return \c true if the File object has an open file, \c false otherwise.
    bool is_open() const noexcept
    {
        return _file != nullptr;
    }

    /// \brief Close any open file.
    void close()
    {
        if (is_open())
        {
            std::fclose(_file);
            _file = nullptr;
        }
    }

    /// \brief Flush the FILE buffer out to the file's device.
    /// \return Zero on success, EOF otherwise.
    int flush()
    {
        return std::fflush(_file);
    }

    /// \brief Sets the internal buffer to use for I/O operations performed on the C stream.
    /// \param buffer   A pointer to the buffer for the stream to use.
    void setbuf(char *buffer)
    {
        std::setbuf(_file, buffer);
    }

    /// \brief  Change the buffering mode of the C stream, optionally setting a new buffer.
    /// \param buffer   A pointer to a buffer for the stream to use or null pointer to change size and mode only.
    /// \param mode     The buffering mode to use.
    /// \param size     The size of the buffer.
    /// \return \c true on success, \c false on failure.
    bool setvbuf(char *buffer, int mode, std::size_t size)
    {
        return std::setvbuf(_file, buffer, mode, size) == 0;
    }

    /// \brief  Read up to count objects into the array buffer from the input stream
    /// \param buffer   A pointer to the first object in the array to be read.
    /// \param size     The size of each object in bytes.
    /// \param count    The number of object to be read.
    /// \return The number of object successfully read.
    size_t read(void *buffer, std::size_t size, std::size_t count)
    {
        return std::fread(buffer, size, count, _file);
    }

    /// \brief  Write up to count binary objects from the array buffer to the output stream.
    /// \param buffer   A pointer to the first object in the array to be written.
    /// \param size     The size of each object in bytes.
    /// \param count    The number of objects to be written.
    /// \return The number of objects successfully written
    size_t write(const void *buffer, std::size_t size, std::size_t count)
    {
        return std::fwrite(buffer, size, count, _file);
    }

    /// \brief  Read the next character from the input stream.
    /// \return The read character on success, EOF on failure.
    int getc()
    {
        return std::fgetc(_file);
    }

    /// \brief  Read at most count - 1 characters from the input stream and store them in the
    ///         character array pointed to by str.
    /// \param str      A pointer to an element in a character array
    /// \param count    Maximum number of characters to read.
    /// \return str on success, null pointer on failure.
    char *gets(char *str, int count)
    {
        return std::fgets(str, count, _file);
    }

    /// \brief  Write a character to the output stream.
    /// \param ch   The character to write.
    /// \return The written character on success, EOF on failure.
    int putc(int ch)
    {
        return std::fputc(ch, _file);
    }

    /// \brief  Write each character from the nul-terminated string pointed to by str.
    ///         The terminating nul character is not written.
    /// \param str  nul-terminated character string to be written.
    /// \return A non-negative value on success, EOF on failure.
    int puts(const char *str)
    {
        return std::fputs(str, _file);
    }

    /// \brief  Push a character into the input buffer associated with the stream.
    /// \param ch   The character to be pushed into the input buffer. If EOF is passed,
    ///             the operation fails and the stream is not affected.
    /// \return ch on success, EOF on failure.
    int ungetc(int ch)
    {
        return std::ungetc(ch, _file);
    }

    /// \brief  Return the current value of the file position indicator for the file stream.
    /// \return The file position indicator on success, or std::streampos(-1) on failure.
    std::streampos tell()
    {
#if defined(_WIN32)
        return std::streampos(std::streamoff(_ftelli64(_file)));
#elif defined(__APPLE__) || defined(__linux__)
        return std::streampos(ftello(_file));
#else
        return std::streampos(std::streamoff(std::ftell(_file)));
#endif
    }

    /// \brief  Set the file position indicator for the stream.
    /// \param off  The number of characters to shift the position relative to dir.
    /// \param dir  The position to which off is added. Can be one of
    ///             std::ios_base::beg, std::ios_base::cur, std::ios_base::end.
    /// \return Zero on success, non-zero otherwise.
    int seek(std::streamoff off, std::ios_base::seekdir dir)
    {
        int origin{dir == std::ios_base::beg ? SEEK_SET : dir == std::ios_base::cur ? SEEK_CUR : SEEK_END};
#if defined(_WIN32)
        return _fseeki64(_file, off, origin);
#elif defined(__APPLE__) || defined(__linux__)
        return fseeko(_file, off, origin);
#else
        return std::fseek(_file, static_cast<long>(off), origin);
#endif
    }

    /// \brief  Obtain the file position indicator and current parse state (if any) for the file stream.
    /// \param pos  A pointer to an \c fpos_t object to store the file position indicator into.
    /// \return Zero on success, nonzero on failure.
    int getpos(std::fpos_t *pos)
    {
        return std::fgetpos(_file, pos);
    }

    /// \brief  Set the file position indicator and the multibyte parsing state (if any) for the C file stream.
    /// \param pos  A pointer to a \c fpos_t object obtained from fgetpos called on a stream associated with the same file
    /// \return Zero on success, nonzero on failure.
    int setpos(const std::fpos_t *pos)
    {
        return std::fsetpos(_file, pos);
    }

    /// \brief  Move the file position indicator to the beginning of the file stream.
    void rewind()
    {
        return std::rewind(_file);
    }

    /// \brief  Reset the error flags and the EOF indicator for the file stream.
    void clearerr()
    {
        std::clearerr(_file);
    }

    /// \brief  Check the file stream for errors.
    /// \return \c true if the file stream has errors, \c false otherwise.
    bool error()
    {
        return std::ferror(_file) != 0;
    }

    /// \brief  Check if the end of the file stream has been reached.
    /// \return \c true if the end of the file has been reached, \c false otherwise.
    bool eof()
    {
        return std::feof(_file) != 0;
    }

private:
    std::FILE  *_file{nullptr};
};

}   // namespace brace

#endif  // _BRACE_LIB_FILE_INC
