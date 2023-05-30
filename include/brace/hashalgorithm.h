
#ifndef BRACE_LIB_HASHALGORITHM_INC
#define BRACE_LIB_HASHALGORITHM_INC

//////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2023 Jeffrey K. Bienstadt
//
// This file is part of the brace C++ library.
//
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.
//////////////////////////////////////////////////////////////////////

/// \file hashalgorithm.h
///
/// \author Jeff Bienstadt

#include <algorithm>
#include <climits>
#include <cstdint>
#include <cstddef>
#include <iomanip>
#include <istream>
#include <sstream>
#include <string>
#include <vector>

//#include <memory>

#include "binistream.h"

namespace brace {

///
/// \brief  Abstract base class for hash algorithms.
///
/// Derived classes must implement the \c do_hash, \c finalize_hash, and \c reset functions.
///
class HashAlgorithm
{
protected:
    /// \brief  The SHA Ch function.
    /// \tparam T   The type of the input and output data
    /// \param x    The first of three words
    /// \param y    The second of three words
    /// \param z    The third of three words
    /// \return A word of type T.
    template <typename T>
    static T SHA_Ch(T x, T y, T z) noexcept
    {
        //return ((x & (y ^ z)) ^ z);
        return (x & y) ^ (~x & z);
    }

    /// \brief  The SHA Maj function.
    /// \tparam T   The type of the input and output data
    /// \param x    The first of three words
    /// \param y    The second of three words
    /// \param z    The third of three words
    /// \return A word of type T.
    template <typename T>
    static T SHA_Maj(T x, T y, T z) noexcept
    {
        //return ((x & (y | z)) | (y & z));
        return (x & y) ^ (x & z) ^ (y & z);
    }

    /// \brief  The SHA Parity function.
    /// \tparam T   The type of the input and output data
    /// \param x    The first of three words
    /// \param y    The second of three words
    /// \param z    The third of three words
    /// \return A word of type T.
    template <typename T>
    static T SHA_Parity(T x, T y, T z) noexcept
    {
        return (x ^ y ^ z);
    }

    /// \brief  A large integer, with overflow detection.
    /// \details    This class is specific to the hash operations and is _not_ intended
    ///             to be a general-purpose large-integer class.
    template <typename T>
    class HashLength
    {
    public:
        /// \brief  Default construct a HashLength object.
        HashLength() noexcept
        : _high{(T)0},
            _low{(T)0}
        {}

        /// \brief  Increment a HashLength object.
        /// \return *this
        /// \exception  std::range_error on overflow.
        HashLength &operator++()
        {
            _low += CHAR_BIT;
            if (_low == 0)
                if (++_high == 0) // overflow!
                    throw std::range_error("Hash maximum length exceeded.");

            return *this;
        }

        /// @brief  Reset a HashLength object to zero (0)
        void reset() noexcept
        {
            _high = _low = (T)0;
        }

        /// @brief  Return the high-order part of a HashLength object.
        /// @return The high-order part of the object.
        T high() const noexcept
        {
            return _high;
        }

        /// @brief Return the low-order part of a HashLength object.
        /// @return The low-order part of the object.
        T low() const noexcept
        {
            return _low;
        }

    private:
        T   _high;
        T   _low;
    };

    /// @brief  A HashLength using 64 bits.
    using HashLength64_t  = HashLength<uint32_t>;   // a 64-bit hash length
    /// @brief  A HashLength using 128 bits.
    using HashLength128_t = HashLength<uint64_t>;   // a 128-bit hash length

public:
    ///
    /// \brief Create a string representation of a hash.
    ///
    /// The string consists of a sequence of n*2 hex digits,
    /// where n is the number of bytes in the hash.
    ///
    /// \param hash A hash as produced by one of the \c compute_hash functions.
    ///
    static std::string hash_to_string(const std::vector<uint8_t> &hash)
    {
        std::stringstream   str;

        str << std::hex << std::uppercase << std::setfill('0');
        for (const auto &b : hash)
            str << std::setw(2) << (unsigned int)b;

        return str.str();
    }

    HashAlgorithm() = delete;

    /// \brief  Compute a hash from raw bytes of a specified length
    ///
    /// \param  buffer  A pointer to the raw data.
    /// \param  length  Length of the data pointed to by \p buffer.
    /// \return A vector of \c uint8_t bytes containing the hash value.
    ///         The length of the returned vector is determined by the
    ///         specific hashing algorithm.
    std::vector<uint8_t> compute_hash(const uint8_t *buffer, size_t length)
    {
        do_hash(buffer, length);

        return finalize_hash();
    }

