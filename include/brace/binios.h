//////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2023 Jeffrey K. Bienstadt
//
// This file is part of the brace C++ library.
//
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.
//////////////////////////////////////////////////////////////////////

/// \file binios.h
///
/// \author Jeff Bienstadt
#ifndef _BRACE_LIB_BINIOS_INC
#define _BRACE_LIB_BINIOS_INC

#include <cstdint>
#include <ios>
#include <string>   // for char_traits

#include "byteorder.h"

namespace brace
{

class BinStreamImpl;

/// \brief  The BinIos class provides a base class for binary stream classes.
///
/// The class shares several aspects with std::basic_ios, such as state flags,
/// but does not include formatting flags or locales since it is not meant to
/// interface with classes that process formatted data.
class BinIos
{
private:
    static constexpr std::ios_base::iostate state_mask = std::ios_base::goodbit | std::ios_base::eofbit | std::ios_base::badbit | std::ios::failbit;

public:
    using byte_type     = uint8_t;                  ///< Type used to represent a byte
    using traits_type   = std::char_traits<char>;   ///< Type used for character traits
    using int_type      = traits_type::int_type;    ///< Type used for integers
    using pos_type      = std::streampos;           ///< Type used for positioning
    using off_type      = std::streamoff;           ///< Type used for offsets

public:
    BinIos()
        : _state_bits{std::ios_base::goodbit}
        , _exception_mask{std::ios_base::goodbit}
    {}

    /// \brief  Construct a BinIos object with a BinStreamImpl object.
    /// \param impl Pointer to a BinStreamImpl object.
    BinIos(BinStreamImpl *impl) noexcept
        : _state_bits{std::ios_base::goodbit}
        , _exception_mask{std::ios_base::goodbit}
        , _impl{impl}
    {}

    /// \brief The copy constructor is deleted. BinIos objects are not copy constructable.
    BinIos(const BinIos &) = delete;
    /// \brief The copy assignment operator is deleted. BinIos objects are not copy assignable.
    BinIos & operator=(const BinIos &) = delete;

    /// \brief  Construct a BinIos object by moving data from another BinIos object.
    /// \param other    Another BinIos object from which to move data.
    BinIos(BinIos &&other)
        : _state_bits{other._state_bits}
        , _exception_mask{other._exception_mask}
        , _endian{other._endian}
        , _impl{other._impl}
    {
        other._state_bits = std::ios_base::goodbit;
        other._exception_mask = std::ios_base::goodbit;
        other._endian = Endian::Native;
        other._impl = nullptr;
    }

    /// \brief  Assign to a BinIos object by moving data from another BinIos object.
    /// \param rhs  Another BinIos object from which to move data
    /// \return *this
    BinIos &operator=(BinIos &&rhs)
    {
        if (this != std::addressof(rhs))
        {
            _state_bits = rhs._state_bits;
            rhs._state_bits = std::ios_base::goodbit;
            _exception_mask = rhs._exception_mask;
            rhs._exception_mask = std::ios_base::goodbit;
            _endian = rhs._endian;
            rhs._endian = Endian::Native;
            _impl = rhs._impl;
            rhs._impl = nullptr;
        }

        return *this;
    }

    virtual ~BinIos()
    {}

public:
    /// \brief  Retrieve the Endian setting for this object.
    /// \return An Endian enum indicating the byte ordering used.
    Endian endian() const noexcept
    {
        return _endian;
    }

    /// \brief  Set the Endian setting for this object.
    /// \param e    An Endian enum specifying the byte ordering used.
    void endian(Endian e) noexcept
    {
        _endian = e;
    }

    //TODO: use std::ios_base::failure rather than own Failure class.


    //
    // State functions
    //

    /// \brief  Return the current stream error state.
    /// \return A bitmask type that can be a combination of std::ios_base::goodbit,
    ///         std::ios_base::badbit, std::ios_base::failbit, std::ios_base::eofbit.
    std::ios_base::iostate rdstate() const noexcept
    {
        return _state_bits;
    }

    /// \brief  Return \c true if no error state bits are set.
    /// \return \c true if all error flags are false, \c false otherwise.
    bool good() const noexcept
    {
        return rdstate() == std::ios::goodbit;
    }

    /// \brief  Return \c true if the associated stream has reached end of file.
    /// \return \c true if end of file has occurred, \c false otherwise.
    bool eof() const noexcept
    {
        return (rdstate() & std::ios_base::eofbit) != 0;
    }

    /// \brief  Return \c true if an error has occurred on the associated stream.
    /// \return \c true if an error has occurred, \c false otherwise.
    bool fail() const noexcept
    {
        return (rdstate() & (std::ios_base::failbit | std::ios_base::badbit)) != 0;
    }

