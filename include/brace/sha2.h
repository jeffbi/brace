//////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2023 Jeffrey K. Bienstadt
//
// This file is part of the brace C++ library.
//
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.
//////////////////////////////////////////////////////////////////////

/// \file sha2.h
///
/// \author Jeff Bienstadt

#ifndef BRACE_LIB_SHA2_INC
#define BRACE_LIB_SHA2_INC

#include <algorithm>

#include "brace/bits.h"
#include "brace/hashalgorithm.h"

namespace brace {

///////////////////////////////////////////////
//  32-bit SHA2
///////////////////////////////////////////////

///
/// \brief  Abstract base class for 32-bit SHA2 (SHA-224 and SHA-256) classes.
///
/// Derived classes must implement the \c reset function to provide
/// the initial state data.
///
class sha2_32 : public HashAlgorithm
{
public:
    /// \brief  Default constructor is deleted.
    sha2_32() = delete;

protected:
    /// \brief  Number of elements in the message block array.
    static constexpr size_t message_block_size = 64;

    /// \brief  Constructs a sha_32 object.
    /// \param hash_size    Length in bits of the algorithm's resultant hash value.
    sha2_32(int hash_size) noexcept
      : HashAlgorithm(hash_size)
    {}

    /// \brief  Called by derived class' \c reset function with a
    ///         pointer to the appropriate initial state values.
    void initialize(const uint32_t *initial_state_values) noexcept
    {
        _index = 0;
        _length.reset();

        _state[0] = initial_state_values[0];
        _state[1] = initial_state_values[1];
        _state[2] = initial_state_values[2];
        _state[3] = initial_state_values[3];
        _state[4] = initial_state_values[4];
        _state[5] = initial_state_values[5];
        _state[6] = initial_state_values[6];
        _state[7] = initial_state_values[7];

        std::fill(std::begin(_message_block), std::end(_message_block), 0);
    }

    /* The SHA Sigma and sigma functions */
    uint32_t Sigma0(uint32_t word)
    {
        return rotate_right(word, 2) ^ rotate_right(word, 13) ^ rotate_right(word, 22);
    }

    uint32_t Sigma1(uint32_t word)
    {
        return rotate_right(word, 6) ^ rotate_right(word, 11) ^ rotate_right(word, 25);
    }

    uint32_t sigma0(uint32_t word)
    {
        return rotate_right(word, 7) ^ rotate_right(word, 18) ^ (word >> 3);
    }

    uint32_t sigma1(uint32_t word)
    {
        return rotate_right(word, 17) ^ rotate_right(word, 19) ^ (word >> 10);
    }

private:
    void do_hash(const uint8_t *input, size_t length) override
    {
        if (length == 0)
            return;

        while (length--)
        {
            _message_block[_index++] = (*input & 0xFF);

            ++_length;  // This will throw on overflow

            if (_index == message_block_size)
                process_message_block();

            ++input;
        }
    }

    std::vector<uint8_t> finalize_hash() override
    {
        pad_message();

        auto    nbytes{hash_size() / 8};

        std::vector<uint8_t>    digest(nbytes);

        for (int i = 0; i < nbytes; ++i)
            digest[i] = (uint8_t)(_state[i >> 2] >> 8 * (3 - (i & 0x03)));

        reset();

        return digest;
    }

    void process_message_block()
    {
        static constexpr uint32_t K[64] = {
                0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
                0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
                0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
                0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
                0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
                0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
                0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
                0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
                0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
                0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
                0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
                0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
                0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
                0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
                0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
                0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
            };

        int         t, t4;  // Loop counter
        uint32_t    W[64];  // Word sequence

        //
        // Initialize the first 16 words in the array W
        //
        for (t = t4 = 0; t < 16; t++, t4 += 4)
        {
            W[t] = (((uint32_t)_message_block[t4 + 0]) << 24)
                | (((uint32_t)_message_block[t4 + 1]) << 16)
                | (((uint32_t)_message_block[t4 + 2]) << 8)
                | (((uint32_t)_message_block[t4 + 3]));
        }

        for (t = 16; t < 64; t++)
            W[t] = sigma1(W[t - 2]) + W[t - 7] + sigma0(W[t - 15]) + W[t - 16];

        uint32_t    temp1, temp2;   // Temporary word values
        uint32_t    a{_state[0]},   // Word buffers
                    b{_state[1]},
                    c{_state[2]},
                    d{_state[3]},
                    e{_state[4]},
                    f{_state[5]},
                    g{_state[6]},
                    h{_state[7]};

        for (t = 0; t < 64; t++)
        {
            temp1 = h + Sigma1(e) + SHA_Ch(e, f, g) + K[t] + W[t];
            temp2 = Sigma0(a) + SHA_Maj(a, b, c);
            h = g;
            g = f;
            f = e;
            e = d + temp1;
            d = c;
            c = b;
            b = a;
            a = temp1 + temp2;
        }

        _state[0] += a;
        _state[1] += b;
        _state[2] += c;
        _state[3] += d;
        _state[4] += e;
        _state[5] += f;
        _state[6] += g;
        _state[7] += h;

        _index = 0;
    }

