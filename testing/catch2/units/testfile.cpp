#define CATCH_CONFIG_MAIN

#include <catch2/catch_test_macros.hpp>
//#include "catch2/catch_all.hpp"
//#include "CompilerThreadJob.h"

SCENARIO("TPlayer", "[object]") {

    GIVEN("TPlayer") {
        int id = 123;

        WHEN("getting player id") {
            THEN("id should be " << id) {
                REQUIRE(id
                        == id);
            }
        }
    }
}