    /// \brief  Compute a hash from a vector of bytes
    ///
    /// \param  buffer  A vector of \c uint8_t values.
    /// \return A vector of \c uint8_t bytes containing the hash value.
    ///         The length of the returned vector is determined by the
    ///         specific hashing algorithm.
    std::vector<uint8_t> compute_hash(const std::vector<uint8_t> &buffer)
    {
        return compute_hash(buffer.data(), buffer.size());
    }

    /// \brief  Compute a hash from a std::string
    ///
    /// \param  s A \c std::string containing the data to be hashed.
    /// \return A vector of \c uint8_t bytes containing the hash value.
    ///         The length of the returned vector is determined by the
    ///         specific hashing algorithm.
    std::vector<uint8_t> compute_hash(const std::string &s)
    {
        return compute_hash(reinterpret_cast<const uint8_t *>(s.c_str()), s.size());
    }

    /// \brief  Compute a hash from bytes read from a stream.
    ///
    /// \param  stream  An input stream from which to read bytes.
    ///                 The stream is read from the current position until
    ///                 end of file is reached. The stream is assumed
    ///                 to have been opened in binary mode.
    /// \return A vector of \c uint8_t bytes containing the hash value.
    ///         The length of the returned vector is determined by the
    ///         specific hashing algorithm.
    std::vector<uint8_t> compute_hash(std::istream &stream)
    {
        constexpr size_t    buf_size = 4096;

        uint8_t buffer[buf_size];

        do
        {
            stream.read((char *)buffer, buf_size);
            do_hash(buffer, static_cast<size_t>(stream.gcount()));
        } while (stream);

        return finalize_hash();
    }

    /// \brief  Compute a hash from bytes read from a stream.
    ///
    /// \param  stream  An input stream from which to read bytes.
    ///                 The stream is read from the current position until
    ///                 \p length bytes are read or end of file is reached.
    ///                 The stream is assumed to have been opened in binary mode.
    /// \param  length  Maximum number of bytes to process.
    ///                 The specified number bytes may not all be processed
    ///                 if end of file is reached before all specified bytes
    ///                 are read from the stream.
    /// \return A vector of \c uint8_t bytes containing the hash value.
    ///         The length of the returned vector is determined by the
    ///         specific hashing algorithm.
    std::vector<uint8_t> compute_hash(std::istream &stream, size_t length)
    {
        constexpr size_t    buf_size = 4096;

        uint8_t buffer[buf_size];

        while (length > 0)
        {
            if (!stream)
                break;

            size_t  count = std::min(length, buf_size);
            stream.read((char *)buffer, count);
            do_hash(buffer, static_cast<size_t>(stream.gcount()));
            length -= static_cast<size_t>(stream.gcount());
        }

        return finalize_hash();
    }

    /// \brief  Compute a hash from bytes read from a binary stream.
    ///
    /// \param  stream  An input stream from which to read bytes.
    ///                 The stream is read from the current position until
    ///                 end of file is reached.
    /// \return A vector of \c uint8_t bytes containing the hash value.
    ///         The length of the returned vector is determined by the
    ///         specific hashing algorithm.
    std::vector<uint8_t> compute_hash(brace::BinIStream &stream)
    {
        constexpr size_t    buf_size = 4096;

        brace::BinIStream::byte_type    buffer[buf_size];

        do
        {
            stream.read(buffer, buf_size);
            do_hash(buffer, static_cast<size_t>(stream.gcount()));
        } while (stream);

        return finalize_hash();
    }

    /// \brief  Compute a hash from bytes read from a binary stream.
    ///
    /// \param  stream  An input stream from which to read bytes.
    ///                 The stream is read from the current position until
    ///                 \p length bytes are read or end of file is reached.
    /// \param  length  Maximum number of bytes to process.
    ///                 The specified number bytes may not all be processed
    ///                 if end of file is reached before all specified bytes
    ///                 are read from the stream.
    /// \return A vector of \c uint8_t bytes containing the hash value.
    ///         The length of the returned vector is determined by the
    ///         specific hashing algorithm.
    std::vector<uint8_t> compute_hash(brace::BinIStream &stream, size_t length)
    {
        constexpr size_t    buf_size = 4096;

        brace::BinIStream::byte_type    buffer[buf_size];

        while (length > 0)
        {
            if (!stream)
                break;

            size_t  count = std::min(length, buf_size);
            stream.read(buffer, count);
            do_hash(buffer, static_cast<size_t>(stream.gcount()));
            length -= static_cast<size_t>(stream.gcount());
        }

        return finalize_hash();
    }

