#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"

#include <brace/ascii_encoding.h>
#include <brace/utf8_encoding.h>
#include <brace/utf32_encoding.h>

TEST_CASE("Simple ASCII tests")
{
    brace::ASCIIEncoding    enc;

    REQUIRE(enc.aka("Fred") == false);
    REQUIRE(enc.aka("ASCII") == true);
}

TEST_CASE("Test encode UTF-8")
{
    constexpr const int cp_dollar{0x24};
    constexpr const int cp_brit_pound{0x00A3};
    constexpr const int cp_cyrillic_I{0x0418};
    constexpr const int cp_ha{0x0939};
    constexpr const int cp_euro{0x20AC};
    constexpr const int cp_hangul{0xD55C};
    constexpr const int cp_hwair{0x10348};
    constexpr const unsigned char dollar[] {0x24};
    constexpr const unsigned char brit_pound[] {0xC2, 0xA3};
    constexpr const unsigned char cyrillic_I[] {0xD0, 0x98};
    constexpr const unsigned char ha[] {0xE0, 0xA4, 0xB9};
    constexpr const unsigned char euro[] {0xE2, 0x82, 0xAC};
    constexpr const unsigned char hangul[] {0xED, 0x95, 0x9C};
    constexpr const unsigned char hwair[] {0xF0, 0x90, 0x8D, 0x88};

    brace::UTF8Encoding     enc;
    unsigned char           bytes[6];
    int                     n;

    n = enc.encode(cp_dollar, bytes, 6);
    REQUIRE(n == 1);
    REQUIRE(memcmp(bytes, dollar, n) == 0);
    n = enc.encode(cp_brit_pound, bytes, 6);
    REQUIRE(n == 2);
    REQUIRE(memcmp(bytes, brit_pound, n) == 0);
    n = enc.encode(cp_cyrillic_I, bytes, 6);
    REQUIRE(n == 2);
    REQUIRE(memcmp(bytes, cyrillic_I, n) == 0);
    n = enc.encode(cp_ha, bytes, 6);
    REQUIRE(n == 3);
    REQUIRE(memcmp(bytes, ha, n) == 0);
    n = enc.encode(cp_euro, bytes, 6);
    REQUIRE(n == 3);
    REQUIRE(memcmp(bytes, euro, n) == 0);
    n = enc.encode(cp_hangul, bytes, 6);
    REQUIRE(n == 3);
    REQUIRE(memcmp(bytes, hangul, n) == 0);
    n = enc.encode(cp_hwair, bytes, 6);
    REQUIRE(n == 4);
    REQUIRE(memcmp(bytes, hwair, n) == 0);
}
TEST_CASE("Test decode UTF-8")
{
    constexpr const unsigned char dollar[] {0x24};
    constexpr const unsigned char brit_pound[] {0xC2, 0xA3};
    constexpr const unsigned char cyrillic_I[] {0xD0, 0x98};
    constexpr const unsigned char ha[] {0xE0, 0xA4, 0xB9};
    constexpr const unsigned char euro[] {0xE2, 0x82, 0xAC};
    constexpr const unsigned char hangul[] {0xED, 0x95, 0x9C};
    constexpr const unsigned char hwair[] {0xF0, 0x90, 0x8D, 0x88};

    brace::UTF8Encoding     enc;

    int     cp;

    cp = enc.decode(dollar);
    REQUIRE(cp == 0x24);
    cp = enc.decode(brit_pound);
    REQUIRE(cp == 0x00A3);
    cp = enc.decode(cyrillic_I);
    REQUIRE(cp == 0x0418);
    cp = enc.decode(ha);
    REQUIRE(cp == 0x0939);
    cp = enc.decode(euro);
    REQUIRE(cp == 0x20AC);
    cp = enc.decode(hangul);
    REQUIRE(cp == 0xD55C);
    cp = enc.decode(hwair);
    REQUIRE(cp == 0x10348);
}

