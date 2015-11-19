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
        std::vector<std::tuple<>> q = db.query<>();
        REQUIRE(q.size() == 10);
    }
    {
        std::vector<std::tuple<Data1>> q = db.query<Data1>();
        REQUIRE(q.size() == 7);
    }
    {
        std::vector<std::tuple<Data2&>> q = db.query<Data2&>();
        REQUIRE(q.size() == 6);
    }
    {
        std::vector<std::tuple<Data1&,Data2&>> q = db.query<Data1&, Data2&>();
        REQUIRE(q.size() == 4);
    }
    {
        std::vector<std::tuple<Not<Data1>>> q = db.query<Not<Data1>>();
        REQUIRE(q.size() == 3);
    }
    {
        std::vector<std::tuple<Not<Data2>>> q = db.query<Not<Data2>>();
        REQUIRE(q.size() == 4);
    }
    {
        std::vector<std::tuple<DB::ComInfo<Data1>,Not<Data2>>> q = db.query<DB::ComInfo<Data1>, Not<Data2>>();
        REQUIRE(q.size() == 3);
    }
    {
        std::vector<std::tuple<DB::ComInfo<Data2>,Not<Data1>>> q = db.query<DB::ComInfo<Data2>, Not<Data1>>();
        REQUIRE(q.size() == 2);
    }
    {
        std::vector<std::tuple<Not<Data1>,Not<Data2>>> q = db.query<Not<Data1>, Not<Data2>>();
        REQUIRE(q.size() == 1);
    }
}