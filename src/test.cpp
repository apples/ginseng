#include <ginseng/ginseng.hpp>

#include <array>
#include <memory>

#include "catch.hpp"

using DB = ginseng::database;
using ginseng::deny;
using ginseng::optional;
using ent_id = DB::ent_id;
using com_id = DB::com_id;

TEST_CASE("Entities can be added and removed from a database", "[ginseng]")
{
    DB db;
    REQUIRE(db.size() == 0);

    ent_id ent1 = db.create_entity();
    REQUIRE(db.size() == 1);

    ent_id ent2 = db.create_entity();
    REQUIRE(db.size() == 2);

    db.destroy_entity(ent2);
    REQUIRE(db.size() == 1);

    db.destroy_entity(ent1);
    REQUIRE(db.size() == 0);
}

TEST_CASE("Components can be added, accessed, and removed from entities", "[ginseng]")
{
    DB db;
    auto ent = db.create_entity();

    struct ComA {
        int x;
    };
    struct ComB {
        double y;
    };

    db.add_component(ent, ComA{7});
    REQUIRE(db.has_component<ComA>(ent) == true);
    ComA *com1ptr1 = &db.get_component<ComA>(ent);
    REQUIRE(com1ptr1 != nullptr);

    db.add_component(ent, ComB{4.2});
    REQUIRE(db.has_component<ComB>(ent) == true);
    ComB *com2ptr1 = &db.get_component<ComB>(ent);
    REQUIRE(com2ptr1 != nullptr);

    REQUIRE(&db.get_component<ComA>(ent) == com1ptr1);
    REQUIRE(db.get_component<ComA>(ent).x == 7);

    REQUIRE(&db.get_component<ComB>(ent) == com2ptr1);
    REQUIRE(db.get_component<ComB>(ent).y == 4.2);

    db.remove_component<ComA>(ent);
    REQUIRE(db.has_component<ComA>(ent) == false);
    REQUIRE(db.has_component<ComB>(ent) == true);

    REQUIRE(&db.get_component<ComB>(ent) == com2ptr1);
    REQUIRE(db.get_component<ComB>(ent).y == 4.2);

    db.remove_component<ComB>(ent);
    REQUIRE(db.has_component<ComA>(ent) == false);
    REQUIRE(db.has_component<ComB>(ent) == false);
}

TEST_CASE("get_optional works", "[ginseng]")
{
    DB db;
    auto ent = db.create_entity();

    struct ComA {
        int x;
    };

    db.add_component(ent, ComA{2});

    REQUIRE(db.get_component<ComA*>(ent) == &db.get_component<ComA>(ent));

    db.remove_component<ComA>(ent);

    REQUIRE(db.get_component<ComA*>(ent) == nullptr);
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
        auto ent = db.create_entity();
        db.add_component(ent, ID{next_id});
        ++next_id;
        if (give_Data1) { db.add_component(ent, Data1{7}); }
        if (give_Data2) { db.add_component(ent, Data2{nullptr}); }
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
    db.visit([&](ID& id, deny<Data1>){
        ++visited[id.id];
    });
    REQUIRE(visited == expected_visited);

    visited = {};
    expected_visited = {{0,0,0,1,1,1,0,0,0,0}};
    db.visit([&](ID& id, deny<Data1>, Data2&){
        ++visited[id.id];
    });
    REQUIRE(visited == expected_visited);

    visited = {};
    expected_visited = {{0,1,1,0,0,0,0,0,0,0}};
    db.visit([&](ID& id, Data1&, deny<Data2>){
        ++visited[id.id];
    });
    REQUIRE(visited == expected_visited);

    visited = {};
    expected_visited = {{1,0,0,0,0,0,0,0,0,0}};
    db.visit([&](ID& id, deny<Data1>, deny<Data2>){
        ++visited[id.id];
    });
    REQUIRE(visited == expected_visited);

    int num_visited = 0;
    db.visit([&](deny<ID>){
        ++num_visited;
    });
    REQUIRE(num_visited == 0);
}

