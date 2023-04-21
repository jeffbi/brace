//////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2023 Jeffrey K. Bienstadt
//
// This file is part of the brace C++ library.
//
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.
//////////////////////////////////////////////////////////////////////

/// \file md5.h
///
/// \author Jeff Bienstadt

#ifndef BRACE_LIB_MD5_INC
#define BRACE_LIB_MD5_INC

#include <algorithm>
#include <cstring>

#include "brace/bits.h"
#include "brace/hashalgorithm.h"

namespace brace {

/*
*   This MD5 hash is based on the code presented in RFC-1321
*   (https://www.rfc-editor.org/info/rfc1321), which includes
*   the following notice:
*
*   Copyright (C) 1991-2, RSA Data Security, Inc. Created 1991. All
*   rights reserved.
*
*   License to copy and use this software is granted provided that it
*   is identified as the "RSA Data Security, Inc. MD5 Message-Digest
*   Algorithm" in all material mentioning or referencing this software
*   or this function.
*
*   License is also granted to make and use derivative works provided
*   that such works are identified as "derived from the RSA Data
*   Security, Inc. MD5 Message-Digest Algorithm" in all material
*   mentioning or referencing the derived work.
*
*   RSA Data Security, Inc. makes no representations concerning either
*   the merchantability of this software or the suitability of this
*   software for any particular purpose. It is provided "as is"
*   without express or implied warranty of any kind.
*
*   These notices must be retained in any copies of any part of this
*   documentation and/or software.
*/
/// \brief  Implements an MD5 hash algorithm.
class MD5 : public HashAlgorithm
{
private:
    using uint1 = uint8_t;
    using uint4 = uint32_t;

public:
    using size_type = uint32_t;
    static constexpr size_type  message_block_size{64};

public:
    MD5() noexcept
      : HashAlgorithm(128)
    {
        reset();
    }

private:
    void do_hash(const uint8_t *input, size_t length) override
    {
//void MD5::update(const unsigned char input[], size_type length)
        // compute number of bytes mod 64
        size_type   index{_count[0] / 8 % message_block_size};

        // Update number of bits
        if ((_count[0] += (length << 3)) < (length << 3))
            _count[1]++;
        _count[1] += (length >> 29);

        // number of bytes we need to fill in buffer
        size_type   firstpart{64 - index};
        size_type   i;

        // transform as many times as possible.
        if (length >= firstpart)
        {
            // fill buffer first, transform
            memcpy(&_buffer[index], input, firstpart);
            transform(_buffer);

            // transform chunks of blocksize (64 bytes)
            for (i = firstpart; i + message_block_size <= length; i += message_block_size)
                transform(&input[i]);

            index = 0;
        }
        else
        {
            i = 0;
        }

        // buffer remaining input
        memcpy(&_buffer[index], &input[i], length - i);
    }

    std::vector<uint8_t> finalize_hash()
    {
        static uint8_t padding[64] =
            {
                0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
            };

        if (!_finalized)
        {
            // Save number of bits
            uint8_t     bits[8];
            encode(bits, _count, 8);

            // pad out to 56 mod 64.
            size_type   index{_count[0] / 8 % 64};
            size_type   padLen{(index < 56) ? (56 - index) : (120 - index)};
            do_hash(padding, padLen);

            // Append length (before padding)
            do_hash(bits, 8);

            // Store state in digest
            encode(_digest, _state, 16);

            // Zeroize sensitive information.
            memset(_buffer, 0, sizeof _buffer);
            memset(_count, 0, sizeof _count);

            _finalized = true;
        }

        return std::vector<uint8_t>(std::begin(_digest), std::end(_digest));
    }

    void reset() override
    {
        _finalized = false;

        _count[0] = 0;
        _count[1] = 0;

        // load magic initialization constants.
        _state[0] = 0x67452301;
        _state[1] = 0xefcdab89;
        _state[2] = 0x98badcfe;
        _state[3] = 0x10325476;
    }

