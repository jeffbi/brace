#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"

#include <filesystem>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "brace/base16.h"
#include "brace/base32.h"
#include "brace/base64.h"
#include "brace/binastream.h"
#include "brace/binfstream.h"

std::string test_data64[][2] = {
    {"",            ""},
    {"f",           "Zg=="},
    {"fo",          "Zm8="},
    {"foo",         "Zm9v"},
    {"foo\n",       "Zm9vCg=="},
    {"foob",        "Zm9vYg=="},
    {"foob\xC3\xBE",    "Zm9vYsO+"},
    {"foob\xC3\xBF",    "Zm9vYsO/"},
    {"fooba",       "Zm9vYmE="},
    {"foobar",      "Zm9vYmFy"}
};
std::string test_data64url[][2] = {
    {"",            ""},
    {"f",           "Zg=="},
    {"fo",          "Zm8="},
    {"foo",         "Zm9v"},
    {"foo\n",       "Zm9vCg=="},
    {"foob",        "Zm9vYg=="},
    {"foob\xC3\xBE",    "Zm9vYsO-"},
    {"foob\xC3\xBF",    "Zm9vYsO_"},
    {"fooba",       "Zm9vYmE="},
    {"foobar",      "Zm9vYmFy"}
};

std::string test_data32[][2] = {
    {"",        ""},
    {"f",       "MY======"},
    {"fo",      "MZXQ===="},
    {"foo",     "MZXW6==="},
    {"foo\n",   "MZXW6CQ="},
    {"foob",    "MZXW6YQ="},
    {"foob\xC3\xBE",    "MZXW6YWDXY======"},
    {"foob\xC3\xBF",    "MZXW6YWDX4======"},
    {"fooba",   "MZXW6YTB"},
    {"foobar",  "MZXW6YTBOI======"}
};
std::string test_data32hex[][2] = {
    {"",        ""},
    {"f",       "CO======"},
    {"fo",      "CPNG===="},
    {"foo",     "CPNMU==="},
    {"foo\n",   "CPNMU2G="},
    {"foob",    "CPNMUOG="},
    {"foob\xC3\xBE",    "CPNMUOM3NO======"},
    {"foob\xC3\xBF",    "CPNMUOM3NS======"},
    {"fooba",   "CPNMUOJ1"},
    {"foobar",  "CPNMUOJ1E8======"}
};

std::string test_data16[][2]
{
    {"",        ""},
    {"f",       "66"},
    {"fo",      "666F"},
    {"foo",     "666F6F"},
    {"foob",    "666F6F62"},
    {"fooba",   "666F6F6261"},
    {"foobar",  "666F6F626172"}
};

std::vector<uint8_t> load_bin_file(const std::string &path)
{
    constexpr size_t        size{4 * 1024};
    uint8_t                 buf[size];
    brace::BinIFStream      stream{path};
    std::vector<uint8_t>    rv{};

    while (stream.good())
    {
        stream.read(buf, size);
        if (stream.gcount())
        {
            rv.insert(rv.end(), buf, buf + stream.gcount());
        }
    }

    return rv;
}

template<typename T>
void simple_round_trip(const std::string &word, const std::string &result, const T &coder)
{
    // Test the encode function template using data pointers.
    REQUIRE(coder.encode((uint8_t *)word.data(), (uint8_t *)word.data() + word.size()) == result);

    // Test decoding what we encoded.
    std::vector<uint8_t> v{word.begin(), word.end()};
    REQUIRE(coder.decode(result) == v);

    // Test the encode function template using vector iterators.
    REQUIRE(coder.encode(v.begin(), v.end()) == result);
}