    void pad_message()
    {
        static constexpr uint8_t    Pad_Byte{0x80};

        //
        // Check to see if the current message block is too small to hold
        // the initial padding bits and length.  If so, we will pad the
        // block, process it, and then continue padding into a second
        // block.
        //
        if (_index >= (message_block_size - 8))
        {
            _message_block[_index++] = Pad_Byte;
            while (_index < message_block_size)
                _message_block[_index++] = 0;

            process_message_block();
        }
        else
        {
            _message_block[_index++] = Pad_Byte;
        }

        while (_index < (message_block_size - 8))
            _message_block[_index++] = 0;

        /*
        * Store the message length as the last 8 octets
        */
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

    uint32_t        _state[8];
    HashLength64_t  _length;// message length, in bits, with overflow detection
    size_t          _index;
    uint8_t         _message_block[message_block_size];
};

/// \brief  Implements the SHA-224 hash algorithm
///
/// The SHA-224 hash algorithm generates a 224-bit (28-byte) hash.
class SHA224 : public sha2_32
{
public:
    /// \brief  Constructs a SHA224 object
    SHA224() noexcept
      : sha2_32{224}
    {
        reset();
    }

private:
    void reset() override
    {
        static constexpr uint32_t H[8] =
            {
                0xC1059ED8,
                0x367CD507,
                0x3070DD17,
                0xF70E5939,
                0xFFC00B31,
                0x68581511,
                0x64F98FA7,
                0xBEFA4FA4
            };

        sha2_32::initialize(H);
    }
};

/// \brief  Implements the SHA-256 hash algorithm
///
/// The SHA-256 hash algorithm generates a 256-bit (32-byte) hash.
class SHA256 : public sha2_32
{
public:
    /// \brief  Constructs a SHA256 object
    SHA256() noexcept
      : sha2_32{256}
    {
        reset();
    }

private:
    void reset() override
    {
        static constexpr uint32_t H[8] =
            {
                0x6A09E667,
                0xBB67AE85,
                0x3C6EF372,
                0xA54FF53A,
                0x510E527F,
                0x9B05688C,
                0x1F83D9AB,
                0x5BE0CD19
            };

        sha2_32::initialize(H);
    }
};


///////////////////////////////////////////////
//  64-bit SHA2
///////////////////////////////////////////////


///
/// \brief  Abstract base class for 64-bit SHA2 (SHA-384 and SHA-512) classes.
///
/// Derived classes must implement the \c reset function to provide
/// the initial state data.
///
class sha2_64 : public HashAlgorithm
{
public:
    /// \brief  Default constructor is deleted.
    sha2_64() = delete;

protected:
    /// \brief  Number of elements in the message block array.
    static constexpr size_t message_block_size = 128;

    /// \brief  Constructs a sha_64 object.
    /// \param hash_size    Length in bits of the algorithm's resultant hash value.
    sha2_64(int hash_size) noexcept
      : HashAlgorithm{hash_size}
    {}

    /// \brief  Called by derived class' \c reset function,
    /// \param  initial_state_values    pointer to the algorithm's initial
    ///                                 state values.
    void initialize(const uint64_t *initial_state_values) noexcept
    {
        _index = 0;
        _length.reset();

        _state[0] = initial_state_values[0];
        _state[1] = initial_state_values[1];
        _state[2] = initial_state_values[2];
        _state[3] = initial_state_values[3];
        _state[4] = initial_state_values[4];
        _state[5] = initial_state_values[5];
        _state[6] = initial_state_values[6];
        _state[7] = initial_state_values[7];

        std::fill(std::begin(_message_block), std::end(_message_block), 0);
    }

    /* The SHA Sigma and sigma functions */
    uint64_t Sigma0(uint64_t word)
    {
        return rotate_right(word, 28) ^ rotate_right(word, 34) ^ rotate_right(word, 39);
    }

    uint64_t Sigma1(uint64_t word)
    {
        return rotate_right(word, 14) ^ rotate_right(word, 18) ^ rotate_right(word, 41);
    }

    uint64_t sigma0(uint64_t word)
    {
        return rotate_right(word, 1) ^ rotate_right(word, 8) ^ (word >> 7);
    }

    uint64_t sigma1(uint64_t word)
    {
        return rotate_right(word, 19) ^ rotate_right(word, 61) ^ (word >> 6);
    }

private:
    void do_hash(const uint8_t *input, size_t length) override
    {
        if (length == 0)
            return;

        while (length--)
        {
            _message_block[_index++] = (*input & 0xFF);

            ++_length;  // This will throw on overflow.

            if (_index == message_block_size)
                process_message_block();

            ++input;
        }
    }

