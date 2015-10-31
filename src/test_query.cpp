#include "catch.hpp"

#include <ginseng/ginseng.hpp>

using DB = Ginseng::Database<>;
using Ginseng::Not;
using EntID = DB::EntID;
using ComID = DB::ComID;

TEST_CASE("Query compiles", "[ginseng]")
{
    DB db;

    struct Data1 {};
    struct Data2 {};

    auto make_ent = [&](bool d1, bool d2){
        auto ent = db.makeEntity();
        if (d1) { db.makeComponent(ent,Data1{}); }
        if (d2) { db.makeComponent(ent,Data2{}); }
    };

    make_ent(false, false);
    make_ent(false, true);
    make_ent(false, true);
    make_ent(true, false);
    make_ent(true, false);
    make_ent(true, false);
    make_ent(true, true);
    make_ent(true, true);
    make_ent(true, true);
    make_ent(true, true);

    {
        auto q = db.query<>().size();
        REQUIRE(q == 10);
    }
    {
        auto q = db.query<Data1>().size();
        REQUIRE(q == 7);
    }
    {
        auto q = db.query<Data2>().size();
        REQUIRE(q == 6);
    }
    {
        auto q = db.query<Data1, Data2>().size();
        REQUIRE(q == 4);
    }
    {
        auto q = db.query<Not<Data1>>().size();
        REQUIRE(q == 3);
    }
    {
        auto q = db.query<Not<Data2>>().size();
        REQUIRE(q == 4);
    }
    {
        auto q = db.query<Data1, Not<Data2>>().size();
        REQUIRE(q == 3);
    }
    {
        auto q = db.query<Data2, Not<Data1>>().size();
        REQUIRE(q == 2);
    }
    {
        auto q = db.query<Not<Data1>, Not<Data2>>().size();
        REQUIRE(q == 1);
    }
}