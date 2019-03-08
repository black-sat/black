#define CATCH_CONFIG_MAIN

#include <catch.hpp>

#include <cstdint>

SCENARIO("Incrementing a variable")
{
  GIVEN("An int variable initialized to zero") {
    int i = 0;

    WHEN("The variable is incremented") {
      ++i;
      THEN("The variable equals 1") {
        REQUIRE(i == 1);
      }

      AND_WHEN("The variable is decremented") {
        --i;

        THEN("The variable equals zero again") {
          REQUIRE(i == 0);
        }
      }
    }
  }
}
