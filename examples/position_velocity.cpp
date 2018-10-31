#include <ginseng/ginseng.hpp>

struct position {
    float x = 0;
    float y = 0;
};

struct velocity {
    float x = 0;
    float y = 0;
};

int main() {
    auto db = ginseng::database{};

    auto make_ent = [&db]{
        auto ent = db.create_entity();
        db.add_component(ent, position{});
        db.add_component(ent, velocity{});
    };

    make_ent();
    make_ent();
    make_ent();

    db.visit([&](position& pos, const velocity& vel) {
        pos.x += vel.x;
        pos.y += vel.y;
    });
}
