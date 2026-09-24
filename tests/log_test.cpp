#include "raft/log.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Log append and get") {
    raft::Log log;
    log.append({1, "set x=1"});
    log.append({1, "set y=2"});

    REQUIRE(log.size() == 2);
    REQUIRE(log.get(1).command == "set x=1");
    REQUIRE(log.get(1).term == 1);
    REQUIRE(log.get(2).command == "set y=2");
}
