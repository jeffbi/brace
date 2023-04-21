#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"

#include <filesystem>
#include <fstream>
#include <memory>
#include <string>

#include "brace/sha1.h"
#include "brace/sha2.h"
#include "brace/md5.h"

#include "brace/binfstream.h"
#include "brace/binastream.h"

TEST_CASE("Basic sha1 from standard stream (binary mode)")
{
    std::ifstream   stream("test_data/rfc1321.txt.pdf", std::ios_base::in | std::ios_base::binary);
    brace::SHA1     hasher;
    std::string     str{hasher.compute_hash_string(stream)};

    REQUIRE(str == "907D6DD9956C36322B0231E991C621DA79668664");
}

TEST_CASE("Basic sha1 from brace binary stream")
{
    brace::BinIFStream  stream("test_data/rfc1321.txt.pdf");
    brace::SHA1         hasher;
    std::string         str{hasher.compute_hash_string(stream)};

    REQUIRE(str == "907D6DD9956C36322B0231E991C621DA79668664");
}

TEST_CASE("Basic sha224 from standard stream (binary mode)")
{
    std::ifstream   stream("test_data/rfc1321.txt.pdf", std::ios_base::in | std::ios_base::binary);
    brace::SHA224   hasher;
    std::string     str{hasher.compute_hash_string(stream)};

    REQUIRE(str == "9ED8D14878E7A78D8CEA7D40DA3DB16EA409ED6D4BD75351AD761064");
}

TEST_CASE("Basic sha224 from brace binary stream")
{
    brace::BinIFStream  stream("test_data/rfc1321.txt.pdf");
    brace::SHA224       hasher;
    std::string         str{hasher.compute_hash_string(stream)};

    REQUIRE(str == "9ED8D14878E7A78D8CEA7D40DA3DB16EA409ED6D4BD75351AD761064");
}

TEST_CASE("Basic sha256 from standard stream (binary mode)")
{
    std::ifstream   stream("test_data/rfc1321.txt.pdf", std::ios_base::in | std::ios_base::binary);
    brace::SHA256   hasher;
    std::string     str{hasher.compute_hash_string(stream)};

    REQUIRE(str == "ABAB9EEE3A7028306EED3FE5CFAB1DC0B2B16DA52AA2666DAA3385C4806734DD");
}

TEST_CASE("Basic sha256 from brace binary stream")
{
    brace::BinIFStream  stream("test_data/rfc1321.txt.pdf");
    brace::SHA256       hasher;
    std::string         str{hasher.compute_hash_string(stream)};

    REQUIRE(str == "ABAB9EEE3A7028306EED3FE5CFAB1DC0B2B16DA52AA2666DAA3385C4806734DD");
}

TEST_CASE("Basic sha384 from standard stream (binary mode)")
{
    std::ifstream   stream("test_data/rfc1321.txt.pdf", std::ios_base::in | std::ios_base::binary);
    brace::SHA384   hasher;
    std::string     str{hasher.compute_hash_string(stream)};

    REQUIRE(str == "31481EC9368F6A48BEFF548A958AD0D2AD285FCE434A64EBFC1F3CA54A5FBF2445FEE587B1F943721D9C59AC247BC4AE");
}

TEST_CASE("Basic sha384 from brace binary stream")
{
    brace::BinIFStream  stream("test_data/rfc1321.txt.pdf");
    brace::SHA384       hasher;
    std::string         str{hasher.compute_hash_string(stream)};

    REQUIRE(str == "31481EC9368F6A48BEFF548A958AD0D2AD285FCE434A64EBFC1F3CA54A5FBF2445FEE587B1F943721D9C59AC247BC4AE");
}

TEST_CASE("Basic sha512 from standard stream (binary mode)")
{
    std::ifstream   stream("test_data/rfc1321.txt.pdf", std::ios_base::in | std::ios_base::binary);
    brace::SHA512   hasher;
    std::string     str{hasher.compute_hash_string(stream)};

    REQUIRE(str == "C10B324542AAE00F2100489153ABAC2B272C0EF4AD92B15D5ED99221C1D996B000288941FCFC8805EDC127BE73B2A0EF0ACEB698F0C909794731F890EED7E5C6");
}

TEST_CASE("Basic sha512 from brace binary stream")
{
    brace::BinIFStream  stream("test_data/rfc1321.txt.pdf");
    brace::SHA512       hasher;
    std::string         str{hasher.compute_hash_string(stream)};

    REQUIRE(str == "C10B324542AAE00F2100489153ABAC2B272C0EF4AD92B15D5ED99221C1D996B000288941FCFC8805EDC127BE73B2A0EF0ACEB698F0C909794731F890EED7E5C6");
}

TEST_CASE("Basic md5 from standard stream (binary mode)")
{
    std::ifstream   stream("test_data/rfc1321.txt.pdf", std::ios_base::in | std::ios_base::binary);
    brace::MD5      hasher;
    std::string     str{hasher.compute_hash_string(stream)};

    REQUIRE(str == "D26422E528EE388C001F5E8D4498963F");
}

TEST_CASE("Basic md5 from brace binary stream")
{
    brace::BinIFStream  stream("test_data/rfc1321.txt.pdf");
    brace::MD5          hasher;
    std::string         str{hasher.compute_hash_string(stream)};

    REQUIRE(str == "D26422E528EE388C001F5E8D4498963F");
}

TEST_CASE("Basic md5 from brace binary array stream")
{
    std::filesystem::path   path{"test_data/rfc1321.txt.pdf"};
    auto                    size{std::filesystem::file_size(path)};
    auto                    buffer = std::make_unique<uint8_t[]>(size);
    {
        brace::BinIFStream      fstream(path);
        fstream.read(buffer.get(), size);
    }

    brace::BinArrayStream   stream{buffer.get(), size};
    brace::MD5              hasher;
    std::string             str{hasher.compute_hash_string(stream)};

    REQUIRE(str == "D26422E528EE388C001F5E8D4498963F");
}
