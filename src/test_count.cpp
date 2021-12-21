#include <ginseng/ginseng.hpp>

#include "catch.hpp"

using DB = ginseng::database;
using ginseng::deny;
using ginseng::tag;
using ginseng::optional;
using ent_id = DB::ent_id;
using com_id = DB::com_id;

struct ComA {};
struct ComB {};

TEST_CASE("component counts are zero by default", "[ginseng]") {
    DB db;

    REQUIRE(db.count<ComA>() == 0);
    REQUIRE(db.count<ComB>() == 0);
}

TEST_CASE("component counts increase monotonically", "[ginseng]") {
    DB db;

    for (int i = 0; i < 5; ++i) {
        auto e = db.create_entity();
        db.add_component(e, ComA{});
        REQUIRE(db.count<ComA>() == i+1);
    }

    REQUIRE(db.count<ComA>() == 5);
}

TEST_CASE("component counts decrease monotonically", "[ginseng]") {
    DB db;

    for (int i = 0; i < 5; ++i) {
        auto e = db.create_entity();
        db.add_component(e, ComA{});
    }

    REQUIRE(db.count<ComA>() == 5);

    auto x = 5;

    db.visit([&](ent_id eid, const ComA&) {
        db.remove_component<ComA>(eid);
        --x;
        REQUIRE(db.count<ComA>() == x);
    });

    REQUIRE(db.count<ComA>() == 0);
}

TEST_CASE("component counts are independent", "[ginseng]") {
    DB db;

    for (int i = 0; i < 5; ++i) {
        auto e = db.create_entity();
        db.add_component(e, ComA{});
    }

    REQUIRE(db.count<ComA>() == 5);

    for (int i = 0; i < 5; ++i) {
        auto e = db.create_entity();
        db.add_component(e, ComB{});
        REQUIRE(db.count<ComA>() == 5);
        REQUIRE(db.count<ComB>() == i+1);
    }

    REQUIRE(db.count<ComB>() == 5);

    auto x = 5;

    db.visit([&](ent_id eid, const ComA&) {
        if (x%2 == 0) db.remove_component<ComA>(eid);
        --x;
        REQUIRE(db.count<ComB>() == 5);
    });

    REQUIRE(db.count<ComA>() == 3);
    REQUIRE(db.count<ComB>() == 5);
}
