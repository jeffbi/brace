#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"

#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "brace/base64.h"
#include "brace/binastream.h"

std::string test_data[][2] = {
    {"",        ""},
    {"f",       "Zg=="},
    {"fo",      "Zm8="},
    {"foo",     "Zm9v"},
    {"foo\n",   "Zm9vCg=="},
    {"foob",    "Zm9vYg=="},
    {"fooba",   "Zm9vYmE="},
    {"foobar",  "Zm9vYmFy"}
};

TEST_CASE("Simple Base64 round-trip tests", "[base64]")
{
    for (const auto &[word, result] : test_data)
    {
        brace::Base64   coder;

        // Test the encode function template using data pointers.
        REQUIRE(coder.encode((uint8_t *)word.data(), (uint8_t *)word.data() + word.size()) == result);

        // Test decoding what we encoded.
        std::vector<uint8_t> v{word.begin(), word.end()};
        REQUIRE(coder.decode(result) == v);

        // Test the encode function template using vector iterators.
        REQUIRE(coder.encode(v.begin(), v.end()) == result);
    }
}

TEST_CASE("Test encoding from binary stream", "[base64]")
{
    for (const auto &[word, result] : test_data)
    {
        brace::BinIArrayStream  stream((uint8_t *)word.data(), (uint8_t *)word.data() + word.size());
        REQUIRE(brace::Base64().encode(stream) == result);
    }
}

TEST_CASE("Test encoding from binary stream to standard stream", "[base64]")
{
    for (const auto &[word, result] : test_data)
    {
        // Create a memory stream containing the word under test.
        brace::BinIArrayStream  instream((uint8_t *)word.data(), (uint8_t *)word.data() + word.size());
        std::stringstream       encstream;

        // Encode from the memory stream to the encoded stream.
        auto chars_written{brace::Base64().encode(instream, encstream)};
        REQUIRE(chars_written % 4 == 0);
        REQUIRE(encstream.str() == result);

        encstream.seekg(0);

        // Prepare an in-memory binary stream to decode into
        size_t  sz = word.size();
        std::vector<uint8_t>    vec(sz);
        brace::BinOArrayStream  outstream{vec.data(), vec.data() + vec.size()};

        // Decode from the encoded-data stream to the binary output stream
        sz = brace::Base64().decode(encstream, outstream);
        REQUIRE(sz == word.size());
        std::string decoded_word{vec.begin(), vec.end()};
        REQUIRE(decoded_word == word);

        encstream.seekg(0);

        std::vector<uint8_t>    dc_vec{brace::Base64().decode(encstream)};
        decoded_word = std::string{dc_vec.begin(), dc_vec.end()};
        REQUIRE(decoded_word == word);
    }
}