    /// \brief  Create a string representation of a hash computed from
    ///         raw bytes of a specified length
    ///
    /// \param  buffer  A pointer to the raw data.
    /// \param  length  Length of the data pointed to by \p buffer.
    /// \return A string representation of the computed hash.
    std::string compute_hash_string(const uint8_t *buffer, size_t length)
    {
        return hash_to_string(compute_hash(buffer, length));
    }

    /// \brief  Create a string representation of a hash computed from
    ///         a vector of bytes
    ///
    /// \param  buffer  A vector of \c uint8_t values.
    /// \return A string representation of the computed hash.
    std::string compute_hash_string(const std::vector<uint8_t> &buffer)
    {
        return hash_to_string(compute_hash(buffer));
    }

    /// \brief      Create a string representation of a hash computed from
    ///             a std::string.
    /// \param s    A std::string to be used as the input data for the hash.
    /// \return     A string representation of the computed hash.
    std::string compute_hash_string(const std::string &s)
    {
        return hash_to_string(compute_hash(s));
    }

    /// \brief  Create a string representation of a hash computed from
    ///         bytes read from a stream.
    ///
    /// \param  stream  An input stream from which to read bytes.
    ///                 The stream is read from the current position until
    ///                 end of file is reached.
    /// \return A string representation of the computed hash.
    std::string compute_hash_string(std::istream &stream)
    {
        return hash_to_string(compute_hash(stream));
    }

    /// \brief  Create a string representation of a hash computed from
    ///         bytes read from a stream.
    ///
    /// \param  stream  An input stream from which to read bytes.
    ///                 The stream is read from the current position until
    ///                 end of file is reached.
    /// \param  length  Maximum number of bytes to process.
    ///                 The specified number bytes may not all be processed
    ///                 if end of file is reached before all specified bytes
    ///                 are read from the stream.
    /// \return A string representation of the computed hash.
    std::string compute_hash_string(std::istream &stream, size_t length)
    {
        return hash_to_string(compute_hash(stream, length));
    }

    /// \brief  Create a string representation of a hash computed from
    ///         bytes read from a binary stream.
    ///
    /// \param  stream  An input stream from which to read bytes.
    ///                 The stream is read from the current position until
    ///                 end of file is reached.
    /// \return A string representation of the computed hash.
    std::string compute_hash_string(brace::BinIStream &stream)
    {
        return hash_to_string(compute_hash(stream));
    }

    /// \brief  Create a string representation of a hash computed from
    ///         bytes read from a binary stream.
    ///
    /// \param  stream  An input stream from which to read bytes.
    ///                 The stream is read from the current position until
    ///                 end of file is reached.
    /// \param  length  Maximum number of bytes to process.
    ///                 The specified number bytes may not all be processed
    ///                 if end of file is reached before all specified bytes
    ///                 are read from the stream.
    /// \return A string representation of the computed hash.
    std::string compute_hash_string(brace::BinIStream &stream, size_t length)
    {
        return hash_to_string(compute_hash(stream, length));
    }

    /// \brief  Get the size of the produced hash, in bits
    /// \return The size, in bits, of a hash produced by a hash algorithm.
    int hash_size() const noexcept
    {
        return _hash_size;
    }

protected:
    /// \brief  Construct a HashAlgorithm object.
    ///         Invoked by derived class' constructors to set the hash size in bits.
    /// \param hash_size    Length in bits of the algorithm's resultant hash value.
    explicit HashAlgorithm(int hash_size) noexcept
      : _hash_size{hash_size}
    {}

    /// \brief  Perform the hash operation.
    ///
    /// \param  input  A pointer to raw data to be hashed.
    /// \param  length  Length of the data pointed to by \p buffer.
    ///
    /// Override this function in derived classes to compute the hash.
    ///
    /// This function may be called multiple times during computation of a hash,
    /// allowing the hash to be computed in chunks.
    virtual void do_hash(const uint8_t *input, size_t length) = 0;

    /// \brief  Finalize the hash operation and return the resulting hash.
    ///
    /// Override this function in derived classes to finalize the computation
    /// of the hash and return the value of the hash.
    virtual std::vector<uint8_t> finalize_hash() = 0;

    /// \brief  Initialize or reinitialize the algorithm's internal state.
    ///
    /// Override this function in derived classes to initialize the hash algorithm.
    virtual void reset() = 0;

private:
    int _hash_size; // Hash size in bits
};

} // namespace brace

#endif  // BRACE_LIB_HASHALGORITHM_INC
