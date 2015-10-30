#include "catch.hpp"

#include <ginseng/ginseng.hpp>

#include <array>
#include <memory>

using DB = Ginseng::Database<>;
using Ginseng::Not;
using EntID = DB::EntID;
using ComID = DB::ComID;

TEST_CASE("Entities can be emplaced between databases", "[ginseng]")
{
    DB db1;
    DB db2;

    struct Data { int val = 7; };

    REQUIRE(db1.size() == 0);
    REQUIRE(db2.size() == 0);

    auto ent = db1.makeEntity();
    auto data = db1.makeComponent(ent,Data{});
    Data* data_ptr = &data.data();

    REQUIRE(data_ptr != nullptr);

    REQUIRE(db1.size() == 1);
    REQUIRE(db2.size() == 0);

    auto tmp = db1.displaceEntity(ent);

    REQUIRE(db1.size() == 0);
    REQUIRE(db2.size() == 0);

    auto ent2 = db2.emplaceEntity(std::move(tmp));

    REQUIRE(db1.size() == 0);
    REQUIRE(db2.size() == 1);

    auto data2 = ent2.get<Data>();
    REQUIRE(&data2.data() == data_ptr);

    db2.eraseEntity(ent2);

    REQUIRE(db1.size() == 0);
    REQUIRE(db2.size() == 0);
}

TEST_CASE("Components can be emplaced between entities", "[ginseng]")
{
    DB db1;
    DB db2;

    struct Data { int val = 7; };

    auto ent1 = db1.makeEntity();
    auto ent2 = db2.makeEntity();

    REQUIRE(bool(ent1.get<Data>()) == false);
    REQUIRE(bool(ent2.get<Data>()) == false);

    auto cominfo1 = db1.makeComponent(ent1, Data{});
    Data* data_ptr = &cominfo1.data();

    REQUIRE(data_ptr != nullptr);

    REQUIRE(bool(ent1.get<Data>()) == true);
    REQUIRE(bool(ent2.get<Data>()) == false);

    auto com = db1.displaceComponent(cominfo1.id());

    REQUIRE(bool(ent1.get<Data>()) == false);
    REQUIRE(bool(ent2.get<Data>()) == false);

    auto cominfo2 = db2.emplaceComponent(ent2, std::move(com));

    REQUIRE(&cominfo2.cast<Data>() == data_ptr);

    REQUIRE(bool(ent1.get<Data>()) == false);
    REQUIRE(bool(ent2.get<Data>()) == true);

    db2.eraseComponent(cominfo2);

    REQUIRE(bool(ent1.get<Data>()) == false);
    REQUIRE(bool(ent2.get<Data>()) == false);
}
