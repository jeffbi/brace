#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"

#include <string>
#include <filesystem>

#include <brace/binastream.h>
#include <brace/binfstream.h>

#include <fstream>

TEST_CASE("array stream is moveable")
{
    unsigned char   b{0x42};
    int             i{0x42};
    unsigned int    ui{0x80000042u};

    unsigned char   array[1024];
    brace::BinArrayStream   str{array, 1024};

    str << b; b = 0;
    str << i; i = 0;
    str << ui; ui = 0;

    str >> b;
    REQUIRE(b == 0x42);

    // Create a new stream with move constructor
    brace::BinArrayStream   newstr{std::move(str)};
    newstr >> i;
    REQUIRE(i == 0x42);

    unsigned char   newerarray[1024];
    brace::BinArrayStream   newerstr(newerarray, 1024);

    // Move-assign to an existing stream
    newerstr = std::move(newstr);

    newerstr >> ui;
    REQUIRE(ui == 0x80000042u);
}

TEST_CASE("file stream is movable")
{
    unsigned char   b{0x42};
    int             i{0x42};
    unsigned int    ui{0x80000042u};

    brace::BinFStream   str{"junk.bin"};

    str << b; b = 0;
    str << i; i = 0;
    str << ui; ui = 0;

    str.seekg(0);
    str >> b;
    REQUIRE(b == 0x42);

    // Create a new stream with move constructor
    brace::BinFStream   newstr{std::move(str)};
    newstr >> i;
    REQUIRE(i == 0x42);

    brace::BinFStream   newerstr;

    // Move-assign to an existing stream
    newerstr = std::move(newstr);

    newerstr >> ui;
    REQUIRE(ui == 0x80000042u);
}

TEST_CASE("Construct file streams with different filename types")
{
    {
        brace::BinIFStream   str("junk.txt");

        unsigned char   b;
        str >> b;
        REQUIRE(b == static_cast<unsigned char>('T'));
    }
    {
        std::string     filename{"junk.txt"};
        brace::BinIFStream   str(filename);

        unsigned char   b;
        str >> b;
        REQUIRE(b == static_cast<unsigned char>('T'));
    }
    {
        std::filesystem::path     filename{"junk.txt"};
        brace::BinIFStream   str(filename);

        unsigned char   b;
        str >> b;
        REQUIRE(b == static_cast<unsigned char>('T'));
    }
}

TEST_CASE("Open file streams with different filename types")
{
    {
        brace::BinIFStream   str;

        str.open("junk.txt");

        unsigned char   b;
        str >> b;
        REQUIRE(b == static_cast<unsigned char>('T'));
    }
    {
        std::string     filename{"junk.txt"};
        brace::BinIFStream   str;

        str.open(filename);

        unsigned char   b;
        str >> b;
        REQUIRE(b == static_cast<unsigned char>('T'));
    }
    {
        std::filesystem::path     filename{"junk.txt"};
        brace::BinIFStream   str;

        str.open(filename);

        unsigned char   b;
        str >> b;
        REQUIRE(b == static_cast<unsigned char>('T'));
    }
}

TEST_CASE("Simple insertion and extraction")
{
    unsigned char       b{0x42};
    short               shrt{0x42};
    unsigned short      ushrt{0x8042};
    int                 integer{0x42};
    unsigned int        uinteger{0x80000042u};
    long                lng{0x42l};
    unsigned long       ulng{0x80000042ul};
    long long           llng{0x42ll};
    unsigned long long  ullng{0x8000000000000042ul};
    float               flt{-2.0f};
    double              dbl{-2.0};
    long double         ldbl{-2.0l};
    void               *ptr{nullptr};
    const char         *cptr = "fred";

    unsigned char           array[1024];
    brace::BinArrayStream   str{array, 1024};

    str << b;
    str << shrt;
    str << ushrt;
    str << integer;
    str << uinteger;
    str << lng;
    str << ulng;
    str << llng;
    str << ullng;
    str << flt;
    str << dbl;
    str << ldbl;
    str << ptr;
    str << cptr;

    b = 0;
    shrt = 0;
    ushrt = 0;
    integer = 0;
    uinteger = 0;
    lng = 0;
    ulng = 0;
    llng = 0;
    ullng = 0;
    flt = 0.0f;
    dbl = 0.0;
    ldbl = 0.0l;

    str.seekg(0);
    str >> b;
    str >> shrt;
    str >> ushrt;
    str >> integer;
    str >> uinteger;
    str >> lng;
    str >> ulng;
    str >> llng;
    str >> ullng;
    str >> flt;
    str >> dbl;
    str >> ldbl;
    str >> ptr;
    str >> cptr;

    REQUIRE(b == 0x42);
    REQUIRE(shrt == 0x42);
    REQUIRE(ushrt == 0x8042);
    REQUIRE(integer == 0x42);
    REQUIRE(uinteger == 0x80000042);
    REQUIRE(lng == 0x42l);
    REQUIRE(ulng == 0x80000042ul);
    REQUIRE(llng == 0x42ll);
    REQUIRE(ullng == 0x8000000000000042ul);
    REQUIRE_THAT(flt, Catch::Matchers::WithinRel(-2.0f));
    REQUIRE_THAT(dbl, Catch::Matchers::WithinRel(-2.0));
    //REQUIRE_THAT(ldbl, Catch::Matchers::WithinRel(-2.0l));
    REQUIRE(ptr == nullptr);
    REQUIRE(cptr == "fred");
}
