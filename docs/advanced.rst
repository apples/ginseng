Advanced
########

Entity Indices and Versions
***************************

Each entity occupies a "slot" within the database, and each slot is given a unique index.
An ``ent_id`` is conceptually just the index (see :ref:`get_index()`) of the entity slot, though it is more than just a number.

All ``ent_id``s also have an opaque version identifier.
This version identifier is changed whenever an entity is created in an index which was previously occupied by another entity.

Some ``ent_id`` objects might have the same index, but have referred to different entities, and therefore will have different versions.
Because of this, you should not rely on ``get_index()`` for anything other than hashing and debugging,
and should use ``==`` to compare two ``ent_id`` values directly.

Because an ``ent_id`` has both an index and a version,
Ginseng is able to determine if an ``ent_id`` refers to a destroyed entity, even if a new entity is occupying the same index.
Also, Ginseng is able to differentiate between several ``ent_id``s which pointed to different objects, even if they share the same index.

Remember: although indices may get recycled, only one unique entity will exist at a specific index at a specific time.

Advanced Database Methods
*************************

``exists(ent_id)``
==================

This method determines if the given ``ent_id`` points to a currently valid entity.

Even if an entity with the same index currently exists,
this function will return ``false`` if the ``ent_id`` was created from an older entity which occupied the same index.

``get_component_by_id(com_id)``
===============================

.. note::
    This function is almost never necessary.
    You should always prefer the usual ``get_component<Com>(ent_id)`` function unless you have a specific need for this version.

This function directly obtains a component value from the given ``com_id``.
The only way to obtain a ``com_id`` is from a call to ``add_component()`` (tags do not have a ``com_id``).

A ``com_id`` is a simple value that points directly to the component's value.

.. warning::
    Behavior is undefined when an invalid or expired ``com_id`` is used, so you must be extra careful when using this function!

``size()``
==========

Returns the number of entities in the database.

``count<Com>()``
================

Returns the number of entities which have the specified component.

``to_ptr(ent_id)`` and ``from_ptr(void*)``
==========================================

``to_ptr(ent_id)`` converts an ``ent_id`` to a ``void*`` for storage purposes (and no other purposes!).

Use this function to essentially serialize an ``ent_id``.

To convert the ``void*`` back to the original ``ent_id``, use ``from_ptr(void*)``.
It must only be given a ``void*`` which was obtained from ``to_ptr(ent_id)``.

.. note::
    The type ``void*`` is used because many language interop layers (e.g. Lua) also use ``void*`` as a generic "custom value".
    Returning ``void*`` here makes using such libraries smoother.
    Additionally, I feel like using ``void*`` instead of e.g. a ``byte[]`` discourages persistent storage.

.. warning::
    This is not a valid pointer and relies on widespread compiler-specific behavior.
    Do not ever dereference the pointer.

.. warning::
    Entity version checking is not preserved through conversion to and from pointers.

.. warning::
    Null pointers are a valid result and may represent an actual entity.