    std::vector<uint8_t> finalize_hash() override
    {
        pad_message();

        auto    nbytes{hash_size() / 8};

        std::vector<uint8_t>    digest(nbytes);

        for (int i = 0; i < nbytes; ++i)
            digest[i] = (uint8_t)(_state[i >> 3] >> 8 * (7 - (i % 8)));

        reset();

        return digest;
    }

    void process_message_block()
    {
        static constexpr uint64_t K[80] =
            {
                0x428A2F98D728AE22ull, 0x7137449123EF65CDull, 0xB5C0FBCFEC4D3B2Full, 0xE9B5DBA58189DBBCull,
                0x3956C25BF348B538ull, 0x59F111F1B605D019ull, 0x923F82A4AF194F9Bull, 0xAB1C5ED5DA6D8118ull,
                0xD807AA98A3030242ull, 0x12835B0145706FBEull, 0x243185BE4EE4B28Cull, 0x550C7DC3D5FFB4E2ull,
                0x72BE5D74F27B896Full, 0x80DEB1FE3B1696B1ull, 0x9BDC06A725C71235ull, 0xC19BF174CF692694ull,
                0xE49B69C19EF14AD2ull, 0xEFBE4786384F25E3ull, 0x0FC19DC68B8CD5B5ull, 0x240CA1CC77AC9C65ull,
                0x2DE92C6F592B0275ull, 0x4A7484AA6EA6E483ull, 0x5CB0A9DCBD41FBD4ull, 0x76F988DA831153B5ull,
                0x983E5152EE66DFABull, 0xA831C66D2DB43210ull, 0xB00327C898FB213Full, 0xBF597FC7BEEF0EE4ull,
                0xC6E00BF33DA88FC2ull, 0xD5A79147930AA725ull, 0x06CA6351E003826Full, 0x142929670A0E6E70ull,
                0x27B70A8546D22FFCull, 0x2E1B21385C26C926ull, 0x4D2C6DFC5AC42AEDull, 0x53380D139D95B3DFull,
                0x650A73548BAF63DEull, 0x766A0ABB3C77B2A8ull, 0x81C2C92E47EDAEE6ull, 0x92722C851482353Bull,
                0xA2BFE8A14CF10364ull, 0xA81A664BBC423001ull, 0xC24B8B70D0F89791ull, 0xC76C51A30654BE30ull,
                0xD192E819D6EF5218ull, 0xD69906245565A910ull, 0xF40E35855771202Aull, 0x106AA07032BBD1B8ull,
                0x19A4C116B8D2D0C8ull, 0x1E376C085141AB53ull, 0x2748774CDF8EEB99ull, 0x34B0BCB5E19B48A8ull,
                0x391C0CB3C5C95A63ull, 0x4ED8AA4AE3418ACBull, 0x5B9CCA4F7763E373ull, 0x682E6FF3D6B2B8A3ull,
                0x748F82EE5DEFB2FCull, 0x78A5636F43172F60ull, 0x84C87814A1F0AB72ull, 0x8CC702081A6439ECull,
                0x90BEFFFA23631E28ull, 0xA4506CEBDE82BDE9ull, 0xBEF9A3F7B2C67915ull, 0xC67178F2E372532Bull,
                0xCA273ECEEA26619Cull, 0xD186B8C721C0C207ull, 0xEADA7DD6CDE0EB1Eull, 0xF57D4F7FEE6ED178ull,
                0x06F067AA72176FBAull, 0x0A637DC5A2C898A6ull, 0x113F9804BEF90DAEull, 0x1B710B35131C471Bull,
                0x28DB77F523047D84ull, 0x32CAAB7B40C72493ull, 0x3C9EBE0A15C9BEBCull, 0x431D67C49C100D4Cull,
                0x4CC5D4BECB3E42B6ull, 0x597F299CFC657E2Aull, 0x5FCB6FAB3AD6FAECull, 0x6C44198C4A475817ull
            };

        int         t, t8;  // Loop counter
        uint64_t    W[80];  // Word sequence

        //
        // Initialize the first 16 words in the array W
        //
        for (t = t8 = 0; t < 16; t++, t8 += 8)
        {
            W[t] = (((uint64_t)_message_block[t8 + 0]) << 56)
                | (((uint64_t)_message_block[t8 + 1]) << 48)
                | (((uint64_t)_message_block[t8 + 2]) << 40)
                | (((uint64_t)_message_block[t8 + 3]) << 32)
                | (((uint64_t)_message_block[t8 + 4]) << 24)
                | (((uint64_t)_message_block[t8 + 5]) << 16)
                | (((uint64_t)_message_block[t8 + 6]) << 8)
                | (((uint64_t)_message_block[t8 + 7]));
        }

        for (t = 16; t < 80; t++)
            W[t] = sigma1(W[t - 2]) + W[t - 7] + sigma0(W[t - 15]) + W[t - 16];

        uint64_t    temp1, temp2;   // Temporary word values
        uint64_t    a{_state[0]},   // Word buffers
                    b{_state[1]},
                    c{_state[2]},
                    d{_state[3]},
                    e{_state[4]},
                    f{_state[5]},
                    g{_state[6]},
                    h{_state[7]};

        for (t = 0; t < 80; t++)
        {
            temp1 = h + Sigma1(e) + SHA_Ch(e, f, g) + K[t] + W[t];
            temp2 = Sigma0(a) + SHA_Maj(a, b, c);
            h = g;
            g = f;
            f = e;
            e = d + temp1;
            d = c;
            c = b;
            b = a;
            a = temp1 + temp2;
        }

        _state[0] += a;
        _state[1] += b;
        _state[2] += c;
        _state[3] += d;
        _state[4] += e;
        _state[5] += f;
        _state[6] += g;
        _state[7] += h;

        _index = 0;
    }

