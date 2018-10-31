#include <ginseng/ginseng.hpp>

#include <iostream>
#include <string>

using ginseng::database;
using ginseng::tag;
using ginseng::deny;

// Components can be any value type.

struct NameCom {
    std::string name;
};

struct PositionCom {
    double x;
    double y;
};

// Tag components will not contain a value (no allocation).
using IsEnemyTag = tag<struct IsEnemy>;

struct Game {
    database db; // Databases are value types.
    
    Game() {
        // db.create_entity() returns an entity ID.
        auto player = db.create_entity();
        
        // db.add_component() emplaces the given component into the entity.
        db.add_component(player, NameCom{"The Player"});
        db.add_component(player, PositionCom{12, 42});
        
        auto enemy = db.create_entity();
        db.add_component(enemy, NameCom{"An Enemy"});
        db.add_component(enemy, PositionCom{7, 53});
        db.add_component(enemy, IsEnemyTag{});
    }
    
    void run_game() {
        // db.visit() automatically detects visitor parameters.
        db.visit([](const NameCom& name, const PositionCom& pos){
            std::cout << "Entity " << name.name
                      << " is at (" << pos.x << "," << pos.y << ")."
                      << std::endl;
        });
    
        // The deny<> annotation can be used to skip unwanted entities.
        db.visit([](const NameCom& name, deny<IsEnemyTag>){
            std::cout << name.name << " is not an enemy." << std::endl;
        });
    }
};

int main() {
    Game g;
    g.run_game();
}
