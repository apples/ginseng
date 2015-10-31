#include "catch.hpp"

#include <ginseng/ginseng.hpp>

using DB = Ginseng::Database<>;
using Ginseng::Not;
using Ginseng::Tag;
using EntID = DB::EntID;
using ComID = DB::ComID;

TEST_CASE("Tag types do not use dynamic allocation", "[ginseng]")
{
    // Not sure how to actually test this, since allocations happen for the list nodes and entity vector.
    // Instead, I'll just check to make sure it actually compiles.

    DB db;

    struct SomeTag {};

    auto ent = db.makeEntity();
    DB::ComInfo<Tag<SomeTag>> tag_info = db.makeComponent(ent, Tag<SomeTag>{});

    int visited;
    DB::ComInfo<Tag<SomeTag>> visited_info;

    visited = 0;
    db.visit([&](Tag<SomeTag>){
        ++visited;
    });

    REQUIRE(visited == 1);

    visited = 0;
    db.visit([&](DB::ComInfo<Tag<SomeTag>> info){
        ++visited;
        visited_info = info;
    });

    REQUIRE(visited == 1);
    REQUIRE(visited_info.id() == tag_info.id());
}