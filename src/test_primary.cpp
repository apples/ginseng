#include "catch.hpp"

#include <ginseng/ginseng.hpp>

#include <algorithm>

using DB = ginseng::database;
using ginseng::deny;
using ginseng::tag;
using ginseng::optional;
using ent_id = DB::ent_id;
using com_id = DB::com_id;

TEST_CASE("Primary components are visited in cache-freindly order", "[ginseng]")
{
    DB db;

    struct ID { int id; };
    struct Data { float val; };

    int next_id = 0;

    std::vector<ent_id> eids;
    eids.push_back(db.create_entity());
    eids.push_back(db.create_entity());
    eids.push_back(db.create_entity());
    eids.push_back(db.create_entity());
    eids.push_back(db.create_entity());

    db.add_component(eids[1], ID{next_id++});
    db.add_component(eids[3], ID{next_id++});
    db.add_component(eids[0], ID{next_id++});
    db.add_component(eids[4], ID{next_id++});
    db.add_component(eids[2], ID{next_id++});

    std::vector<ID*> ptrs;
    ptrs.reserve(5);

    db.visit([&](ID& id) { ptrs.push_back(&id); });
    REQUIRE(std::is_sorted(begin(ptrs), end(ptrs)));

    ptrs.clear();
    db.visit([&](ent_id, ID& id) { ptrs.push_back(&id); });
    REQUIRE(std::is_sorted(begin(ptrs), end(ptrs)));

    db.add_component(eids[0], Data{7.5});
    db.add_component(eids[3], Data{7.5});
    db.add_component(eids[1], Data{7.5});
    db.add_component(eids[2], Data{7.5});
    db.add_component(eids[4], Data{7.5});

    ptrs.clear();
    db.visit([&](ID& id) { ptrs.push_back(&id); });
    REQUIRE(std::is_sorted(begin(ptrs), end(ptrs)));

    std::vector<Data*> dataptrs;
    dataptrs.reserve(5);

    ptrs.clear();
    db.visit([&](ID& id, Data& data) { ptrs.push_back(&id); dataptrs.push_back(&data); });
    REQUIRE(std::is_sorted(begin(ptrs), end(ptrs)));
    REQUIRE(!std::is_sorted(begin(dataptrs), end(dataptrs)));

    ptrs.clear();
    dataptrs.clear();
    db.visit([&](Data& data, ID& id) { ptrs.push_back(&id); dataptrs.push_back(&data); });
    REQUIRE(!std::is_sorted(begin(ptrs), end(ptrs)));
    REQUIRE(std::is_sorted(begin(dataptrs), end(dataptrs)));
}