template<typename T>
void test_encoding_from_binary_stream_to_standard_stream(
    const std::string &word, const std::string &result, const T &coder)
{
    // Create a memory stream containing the word under test.
    brace::BinIArrayStream  instream((uint8_t *)word.data(), (uint8_t *)word.data() + word.size());
    std::stringstream       encstream;

    // Encode from the memory stream to the encoded stream.
    auto chars_written{coder.encode(instream, encstream)};
    REQUIRE(encstream.str() == result);

    encstream.clear();
    encstream.seekg(0);

    // Prepare an in-memory binary stream to decode into
    size_t  sz = word.size();
    std::vector<uint8_t>    vec(sz);
    brace::BinOArrayStream  outstream{vec.data(), vec.data() + vec.size()};

    // Decode from the encoded-data stream to the binary output stream
    sz = coder.decode(encstream, outstream);
    REQUIRE(sz == word.size());
    std::string decoded_word{vec.begin(), vec.end()};
    REQUIRE(decoded_word == word);

    encstream.clear();
    encstream.seekg(0);

    // Decode from the encoded-data stream to a vector
    std::vector<uint8_t>    dc_vec{coder.decode(encstream)};
    decoded_word = std::string{dc_vec.begin(), dc_vec.end()};
    REQUIRE(decoded_word == word);
}

template<typename T>
void test_encoding_from_external_file(const T &coder, const std::string &path,
                                      std::string_view expected_head,
                                      std::string_view expected_tail)
{
    // Test loading the file into a vector and encoding from that.
    std::vector<uint8_t>    vec{load_bin_file(path)};
    std::string             enc{coder.encode(vec.begin(), vec.end())};
    std::string             head{enc.substr(0, 16)};
    std::string             tail{enc.substr(enc.size() - 16, 16)};
    REQUIRE(head == expected_head);
    REQUIRE(tail == expected_tail);

    std::vector<uint8_t>    dec{coder.decode(enc)};
    REQUIRE(dec == vec);

    // Test encoding from the file, through a stream
    brace::BinIFStream  stream(path);
    enc = head = tail = "";
    enc = coder.encode(stream);
    head = enc.substr(0, 16);
    tail = enc.substr(enc.size() - 16, 16);
    REQUIRE(head == expected_head);
    REQUIRE(tail == expected_tail);

    dec.clear();
    dec = coder.decode(enc);
    REQUIRE(dec == vec);
}

TEST_CASE("Simple Base64 round-trip tests", "[base64]")
{
    for (const auto &[word, result] : test_data64)
    {
        simple_round_trip(word, result, brace::Base64{});
    }
}
TEST_CASE("Simple Base64Url round-trip tests", "[base64Url]")
{
    for (const auto &[word, result] : test_data64url)
    {
        simple_round_trip(word, result, brace::Base64Url{});
    }
}

TEST_CASE("Test Base64 encoding from binary stream", "[base64]")
{
    for (const auto &[word, result] : test_data64)
    {
        brace::BinIArrayStream  stream((uint8_t *)word.data(), (uint8_t *)word.data() + word.size());
        REQUIRE(brace::Base64().encode(stream) == result);
    }
}
TEST_CASE("Test Base64Url encoding from binary stream", "[base64Url]")
{
    for (const auto &[word, result] : test_data64url)
    {
        brace::BinIArrayStream  stream((uint8_t *)word.data(), (uint8_t *)word.data() + word.size());
        REQUIRE(brace::Base64Url().encode(stream) == result);
    }
}
TEST_CASE("Test Base64 encoding from binary stream to standard stream", "[base64]")
{
    for (const auto &[word, result] : test_data64)
    {
        test_encoding_from_binary_stream_to_standard_stream(word, result, brace::Base64{});
    }
}
TEST_CASE("Test Base64Url encoding from binary stream to standard stream", "[base64Url]")
{
    for (const auto &[word, result] : test_data64url)
    {
        test_encoding_from_binary_stream_to_standard_stream(word, result, brace::Base64Url{});
    }
}
TEST_CASE("Test Base64 encoding from external file", "[base64]")
{
    constexpr const char   *path{"test_data/rfc1321.txt.pdf"};
    constexpr const char   *head{"JVBERi0xLjIKJcfs"};
    constexpr const char   *tail{"OTM0CiUlRU9GCg=="};

    test_encoding_from_external_file(brace::Base64{}, path, head, tail);
}
TEST_CASE("Test Base64Url encoding from external file", "[base64Url]")
{
    constexpr const char   *path{"test_data/rfc1321.txt.pdf"};
    constexpr const char   *head{"JVBERi0xLjIKJcfs"};
    constexpr const char   *tail{"OTM0CiUlRU9GCg=="};

    test_encoding_from_external_file(brace::Base64Url{}, path, head, tail);
}


