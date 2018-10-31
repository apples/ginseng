# Ginseng

Ginseng is an entity-component-system (ECS) library designed for use in games.

The main advantage over similar libraries is that the component types do not need to be listed or registered.
Component types are detected dynamically.

Any function-like object can be used as a system.
The function's parameters are used to determine the required components.

## Features

- Fully type safe!
- No dynamic casts.
- Type erasure; no intrusive inheritance.
- No exceptions are thrown.
- Unlimited component types.
- Systems are just regular functions.
- Component objects are stable and can be added or removed freely.

## Status

Ginseng is currently in-development, but it already works exceptionally well.

Version 1.0 is considered stable.

## Dependencies

There are none! Ginseng is a single-header library that only requires C++17.

## Examples

See the `examples/` directory.

## Tutorial

Creating a Ginseng database:

```cpp
auto database = ginseng::database{};
```

No configuration needed.

Creating a position component:

```cpp
struct position {
    float x;
    float y;
};
```

Components are just regular value types.

Creating an entity:

```cpp
auto eid = database.create_entity();
```

Adding a component to an entity:

```cpp
database.add_component(eid, position{7, 42});
```

Checking for and getting a component from an entity:

```cpp
if (database.has_component<position>(eid)) {
    auto& pos = database.get_component<position>(eid);
}
```

Visiting all entities with a set of components:

```cpp
database.visit([](position& pos, velocity& vel) {
    pos.x += vel.x;
    pos.y += vel.y;
});
```

Visitor parameter types are used to fetch components.

Removing a component from an entity:

```cpp
database.remove_component<position>(eid);
```

Destroying an entity:

```cpp
database.destroy_entity(eid);
```

Destroying an entity also destroys its components.

Creating a tag component:

```cpp
using my_tag = ginseng::tag<struct my_tag_t>;
```

Tags are empty components that do not occupy memory.

Removing all entities that have a tag:

```cpp
database.visit([](ginseng::database::ent_id eid, my_tag) {
    database.destroy_entity(eid);
});
```

It is safe to add or remove entities or components while visiting.

## License

MIT

See [LICENSE.txt](https://github.com/dbralir/ginseng/blob/master/LICENSE.txt).

Copyright 2015 Jeramy Harrison
