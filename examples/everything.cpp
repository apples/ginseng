#include <ginseng/ginseng.hpp>

// Common type aliases

using ginseng::database;
using ent_id = database::ent_id;
using ginseng::tag;

// Standard component

struct posvel {
    float x = 0;
    float y = 0;
    float vx = 0;
    float vy = 0;
};

// Tag components

using wall_hit = tag<struct wall_hit_tag>;
using no_hit = tag<struct no_hit_tag>;

// Position update system

void update_positions(database& db) {
    db.visit([&](ent_id eid, posvel& pv) {
        pv.x += pv.vx;
        pv.y += pv.vy;

        if (pv.x > 100 || pv.x < -100 || pv.y > 100 || pv.y < -100) {
            db.add_component(eid, wall_hit{});
        }
    });
}

// No hit tag system

void update_no_hits(database& db) {
    db.visit([&](ent_id eid, ginseng::deny<wall_hit>) {
        db.add_component(eid, no_hit{});
    });
}

// Remove wall hitters system

void update_wall_hits(database& db) {
    db.visit([&](ent_id eid, wall_hit) {
        db.destroy_entity(eid);
    });
}

int main() {
    auto db = ginseng::database{};

    auto make_ent = [&db]{
        auto ent = db.create_entity();
        db.add_component(ent, posvel{});
    };

    for (int i = 0; i < 10; ++i) {
        make_ent();
    }

    update_positions(db);
    update_no_hits(db);
    update_wall_hits(db);
}
