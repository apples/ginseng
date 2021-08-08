Tutorial
########

Installation
************

Ginseng is a header-only library.

The only thing you need to do to include it in your project is Add the ``include`` directory to your project's compile include directories.

Ginseng requires C++17.

For example, using CMake:

.. code-block:: cmake
    
    add_executable(my_game main.cpp)
    target_compile_features(my_game PRIVATE cxx_std_17)
    target_include_directories(my_game PRIVATE
        "${CMAKE_SOURCE_DIR}/ext/ginseng/include")

Then, just include the main header in your source code:

.. code-block:: cpp

    #include <ginseng/ginseng.hpp>

You may want to use a few type aliases to make things easier:

.. code-block:: cpp

    using database = ginseng::database;
    using ent_id = database::ent_id;
    using com_id = database::com_id;

Constructing a Database
***********************

You need a ``ginseng::database`` to hold your entities.
Typically this would be placed directly inside of some kind of ``game_state`` class, or whatever your equivalent is.

.. code-block:: cpp

    struct game_state {
        ginseng::database ent_db;
    };

There are no constructor parameters or any other setup needed.

Defining Components
*******************

In Ginseng, any value type is a valid component. Even ``int`` is a valid component.

Typically, you should define your components as simple structs:

.. code-block:: cpp

    namespace component {

        struct position {
            float x;
            float y;
        };

        struct velocity {
            float vx;
            float vy;
        };
    
    }

There is no need to register these components, you can start using them right away.

There is no strict requirement, but your components should ideally be simple struct types, which are easily serializable.
Features such as virtual functions or inheritance will likely not work intuitively in Ginseng.

Components must be a non-pointer value type. Pointers and references must be wrapped in a struct or class component.

Using Entities
**************

An entity is essentially just a collection of components, where each component has a distinct type.

Typically, an entity will correspond to a game object or actor.

There is no limit to the number of entities you may create.

Creating Entities
=================

To create an entity, simply call the database's ``create_entity`` method:

.. code-block:: cpp

    auto my_ent = ent_db.create_entity();

The returned value is a handle to the created entity, of type ``ginseng::database::ent_id``.

It is a trivially copyable value type, so feel free to store and copy it as if it were a pointer type.

Destroying Entities
===================

Destroy entities with the ``destroy_entity`` method:

.. code-block:: cpp

    ent_db.destroy_entity(my_ent);

All of the components attached to the entity will also be destroyed.

Using Components
****************

Components are pieces of data which can be attached to an entity.

Adding Components to Entities
=============================

To add a component to an existing entity, use the ``add_component`` method:

.. code-block:: cpp

    ent_db.add_component(my_ent, component::position{x, y});

This will copy (or move) the given component and attach it to the entity.

.. warning::
    Attempting to add a component to an entity which already has a component of the same type will **overwrite** the existing component.
    An entity cannot have two components of the same type.

Removing Components from Entities
=================================

To remove an existing component from an entity, use the ``remove_component`` method:

.. code-block:: cpp

    ent_db.remove_component<component::position>(my_ent);

This will detach the component from the entity and delete the component.

Notice that the component type must be passed as a template parameter, since there is no function parameter to deduce it from.

.. warning::
    Calling ``remove_component`` for a component type which does not exist on that entity is **undefined behavior**.
    There is no error-checking for this scenario.

Determine if an Entity has a Component
======================================

To determing if an entity has a component of a certain type, use the ``has_component`` method:

.. code-block:: cpp

    if (ent_db.has_component<component::position>(my_ent)) {
        std::cout << "Position exists!\n";
    }

.. note::
    If you need to retrieve a component that may not exist,
    the preferred way to do this is to call the ``get_component`` method with a component pointer type.

Getting a Component From an Entity
==================================

If you know a component exists on an entity, you can use the ``get_component`` method to retrieve it directly:

.. code-block:: cpp

    auto& pos = ent_db.get_component<component::position>(my_ent);

If you are unsure if the component exists or not, use ``get_component`` with a component pointer type:

.. code-block:: cpp

    if (auto* pos_ptr = ent_db.get_component<component::position*>(my_ent)) {
        std::cout << "Position exists.\n";
    } else {
        std::cout << "Position does not exist.\n";
    }

Either method above will return a direct, mutable reference to the component data.
Feel free to modify it as you please.

Tag Components
**************

Tag components work similarly to normal components, except they do not have a value.

Typically a tag component is used when you need to identify an entity as being part of some broader category.

Define tag components using the ``ginseng::tag<T>`` template:

.. code-block:: cpp

    namespace component {

        using player_tag = ginseng::tag<struct player_tag_t>;

        using enemy_tag = ginseng::tag<struct enemy_tag_t>;
    
    }

Adding and removing tag components works the same as with normal components:

.. code-block:: cpp

    ent_db.add_component(my_ent, component::player_tag{});

    ent_db.has_component<component::player_tag>(my_ent);

    ent_db.remove_component<component::player_tag>(my_ent);

However, because tag components have no value, you cannot use them with the ``get_component`` database method.

Tag components can also be used in visitor functions just like regular components,
the only difference being their lack of a data value:

.. code-block:: cpp

    ent_db.visit([](component::player_tag, component::position& pos) {
        process_player_movement(pos);
    });

Using Visitors (aka Systems)
****************************

Ginseng does not have traditional ECS "systems" that need to be registered.
Instead, visitor functions provide the same functionality in a more immediate style.

To run a visitor on your entities, use the ``visit`` method:

.. code-block:: cpp

    ent_db.visit([](const component::velocity& vel, component::position& pos) {
        pos.x += vel.x;
        pos.y += vel.y;
    });

See the :doc:``visit`` page for more details and examples.
