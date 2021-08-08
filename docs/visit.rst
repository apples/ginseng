Visitors
########

Intro
*****

Ginseng does not have traditional ECS "systems" that need to be registered.
Instead, visitor functions provide the same functionality in a more immediate style.

It is recommended to prefer using many specialized visitors, instead of having only a few visitors that do a lot of work.
Common examples of visitors are: drawing sprites, updating physics, updating timers, running AI logic, etc.

Typically, your game's update loop will consist mostly of running visitors.

Running a Visitor
*****************

To run a visitor on your entities, use the ``visit`` method:

.. code-block:: cpp

    ent_db.visit([](const component::velocity& vel, component::position& pos) {
        pos.x += vel.x;
        pos.y += vel.y;
    });

The visitor function is called on every entity, as long as that entity can provide the requested component parameters.

A non-tag component parameter must match the deduced type of either ``T``, ``T&``, or ``const T&``.

For tag components, the type must match ``T`` or ``const T&``. A mutable reference is not allowed, since tags have no value.

Special Parameter Types
***********************

Other than component types, there are a few special types that you can use as well.

These parameters must match the deduced type of either ``T`` or ``const T&``.
Mutable references are not allowed because all of these parameters are either valueless or temporary.

.. note::
    These special parameters are ignored when determining the "primary" component for a visitor.
    The primary component will always be the first parameter type which is a concrete component type.

``ginseng::database::ent_id``
=============================

A parameter of this type will be the ``ent_id`` of the current entity that is being visited.

You may freely add, get, or remove components from this entity during the visit,
and the ``ent_id`` will remain valid if copied out of the visitor.

You can also destroy the entity itself.

Example:

.. code-block:: cpp

    ent_db.visit([](ginseng::database::ent_id id, component::timer& timer) {
        if (timer.remaining <= 0) {
            ent_db.destroy_entity(id);
        }
    });

``ginseng::optional<T>``
========================

This type represents an optional component. All entites will match this parameter.

This type should be treated as a pointer.

It is explicitly convertible to ``bool``, which indicates whether the entity has this component.

Additionally, for non-tag components, you can dereference it (with ``*`` or ``->``) to get the component's data.

There is also a ``.get()`` method which does the same as dereferencing.

Optional tag components cannot be dereferenced and have no ``.get()`` method.

Example:

.. code-block:: cpp

    ent_db.visit([](const component::sprite& sprite, ginseng::optional<component::animation> anim) {
        if (anim) {
            draw_with_animation(sprite, *anim);
        } else {
            draw_without_animation(sprite);
        }
    });

``ginseng::require<T>``
=======================

A parameter of this type works the same way as a normal component type, except the component's data is not retrieved.

Use this when you need to visit entities which have a certain component, but you don't actually care about the value of that component.

This is only an optimization, and in most cases is not needed.

Example:

.. code-block:: cpp

    ent_db.visit([](ginseng::require<component::player>, component::position& pos) {
        process_player_movement(pos);
    });

``ginseng::deny<T>``
====================

This does the opposite of ``ginseng::require<T>``. Only components which **do not** have a component of this type will be visited.

.. note::
    Usually, it is better to create a tag component and add that tag to entities you care about,
    since ``ginseng::deny<T>`` might match a broader category of entities than you expect,
    especially as your project evolves over time.

Example:

.. code-block:: cpp

    ent_db.visit([](ginseng::deny<component::player>, component::position& pos) {
        process_npc_movement(pos);
    });

Primary Component
*****************

.. note::
    This is purely a discussion of optimization.
    You can use Ginseng perfectly fine without this knowledge.

The first normal component parameter of the visitor function will be used as the "primary" component.

The ``visit`` method is optimized to only examine entities which definitely have the primary component.

For example, let's say we've set up three entities as follows:

.. code-block:: cpp

    auto ent1 = ent_db.create_entity();
    ent_db.add_component(ent1, component::position{});

    auto ent2 = ent_db.create_entity();
    ent_db.add_component(ent1, component::position{});

    auto ent3 = ent_db.create_entity();
    ent_db.add_component(ent1, component::position{});
    ent_db.add_component(ent1, component::velocity{});

Now, if we run this visitor function:

.. code-block:: cpp

    ent_db.visit([](const component::velocity& vel, component::position& pos) {
        pos.x += vel.x;
        pos.y += vel.y;
    });

Since ``component::velocity`` is the first component parameter, it will be the primary component.

Therefore, only ``ent3`` will be considered for the visitor. Entities ``ent1`` and ``ent2`` will not even be considered.

This can be a huge optimization in the case where you have many entities, but a specific component type will be used by only a few.

An extreme example would be if your game has thousands of entities, but only one entity has the ``component::player`` component.
A visitor function which uses ``component::player`` as its primary component would immediately visit the player entity, and no other entities would even be examined.