    void transform(const uint1 block[message_block_size])
    {
        static constexpr uint4  S11{7};
        static constexpr uint4  S12{12};
        static constexpr uint4  S13{17};
        static constexpr uint4  S14{22};
        static constexpr uint4  S21{5};
        static constexpr uint4  S22{9};
        static constexpr uint4  S23{14};
        static constexpr uint4  S24{20};
        static constexpr uint4  S31{4};
        static constexpr uint4  S32{11};
        static constexpr uint4  S33{16};
        static constexpr uint4  S34{23};
        static constexpr uint4  S41{6};
        static constexpr uint4  S42{10};
        static constexpr uint4  S43{15};
        static constexpr uint4  S44{21};

        uint4   a = _state[0],
                b = _state[1],
                c = _state[2],
                d = _state[3],
                x[16];

        decode(x, block, message_block_size);

        /* Round 1 */
        FF(a, b, c, d, x[ 0], S11, 0xd76aa478); /* 1 */
        FF(d, a, b, c, x[ 1], S12, 0xe8c7b756); /* 2 */
        FF(c, d, a, b, x[ 2], S13, 0x242070db); /* 3 */
        FF(b, c, d, a, x[ 3], S14, 0xc1bdceee); /* 4 */
        FF(a, b, c, d, x[ 4], S11, 0xf57c0faf); /* 5 */
        FF(d, a, b, c, x[ 5], S12, 0x4787c62a); /* 6 */
        FF(c, d, a, b, x[ 6], S13, 0xa8304613); /* 7 */
        FF(b, c, d, a, x[ 7], S14, 0xfd469501); /* 8 */
        FF(a, b, c, d, x[ 8], S11, 0x698098d8); /* 9 */
        FF(d, a, b, c, x[ 9], S12, 0x8b44f7af); /* 10 */
        FF(c, d, a, b, x[10], S13, 0xffff5bb1); /* 11 */
        FF(b, c, d, a, x[11], S14, 0x895cd7be); /* 12 */
        FF(a, b, c, d, x[12], S11, 0x6b901122); /* 13 */
        FF(d, a, b, c, x[13], S12, 0xfd987193); /* 14 */
        FF(c, d, a, b, x[14], S13, 0xa679438e); /* 15 */
        FF(b, c, d, a, x[15], S14, 0x49b40821); /* 16 */

        /* Round 2 */
        GG(a, b, c, d, x[ 1], S21, 0xf61e2562); /* 17 */
        GG(d, a, b, c, x[ 6], S22, 0xc040b340); /* 18 */
        GG(c, d, a, b, x[11], S23, 0x265e5a51); /* 19 */
        GG(b, c, d, a, x[ 0], S24, 0xe9b6c7aa); /* 20 */
        GG(a, b, c, d, x[ 5], S21, 0xd62f105d); /* 21 */
        GG(d, a, b, c, x[10], S22,  0x2441453); /* 22 */
        GG(c, d, a, b, x[15], S23, 0xd8a1e681); /* 23 */
        GG(b, c, d, a, x[ 4], S24, 0xe7d3fbc8); /* 24 */
        GG(a, b, c, d, x[ 9], S21, 0x21e1cde6); /* 25 */
        GG(d, a, b, c, x[14], S22, 0xc33707d6); /* 26 */
        GG(c, d, a, b, x[ 3], S23, 0xf4d50d87); /* 27 */
        GG(b, c, d, a, x[ 8], S24, 0x455a14ed); /* 28 */
        GG(a, b, c, d, x[13], S21, 0xa9e3e905); /* 29 */
        GG(d, a, b, c, x[ 2], S22, 0xfcefa3f8); /* 30 */
        GG(c, d, a, b, x[ 7], S23, 0x676f02d9); /* 31 */
        GG(b, c, d, a, x[12], S24, 0x8d2a4c8a); /* 32 */

        /* Round 3 */
        HH(a, b, c, d, x[ 5], S31, 0xfffa3942); /* 33 */
        HH(d, a, b, c, x[ 8], S32, 0x8771f681); /* 34 */
        HH(c, d, a, b, x[11], S33, 0x6d9d6122); /* 35 */
        HH(b, c, d, a, x[14], S34, 0xfde5380c); /* 36 */
        HH(a, b, c, d, x[ 1], S31, 0xa4beea44); /* 37 */
        HH(d, a, b, c, x[ 4], S32, 0x4bdecfa9); /* 38 */
        HH(c, d, a, b, x[ 7], S33, 0xf6bb4b60); /* 39 */
        HH(b, c, d, a, x[10], S34, 0xbebfbc70); /* 40 */
        HH(a, b, c, d, x[13], S31, 0x289b7ec6); /* 41 */
        HH(d, a, b, c, x[ 0], S32, 0xeaa127fa); /* 42 */
        HH(c, d, a, b, x[ 3], S33, 0xd4ef3085); /* 43 */
        HH(b, c, d, a, x[ 6], S34,  0x4881d05); /* 44 */
        HH(a, b, c, d, x[ 9], S31, 0xd9d4d039); /* 45 */
        HH(d, a, b, c, x[12], S32, 0xe6db99e5); /* 46 */
        HH(c, d, a, b, x[15], S33, 0x1fa27cf8); /* 47 */
        HH(b, c, d, a, x[ 2], S34, 0xc4ac5665); /* 48 */

        /* Round 4 */
        II(a, b, c, d, x[ 0], S41, 0xf4292244); /* 49 */
        II(d, a, b, c, x[ 7], S42, 0x432aff97); /* 50 */
        II(c, d, a, b, x[14], S43, 0xab9423a7); /* 51 */
        II(b, c, d, a, x[ 5], S44, 0xfc93a039); /* 52 */
        II(a, b, c, d, x[12], S41, 0x655b59c3); /* 53 */
        II(d, a, b, c, x[ 3], S42, 0x8f0ccc92); /* 54 */
        II(c, d, a, b, x[10], S43, 0xffeff47d); /* 55 */
        II(b, c, d, a, x[ 1], S44, 0x85845dd1); /* 56 */
        II(a, b, c, d, x[ 8], S41, 0x6fa87e4f); /* 57 */
        II(d, a, b, c, x[15], S42, 0xfe2ce6e0); /* 58 */
        II(c, d, a, b, x[ 6], S43, 0xa3014314); /* 59 */
        II(b, c, d, a, x[13], S44, 0x4e0811a1); /* 60 */
        II(a, b, c, d, x[ 4], S41, 0xf7537e82); /* 61 */
        II(d, a, b, c, x[11], S42, 0xbd3af235); /* 62 */
        II(c, d, a, b, x[ 2], S43, 0x2ad7d2bb); /* 63 */
        II(b, c, d, a, x[ 9], S44, 0xeb86d391); /* 64 */

        _state[0] += a;
        _state[1] += b;
        _state[2] += c;
        _state[3] += d;

        std::fill(x, x + 16, 0);
    }

