#include "catch.hpp"

#include <ginseng/ginseng.hpp>

#include <array>
#include <memory>

using DB = Ginseng::Database<>;
using Ginseng::Not;
using EntID = DB::EntID;
using ComID = DB::ComID;

TEST_CASE("Entities can be added and removed from a database", "[ginseng]")
{
    DB db;
    REQUIRE(db.size() == 0);

    EntID ent1 = db.makeEntity();
    REQUIRE(db.size() == 1);

    EntID ent2 = db.makeEntity();
    REQUIRE(db.size() == 2);

    db.eraseEntity(ent2);
    REQUIRE(db.size() == 1);

    db.eraseEntity(ent1);
    REQUIRE(db.size() == 0);
}

TEST_CASE("Components can be added, accessed, and removed from entities", "[ginseng]")
{
    DB db;
    auto ent = db.makeEntity();

    struct ComA {
        int x;
    };
    struct ComB {
        double y;
    };

    auto com1 = db.makeComponent(ent, ComA{7});
    REQUIRE(bool(com1) == true);
    ComA *com1ptr1 = &com1.data();
    REQUIRE(com1ptr1 != nullptr);

    auto com1info = ent.get<ComA>();
    REQUIRE(bool(com1info) == true);
    ComA *com1ptr2 = &com1info.data();
    REQUIRE(com1ptr1 == com1ptr2);


    auto com2 = db.makeComponent(ent, ComB{4.2});
    REQUIRE(bool(com2) == true);
    ComB *com2ptr1 = &com2.data();
    REQUIRE(com2ptr1 != nullptr);

    auto com2info = ent.get<ComB>();
    REQUIRE(bool(com2info) == true);
    ComB *com2ptr2 = &com2info.data();
    REQUIRE(com2ptr1 == com2ptr2);

    REQUIRE(&ent.get<ComA>().data() == com1ptr1);
    REQUIRE(ent.get<ComA>().data().x == 7);

    REQUIRE(&ent.get<ComB>().data() == com2ptr1);
    REQUIRE(ent.get<ComB>().data().y == 4.2);

    db.eraseComponent(ent.get<ComA>().id());

    REQUIRE(&ent.get<ComB>().data() == com2ptr1);
    REQUIRE(ent.get<ComB>().data().y == 4.2);

    db.eraseComponent(ent.get<ComB>().id());
}

TEST_CASE("Databases can visit entities with specific components", "[ginseng]")
{
    DB db;

    struct ID { int id; };
    struct Data1 { double val; };
    struct Data2 { std::unique_ptr<int> no_move; };

    int next_id = 0;

    auto make_ent = [&](bool give_Data1, bool give_Data2)
    {
        auto ent = db.makeEntity();
        auto id_com = db.makeComponent(ent, ID{next_id});
        ++next_id;
        if (give_Data1) { db.makeComponent(ent, Data1{7}); }
        if (give_Data2) { db.makeComponent(ent, Data2{nullptr}); }
        return ent;
    };

    make_ent(false, false);
    make_ent(true, false);
    make_ent(true, false);
    make_ent(false, true);
    make_ent(false, true);
    make_ent(false, true);
    make_ent(true, true);
    make_ent(true, true);
    make_ent(true, true);
    make_ent(true, true);

    REQUIRE(next_id == 10);
    REQUIRE(db.size() == next_id);

    std::array<int,10> visited;
    std::array<int,10> expected_visited;

    visited = {};
    expected_visited = {{1,1,1,1,1,1,1,1,1,1}};
    db.visit([&](ID& id){
        ++visited[id.id];
    });
    REQUIRE(visited == expected_visited);

    visited = {};
    expected_visited = {{0,1,1,0,0,0,1,1,1,1}};
    db.visit([&](ID& id, Data1&){
        ++visited[id.id];
    });
    REQUIRE(visited == expected_visited);

    visited = {};
    expected_visited = {{0,0,0,1,1,1,1,1,1,1}};
    db.visit([&](ID& id, Data2&){
        ++visited[id.id];
    });
    REQUIRE(visited == expected_visited);

    visited = {};
    expected_visited = {{0,0,0,0,0,0,1,1,1,1}};
    db.visit([&](ID& id, Data1&, Data2&){
        ++visited[id.id];
    });
    REQUIRE(visited == expected_visited);

    visited = {};
    expected_visited = {{1,0,0,1,1,1,0,0,0,0}};
    db.visit([&](ID& id, Not<Data1>){
        ++visited[id.id];
    });
    REQUIRE(visited == expected_visited);

    visited = {};
    expected_visited = {{0,0,0,1,1,1,0,0,0,0}};
    db.visit([&](ID& id, Not<Data1>, Data2&){
        ++visited[id.id];
    });
    REQUIRE(visited == expected_visited);

    visited = {};
    expected_visited = {{0,1,1,0,0,0,0,0,0,0}};
    db.visit([&](ID& id, Data1&, Not<Data2>){
        ++visited[id.id];
    });
    REQUIRE(visited == expected_visited);

    visited = {};
    expected_visited = {{1,0,0,0,0,0,0,0,0,0}};
    db.visit([&](ID& id, Not<Data1>, Not<Data2>){
        ++visited[id.id];
    });
    REQUIRE(visited == expected_visited);

    int num_visited = 0;
    db.visit([&](Not<ID>){
        ++num_visited;
    });
    REQUIRE(num_visited == 0);
}

