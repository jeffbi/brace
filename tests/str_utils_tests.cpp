#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"

#include <string>

#include "brace/string.h"

TEST_CASE("Test to_lower")
{
    {
        // test on strings
        std::string dog{"I'm a dog."};
        std::string louder_dog{brace::to_upper(dog)};
        REQUIRE(louder_dog == "I'M A DOG.");

        std::string quieter_dog{brace::to_lower(louder_dog)};
        REQUIRE(quieter_dog == "i'm a dog.");

        REQUIRE(brace::ci_compare(quieter_dog, louder_dog) == 0);
    }
    {
        // test on U32strings
        std::u32string dog{U"I'm a dog."};
        std::u32string louder_dog{brace::to_upper(dog)};
        REQUIRE(louder_dog == U"I'M A DOG.");

        std::u32string quieter_dog{brace::to_lower(louder_dog)};
        REQUIRE(quieter_dog == U"i'm a dog.");

        REQUIRE(brace::ci_compare(quieter_dog, louder_dog) == 0);
    }
}