TEST_CASE("ent_id matches all entities and is the entity's id", "[ginseng]")
{
    DB db;

    db.create_entity();
    db.create_entity();
    db.create_entity();

    int visited[] = {0, 0, 0};

    db.visit([&](DB::ent_id eid){
        ++visited[eid.get_index()];
    });

    REQUIRE(visited[0] == 1);
    REQUIRE(visited[1] == 1);
    REQUIRE(visited[2] == 1);
}

TEST_CASE("optional can be used instead of components", "[ginseng]")
{
    DB db;

    struct Data {};
    struct Data2 {};

    auto ent = db.create_entity();
    db.add_component(ent,Data{});

    int visited = 0;
    optional<Data> mdata;
    optional<Data2> mdata2;
    db.visit([&](optional<Data> data, optional<Data2> data2){
        ++visited;
        mdata = data;
        mdata2 = data2;
    });
    REQUIRE(visited == 1);
    REQUIRE(bool(mdata) == true);
    REQUIRE(bool(mdata2) == false);
}

TEST_CASE("deleted entites are not revisited", "[ginseng]")
{
    DB db;

    struct Data {};

    auto ent = db.create_entity();
    db.create_entity();
    db.create_entity();

    int visited = 0;
    db.visit([&](ent_id eid){
        ++visited;
        db.add_component(eid, Data{});
    });
    REQUIRE(visited == 3);

    visited = 0;
    db.visit([&](ent_id, Data&){
        ++visited;
    });
    REQUIRE(visited == 3);

    db.destroy_entity(ent);

    visited = 0;
    db.visit([&](ent_id){
        ++visited;
    });
    REQUIRE(visited == 2);

    visited = 0;
    db.visit([&](ent_id, Data&){
        ++visited;
    });
    REQUIRE(visited == 2);
}

TEST_CASE("versioning works", "[ginseng]")
{
    DB db;

    auto ent1 = db.create_entity();
    auto ent2 = db.create_entity();
    auto ent3 = db.create_entity();

    REQUIRE(db.exists(ent1));
    REQUIRE(db.exists(ent2));
    REQUIRE(db.exists(ent3));

    db.destroy_entity(ent2);

    REQUIRE(db.exists(ent1));
    REQUIRE(!db.exists(ent2));
    REQUIRE(db.exists(ent3));

    db.destroy_entity(ent1);

    REQUIRE(!db.exists(ent1));
    REQUIRE(!db.exists(ent2));
    REQUIRE(db.exists(ent3));

    db.destroy_entity(ent3);

    REQUIRE(!db.exists(ent1));
    REQUIRE(!db.exists(ent2));
    REQUIRE(!db.exists(ent3));

    auto ent4 = db.create_entity();

    REQUIRE(!(ent4 == ent1));
    REQUIRE(!(ent4 == ent2));
    REQUIRE(!(ent4 == ent3));
    REQUIRE(!db.exists(ent1));
    REQUIRE(!db.exists(ent2));
    REQUIRE(!db.exists(ent3));
    REQUIRE(db.exists(ent4));
}

TEST_CASE("ent_ids can be copied", "[ginseng]")
{
    struct Data { int x; };

    DB db;

    auto ent1 = db.create_entity();

    db.add_component(ent1, Data{42});

    auto ent2 = ent1;

    REQUIRE(ent1 == ent2);

    auto& a = db.get_component<Data>(ent1);
    auto& b = db.get_component<Data>(ent2);

    REQUIRE(&a == &b);

    ent2 = ent1;

    REQUIRE(ent1 == ent2);

    auto& c = db.get_component<Data>(ent1);
    auto& d = db.get_component<Data>(ent2);

    REQUIRE(&a == &c);
    REQUIRE(&c == &d);
}

TEST_CASE("to_ptr and from_ptr work round-trip", "[ginseng]")
{
    DB db;

    for (int i = 0; i < 10; ++i) {
        db.create_entity();
    }

    auto ent = db.create_entity();

    auto ptr = db.to_ptr(ent);

    REQUIRE(reinterpret_cast<std::intptr_t>(ptr) == 10);

    auto ent2 = db.from_ptr(ptr);

    REQUIRE(ent == ent2);
}