TEST_CASE("Encode UTF-32")
{
    constexpr const int cp_dollar{0x24};
    constexpr const int cp_brit_pound{0x00A3};
    constexpr const int cp_cyrillic_I{0x0418};
    constexpr const int cp_ha{0x0939};
    constexpr const int cp_euro{0x20AC};
    constexpr const int cp_hangul{0xD55C};
    constexpr const int cp_hwair{0x10348};

    brace::UTF32Encoding    enc;
    uint32_t    cp;
    int         n;

    n = enc.encode(cp_dollar, reinterpret_cast<unsigned char *>(&cp), 4);
    REQUIRE(n == 4);
    REQUIRE(cp == cp_dollar);
    n = enc.encode(cp_brit_pound, reinterpret_cast<unsigned char *>(&cp), 4);
    REQUIRE(n == 4);
    REQUIRE(cp == cp_brit_pound);
    n = enc.encode(cp_cyrillic_I, reinterpret_cast<unsigned char *>(&cp), 4);
    REQUIRE(n == 4);
    REQUIRE(cp == cp_cyrillic_I);
    n = enc.encode(cp_ha, reinterpret_cast<unsigned char *>(&cp), 4);
    REQUIRE(n == 4);
    REQUIRE(cp == cp_ha);
    n = enc.encode(cp_euro, reinterpret_cast<unsigned char *>(&cp), 4);
    REQUIRE(n == 4);
    REQUIRE(cp == cp_euro);
    n = enc.encode(cp_hangul, reinterpret_cast<unsigned char *>(&cp), 4);
    REQUIRE(n == 4);
    REQUIRE(cp == cp_hangul);
    n = enc.encode(cp_hwair, reinterpret_cast<unsigned char *>(&cp), 4);
    REQUIRE(n == 4);
    REQUIRE(cp == cp_hwair);
}
TEST_CASE("Decode UTF-32")
{
    constexpr const unsigned char be_dollar[]{0x00, 0x00, 0x00, 0x24};
    constexpr const unsigned char be_brit_pound[]{0x00, 0x00, 0x00, 0xA3};
    constexpr const unsigned char be_cyrillic_I[]{0x00, 0x00, 0x04, 0x18};
    constexpr const unsigned char be_ha[]{0x00, 0x00, 0x09, 0x39};
    constexpr const unsigned char be_euro[]{0x00, 0x00, 0x20, 0xAC};
    constexpr const unsigned char be_hangul[]{0x00, 0x00, 0xD5, 0x5C};
    constexpr const unsigned char be_hwair[]{0x00, 0x01, 0x03, 0x48};

    constexpr const unsigned char le_dollar[]{0x24, 0x00, 0x00, 0x00};
    constexpr const unsigned char le_brit_pound[]{0xA3, 0x00, 0x00, 0x00};
    constexpr const unsigned char le_cyrillic_I[]{0x18, 0x04, 0x00, 0x00};
    constexpr const unsigned char le_ha[]{0x39, 0x09, 0x00, 0x00};
    constexpr const unsigned char le_euro[]{0xAC, 0x20, 0x00, 0x00};
    constexpr const unsigned char le_hangul[]{0x5C,0xD5,  0x00, 0x00};
    constexpr const unsigned char le_hwair[]{0x48, 0x03, 0x01, 0x00};

    constexpr const int cp_dollar{0x24};
    constexpr const int cp_brit_pound{0x00A3};
    constexpr const int cp_cyrillic_I{0x0418};
    constexpr const int cp_ha{0x0939};
    constexpr const int cp_euro{0x20AC};
    constexpr const int cp_hangul{0xD55C};
    constexpr const int cp_hwair{0x10348};

    brace::UTF32Encoding    enc{brace::Endian::Big};
    brace::UTF32Encoding    enc_le(brace::Endian::Little);
    int     cp;

    cp = enc.decode(be_dollar);
    REQUIRE(cp == cp_dollar);
    cp = enc.decode(be_brit_pound);
    REQUIRE(cp == cp_brit_pound);
    cp = enc.decode(be_cyrillic_I);
    REQUIRE(cp == cp_cyrillic_I);
    cp = enc.decode(be_ha);
    REQUIRE(cp == cp_ha);
    cp = enc.decode(be_euro);
    REQUIRE(cp == cp_euro);
    cp = enc.decode(be_hangul);
    REQUIRE(cp == cp_hangul);
    cp = enc.decode(be_hwair);
    REQUIRE(cp == cp_hwair);

    cp = enc_le.decode(le_dollar);
    REQUIRE(cp == cp_dollar);
    cp = enc_le.decode(le_brit_pound);
    REQUIRE(cp == cp_brit_pound);
    cp = enc_le.decode(le_cyrillic_I);
    REQUIRE(cp == cp_cyrillic_I);
    cp = enc_le.decode(le_ha);
    REQUIRE(cp == cp_ha);
    cp = enc_le.decode(le_euro);
    REQUIRE(cp == cp_euro);
    cp = enc_le.decode(le_hangul);
    REQUIRE(cp == cp_hangul);
    cp = enc_le.decode(le_hwair);
    REQUIRE(cp == cp_hwair);
}