    void pad_message()
    {
        static constexpr uint8_t    Pad_Byte{0x80};

        //
        // Check to see if the current message block is too small to hold
        // the initial padding bits and length.  If so, we will pad the
        // block, process it, and then continue padding into a second block.
        //
        if (_index >= (message_block_size - 16))
        {
            _message_block[_index++] = Pad_Byte;
            while (_index < message_block_size)
                _message_block[_index++] = 0;

            process_message_block();
        }
        else
        {
            _message_block[_index++] = Pad_Byte;
        }

        while (_index < (message_block_size - 16))
            _message_block[_index++] = 0;

        /*
        * Store the message length in the last 16 octets
        */
        _message_block[112] = (uint8_t)(_length.high() >> 56);
        _message_block[113] = (uint8_t)(_length.high() >> 48);
        _message_block[114] = (uint8_t)(_length.high() >> 40);
        _message_block[115] = (uint8_t)(_length.high() >> 32);
        _message_block[116] = (uint8_t)(_length.high() >> 24);
        _message_block[117] = (uint8_t)(_length.high() >> 16);
        _message_block[118] = (uint8_t)(_length.high() >> 8);
        _message_block[119] = (uint8_t)(_length.high());

        _message_block[120] = (uint8_t)(_length.low() >> 56);
        _message_block[121] = (uint8_t)(_length.low() >> 48);
        _message_block[122] = (uint8_t)(_length.low() >> 40);
        _message_block[123] = (uint8_t)(_length.low() >> 32);
        _message_block[124] = (uint8_t)(_length.low() >> 24);
        _message_block[125] = (uint8_t)(_length.low() >> 16);
        _message_block[126] = (uint8_t)(_length.low() >> 8);
        _message_block[127] = (uint8_t)(_length.low());

        process_message_block();
    }

    uint64_t        _state[8];
    HashLength128_t _length;    // message length, in bits, with overflow detection
    size_t          _index;
    uint8_t         _message_block[message_block_size];
};


/// \brief  Implements the SHA-384 hash algorithm
///
/// The SHA-384 hash algorithm generates a 384-bit (48-byte) hash.
class SHA384 : public sha2_64
{
public:
    /// \brief  Constructs a SHA384 object
    SHA384() noexcept
      : sha2_64{384}
    {
        reset();
    }

private:
    void reset() override
    {
        static constexpr uint64_t H[8] =
            {
                0xCBBB9D5DC1059ED8ull,
                0x629A292A367CD507ull,
                0x9159015A3070DD17ull,
                0x152FECD8F70E5939ull,
                0x67332667FFC00B31ull,
                0x8EB44A8768581511ull,
                0xDB0C2E0D64F98FA7ull,
                0x47B5481DBEFA4FA4ull
            };

        sha2_64::initialize(H);
    }
};

/// \brief  Implements the SHA-512 hash algorithm
///
/// The SHA-512 hash algorithm generates a 512-bit (64-byte) hash.
class SHA512 : public sha2_64
{
public:
    /// \brief  Constructs a SHA512 object
    SHA512() noexcept
      : sha2_64{512}
    {
        reset();
    }

private:
    void reset() override
    {
        static constexpr uint64_t H[8] =
            {
                0x6A09E667F3BCC908ull,
                0xBB67AE8584CAA73Bull,
                0x3C6EF372FE94F82Bull,
                0xA54FF53A5F1D36F1ull,
                0x510E527FADE682D1ull,
                0x9B05688C2B3E6C1Full,
                0x1F83D9ABFB41BD6Bull,
                0x5BE0CD19137E2179ull
            };

        sha2_64::initialize(H);
    }

};

} // namespace brace

#endif  // BRACE_LIB_SHA2_INC
