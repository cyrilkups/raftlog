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

TEST_CASE("Log cut_off_from discards entries from index onward") {
    raft::Log log;
    log.append({1, "set x=1"});
    log.append({1, "set y=2"});
    log.append({2, "set z=3"});

    log.cut_off_from(2);

    REQUIRE(log.size() == 1);
    REQUIRE(log.get(1).command == "set x=1");
}

TEST_CASE("Log cut_off_from with out-of-range index is a no-op") {
    raft::Log log;
    log.append({1, "set x=1"});

    log.cut_off_from(5);
    REQUIRE(log.size() == 1);

    log.cut_off_from(0);
    REQUIRE(log.size() == 1);
}
