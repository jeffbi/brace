//////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2023 Jeffrey K. Bienstadt
//
// This file is part of the brace C++ library.
//
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.
//////////////////////////////////////////////////////////////////////

/// \file sha1.h
///
/// \author Jeff Bienstadt
#ifndef BRACE_LIB_SHA1_INC
#define BRACE_LIB_SHA1_INC

#include <algorithm>
#include <cstdint>
#include <vector>

#include "bits.h"
#include "hashalgorithm.h"

namespace brace {

/// \brief  Implements the SHA-1 hash algorithm
///
/// The SHA-1 hash algorithm generates a 16-bit (20-byte) hash.
///
/// \note   The SHA-1 hash algorithm is not considered secure and
///         its use is \b not recommended for security-related hashing.
///         Use the more secure SHA-2 family of hash algorithms such as
///         SHA-256 or SHA-521 instead.
class SHA1 : public HashAlgorithm
{
public:
    /// \brief  Constructs a SHA1 hash algorithm object.
    SHA1() noexcept
      : HashAlgorithm{160}
    {
        reset();
    }

private:
    static constexpr size_t message_block_size = 64;    // size of the message-block array

    void do_hash(const uint8_t *input, size_t length) override
    {
        if (length == 0)
            return;

        while (length--)
        {
            _message_block[_index++] = (*input & 0xFF);

            ++_length;  // this will throw on overflow

            if (_index == message_block_size)
                process_message_block();

            ++input;
        }
    }

    std::vector<uint8_t> finalize_hash() override
    {
        pad_message();

        std::vector<uint8_t>    digest(20);

        for (size_t i=0; i < digest.size(); i++)
            digest[i] = (uint8_t)(_state[i >> 2] >> (8 * (3 - (i & 0x03))));

        reset();    // clear any potentially sensitive information

        return digest;
    }

    void reset() override
    {
        _index = 0;
        _length.reset();

        _state[0] = 0x67452301;
        _state[1] = 0xEFCDAB89;
        _state[2] = 0x98BADCFE;
        _state[3] = 0x10325476;
        _state[4] = 0xC3D2E1F0;

        std::fill(std::begin(_message_block), std::end(_message_block), 0);
    }

    void process_message_block()
    {
        static constexpr uint32_t  K[] =
                        {
                            0x5A827999,
                            0x6ED9EBA1,
                            0x8F1BBCDC,
                            0xCA62C1D6
                        };

        int         i;
        uint32_t    W[80];

        //
        // Initialize first 16 words of W
        //
        for (i=0; i < 16; i++)
        {
            W[i]  = ((uint32_t)_message_block[i * 4    ]) << 24;
            W[i] |= ((uint32_t)_message_block[i * 4 + 1]) << 16;
            W[i] |= ((uint32_t)_message_block[i * 4 + 2]) <<  8;
            W[i] |= ((uint32_t)_message_block[i * 4 + 3]);
        }

        for (; i < 80; i++)
        {
            W[i] = rotate_left(W[i - 3] ^ W[i - 8] ^ W[i - 14] ^ W[i - 16], 1);
        }

        uint32_t    temp;
        uint32_t    a{_state[0]},
                    b{_state[1]},
                    c{_state[2]},
                    d{_state[3]},
                    e{_state[4]};

        for (i=0; i < 20; i++)
        {
            temp = rotate_left(a, 5) + SHA_Ch(b, c, d) + e + W[i] + K[0];
            e = d;
            d = c;
            c = rotate_left(b, 30);
            b = a;
            a = temp;
        }

        for (; i < 40; i++)
        {
            temp = rotate_left(a, 5) + SHA_Parity(b, c, d) + e + W[i] + K[1];
            e = d;
            d = c;
            c = rotate_left(b, 30);
            b = a;
            a = temp;
        }

        for (; i < 60; i++)
        {
            temp = rotate_left(a, 5) + SHA_Maj(b, c, d) + e + W[i] + K[2];
            e = d;
            d = c;
            c = rotate_left(b, 30);
            b = a;
            a = temp;
        }

        for (; i < 80; i++)
        {
            temp = rotate_left(a, 5) + SHA_Parity(b, c, d) + e + W[i] + K[3];
            e = d;
            d = c;
            c = rotate_left(b, 30);
            b = a;
            a = temp;
        }

        _state[0] += a;
        _state[1] += b;
        _state[2] += c;
        _state[3] += d;
        _state[4] += e;

        _index = 0;
    }

    void pad_message()
    {
        if (_index >= message_block_size - 8)
        {
            _message_block[_index++] = 0x80;

            while (_index < message_block_size)
                _message_block[_index++] = 0;

            process_message_block();
        }
        else
        {
            _message_block[_index++] = 0x80;
        }

        while (_index < message_block_size - 8)
            _message_block[_index++] = 0;

        //
        // Store the message length in the last 8 octets
        //
        _message_block[56] = _length.high() >> 24;
        _message_block[57] = _length.high() >> 16;
        _message_block[58] = _length.high() >>  8;
        _message_block[59] = _length.high() >>  0;
        _message_block[60] = _length.low() >> 24;
        _message_block[61] = _length.low() >> 16;
        _message_block[62] = _length.low() >>  8;
        _message_block[63] = _length.low() >>  0;

        process_message_block();
    }

    uint32_t        _state[5];
    HashLength64_t  _length;    // message length, in bits, with overflow detection
    size_t          _index;
    uint8_t         _message_block[message_block_size];
};

} // namespace brace

#endif  // BRACE_LIB_SHA1_INC