TEST_CASE("Simple Base32 round-trip tests", "[base32]")
{
    for (const auto &[word, result] : test_data32)
    {
        simple_round_trip(word, result, brace::Base32{});
    }
}
TEST_CASE("Simple Base32Hex round-trip tests", "[base32Hex]")
{
    for (const auto &[word, result] : test_data32hex)
    {
        simple_round_trip(word, result, brace::Base32Hex{});
    }
}

TEST_CASE("Test Base32 encoding from binary stream", "[base32]")
{
    for (const auto &[word, result] : test_data32)
    {
        brace::BinIArrayStream  stream((uint8_t *)word.data(), (uint8_t *)word.data() + word.size());
        REQUIRE(brace::Base32().encode(stream) == result);
    }
}
TEST_CASE("Test Base32Hex encoding from binary stream", "[base32Hex]")
{
    for (const auto &[word, result] : test_data32hex)
    {
        brace::BinIArrayStream  stream((uint8_t *)word.data(), (uint8_t *)word.data() + word.size());
        REQUIRE(brace::Base32Hex().encode(stream) == result);
    }
}
TEST_CASE("Test Base32 encoding from binary stream to standard stream", "[base32]")
{
    for (const auto &[word, result] : test_data32)
    {
        test_encoding_from_binary_stream_to_standard_stream(word, result, brace::Base32{});
    }
}
TEST_CASE("Test Base32Hex encoding from binary stream to standard stream", "[base32Hex]")
{
    for (const auto &[word, result] : test_data32hex)
    {
        test_encoding_from_binary_stream_to_standard_stream(word, result, brace::Base32Hex{});
    }
}
TEST_CASE("Test Base32 encoding from external file", "[base32]")
{
    constexpr const char   *path{"test_data/rfc1321.txt.pdf"};
    constexpr const char   *head{"EVIEIRRNGEXDECRF"};
    constexpr const char   *tail{"GM2AUJJFIVHUMCQ="};

    test_encoding_from_external_file(brace::Base32{}, path, head, tail);
}
TEST_CASE("Test Base32Hex encoding from external file", "[base32Hex]")
{
    constexpr const char   *path{"test_data/rfc1321.txt.pdf"};
    constexpr const char   *head{"4L848HHD64N342H5"};
    constexpr const char   *tail{"6CQ0K9958L7KC2G="};

    test_encoding_from_external_file(brace::Base32Hex{}, path, head, tail);
}

TEST_CASE("Simple Base16 round-trip tests", "[base16]")
{
    for (const auto &[word, result] : test_data16)
    {
        simple_round_trip(word, result, brace::Base16{});
    }
}
TEST_CASE("Test Base16 encoding from binary stream", "[base16]")
{
    for (const auto &[word, result] : test_data16)
    {
        brace::BinIArrayStream  stream((uint8_t *)word.data(), (uint8_t *)word.data() + word.size());
        REQUIRE(brace::Base16().encode(stream) == result);
    }
}
TEST_CASE("Test Base16 encoding from binary stream to standard stream", "[base16]")
{
    for (const auto &[word, result] : test_data16)
    {
        test_encoding_from_binary_stream_to_standard_stream(word, result, brace::Base16{});
    }
}
TEST_CASE("Test Base16 encoding from external file", "[base16]")
{
    constexpr const char   *path{"test_data/rfc1321.txt.pdf"};
    constexpr const char   *head{"255044462D312E32"};
    constexpr const char   *tail{"340A2525454F460A"};

    test_encoding_from_external_file(brace::Base16{}, path, head, tail);
}
