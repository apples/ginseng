#include <ginseng/ginseng.hpp>

struct position {
    float x = 0;
    float y = 0;
};

struct sprite {
    std::string texture;
};

void do_render(ginseng::database& render) {
    // sprite is the primary component
    db.visit([&](const sprite& spr, const position& pos) {
        draw_sprite(pos.x, pos.y, spr.texture);
    });
}
