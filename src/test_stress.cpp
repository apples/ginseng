#include "catch.hpp"

#include <ginseng/ginseng.hpp>

#include <algorithm>

using DB = ginseng::database;
using ginseng::deny;
using ginseng::tag;
using ginseng::optional;
using ent_id = DB::ent_id;
using com_id = DB::com_id;

TEST_CASE("Component sets can handle many components", "[ginseng]")
{
    DB db;

    struct ID { int id; };

    int next_id = 0;

    auto make_ent = [&]()
    {
        auto ent = db.create_entity();
        db.add_component(ent, ID{next_id});
        ++next_id;
        return ent;
    };

    constexpr auto SZ = 10000000;

    for (int i=0; i<SZ; ++i) {
        make_ent();
    }

    REQUIRE(db.size() == SZ);

    auto sz = db.size();
    auto c = 0;
    auto cd = 0;

    db.visit([&](ent_id eid, ID& id){
        if (id.id != c) {
            ++cd;
        }
        if (c % 11 == 0) {
            db.destroy_entity(eid);
            --sz;
        }
        if (c % 7 == 0) {
            make_ent();
            ++sz;
        }
        ++c;
    });

    REQUIRE(db.size() == sz);
    REQUIRE(c == SZ);
    REQUIRE(cd == 0);

    auto sz2 = 0ull;
    db.visit([&](ent_id, ID&){
        ++sz2;
    });

    REQUIRE(sz == sz2);

    REQUIRE(db.size() == 10519481);
}