    /// \brief  Return \c true if a non-recoverable error has occurred on the assiciated stream.
    /// \return \c true if a non-recoverable error has occurred, \c false otherwise.
    bool bad() const noexcept
    {
        return (rdstate() & std::ios_base::badbit) != 0;
    }

    /// \brief  Return true if an error has occurred on the associated stream.
    /// \return \c true if an error has occurred, \c false otherwise.
    bool operator !() const noexcept
    {
        return fail();
    }

    /// \brief  Check whether the associated stream has no errors.
    /// \return \c true if the stream has no errors, \c false otherwise.
    explicit operator bool() const
    {
        return !fail();
    }

    /// \brief  Set the stream error state flags by assigning them the value state.
    ///         By default assigns the value std::ios_base::goodbit, which clears
    ///         all error state flags.
    /// \param state    New error state flags. Can be a combination of std::ios_base::goodbit,
    ///                 std::ios_base::badbit, std::ios_base::failbit, std::ios_base::eofbit.
    void clear(std::ios_base::iostate state = std::ios_base::goodbit)
    {
        _state_bits = state & state_mask;
    }

    /// \brief  Set the stream error flags state in addition to currently set flags.
    /// \param state    Stream error flags to be set. Can be a combination of std::ios_base::goodbit,
    ///                 std::ios_base::badbit, std::ios_base::failbit, std::ios_base::eofbit.
    void setstate(std::ios_base::iostate state)
    {
        clear(rdstate() | state);

        const std::ios_base::iostate    excepts{exceptions() & rdstate()};

        throw_on_exception_bits(excepts);
    }

    /// \brief  Get the exception mask of the stream
    /// \return The current exception mask.
    std::ios_base::iostate exceptions() const noexcept
    {
        return _exception_mask;
    }

    /// \brief  Sets the exception mask to except.
    /// \param except   New exception mask to set.
    void exceptions(std::ios_base::iostate except)
    {
        _exception_mask = except & state_mask;

        const std::ios_base::iostate    excepts{rdstate() & _exception_mask};

        throw_on_exception_bits(excepts);
    }

    /// \brief  Return the associated stream implementation object.
    /// \return Pointer to the associated stream implementation object.
    BinStreamImpl *rdimpl() const
    {
        return _impl;
    }

    /// \brief  Set the associated stream implementation to impl.
    /// \param impl Pointer to the new implementation object.
    /// \return Pointer to the previous implementation object.
    BinStreamImpl *rdimpl(BinStreamImpl *impl)
    {
        auto old_impl{_impl};
        clear();
        _impl = impl;

        return old_impl;
    }

protected:
    /// \brief  Clear specified state bits
    /// \param bits_to_clear    Specifies which state bits to clear.
    void clear_state_bit(std::ios_base::iostate bits_to_clear)
    {
        _state_bits &= ~(bits_to_clear & state_mask);
    }

    /// \brief  Move the contents of another BinStreamImpl object into this one.
    /// \param other    The other BinStreamImpl object whose contents is to be moved.
    void move(BinIos &other)
    {
        _state_bits = other._state_bits;
        _exception_mask = other._exception_mask;
        _endian = other._endian;
        other._state_bits = std::ios_base::goodbit;
        other._exception_mask = std::ios_base::goodbit;
        other._endian = Endian::Native;
    }

    /// \brief  Swap the contents of this object with that of another BinIos object.
    /// \param other    Another BinIos object whose content is to be swapped.
    void swap(BinIos &other)
    {
        if (this != std::addressof(other))
        {
            std::swap(_state_bits, other._state_bits);
            std::swap(_exception_mask, other._exception_mask);
            std::swap(_endian, other._endian);
        }
    }

    /// \brief  Initialize this BinIos object with a pointer to an implementation object.
    /// \param impl A pointer to a BinStreamImpl object that will perform internal operations.
    void init(BinStreamImpl *impl)
    {
        _impl = impl;
    }

private:
    void throw_on_exception_bits(std::ios_base::iostate excepts)
    {
#if defined(__cpp_exceptions)
        if (excepts)
        {
            const char *msg = nullptr;

            if (excepts & std::ios_base::badbit)
                msg = "std::ios_base::badbit set";
            else if (excepts & std::ios_base::failbit)
                msg = "std::ios_base::failbit set";
            else if (excepts & std::ios_base::eofbit)
                msg = "std::ios_base::eofbit set";

            if (msg)
                throw std::ios_base::failure(msg);
        }
#endif
    }
private:
    std::ios_base::iostate  _state_bits;
    std::ios_base::iostate  _exception_mask;
    Endian                  _endian{Endian::Native};
    BinStreamImpl          *_impl{nullptr};
};

}   // namespace brace

#endif  // _BRACE_LIB_BINIOS_INC