    // decodes input (unsigned char) into output (uint4). Assumes len is a multiple of 4.
    void decode(uint4 output[], const uint1 input[], size_type len)
    {
        for (unsigned int i = 0, j = 0; j < len; i++, j += 4)
            output[i] = ((uint4)input[j]) |
                        (((uint4)input[j+1]) << 8) |
                        (((uint4)input[j+2]) << 16) |
                        (((uint4)input[j+3]) << 24);
    }

    // encodes input (uint4) into output (unsigned char). Assumes len is
    // a multiple of 4.
    void encode(uint1 output[], const uint4 input[], size_type len)
    {
        for (size_type i = 0, j = 0; j < len; i++, j += 4)
        {
            output[j]   = input[i] & 0xFF;
            output[j+1] = (input[i] >> 8) & 0xFF;
            output[j+2] = (input[i] >> 16) & 0xFF;
            output[j+3] = (input[i] >> 24) & 0xFF;
        }
    }

private:
    static uint4 F(uint4 x, uint4 y, uint4 z)
    {
        return x & y | ~x & z;
    }
    static uint4 G(uint4 x, uint4 y, uint4 z)
    {
        return x & z | y & ~z;
    }
    static uint4 H(uint4 x, uint4 y, uint4 z)
    {
        return x ^ y ^ z;
    }
    static uint4 I(uint4 x, uint4 y, uint4 z)
    {
        return y ^ (x | ~z);
    }
    //static uint4 rotate_left(uint4 x, int n);
    static void FF(uint4 &a, uint4 b, uint4 c, uint4 d, uint4 x, uint4 s, uint4 ac)
    {
        a = rotate_left(a+ F(b, c, d) + x + ac, s) + b;
    }
    static void GG(uint4 &a, uint4 b, uint4 c, uint4 d, uint4 x, uint4 s, uint4 ac)
    {
        a = rotate_left(a + G(b, c, d) + x + ac, s) + b;
    }
    static void HH(uint4 &a, uint4 b, uint4 c, uint4 d, uint4 x, uint4 s, uint4 ac)
    {
        a = rotate_left(a + H(b, c, d) + x + ac, s) + b;
    }
    static void II(uint4 &a, uint4 b, uint4 c, uint4 d, uint4 x, uint4 s, uint4 ac)
    {
        a = rotate_left(a + I(b, c, d) + x + ac, s) + b;
    }

private:
    bool    _finalized{false};
    uint1   _buffer[message_block_size]; // bytes that didn't fit in last 64 byte chunk
    uint4   _count[2];   // 64bit counter for number of bits (lo, hi)
    uint4   _state[4];   // digest so far
    uint1   _digest[16]; // the result
};

}

#endif  // BRACE_LIB_MD5_INC
