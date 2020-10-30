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
- Entity IDs are versioned, so no worries about double-deletion or stale IDs.

## Status

Feature-complete and stable API.

## Dependencies

There are none! Ginseng is a single-header library that only requires C++17.

## Examples

See the `examples/` directory.

## Setup

Ginseng is header-only, so you just need to include it:

```cpp
#include <ginseng/ginseng.hpp>
```

## Initialization

All operations happen within a "database". The database has no constructor parameters:

```cpp
auto database = ginseng::database{};
```

No configuration needed or available.

## Managing Entities

### Creating an entity

```cpp
auto eid = database.create_entity();
```

The `ent_id` returned is an opaque struct.

It will be valid until the entity is destroyed, and will never refer to any other entity.

### Destroying an entity

```cpp
database.destroy_entity(eid);
```

Any attached components are also destroyed.

If called again with the same `ent_id`, nothing happens, it is a safe operation.

### Checking if an entity still exists

```cpp
auto eid = database.create_entity();

assert(database.exists(eid) == true);

database.destroy_entity(eid);

assert(database.exists(eid) == false);
```

## Managing Components

### Defining a component

A component can be any trivial type:

```cpp
struct position {
    float x = 0;
    float y = 0;
};
```

The only restriction is that it cannot be a raw pointer type.

### Adding a component to an entity

```cpp
auto pos = position{7, 42};

auto cid = database.add_component(eid, pos);
```

The `com_id` that is returned can be used to immediately refer to the added component efficiently,
and it will remain valid until the component is removed.

However, unlike an `ent_id`, a `com_id` cannot be checked for validity, so it is not recommended to store these.

### Checking for and getting a component from an entity

#### Safe method

```cpp
if (auto position = database.get_component<position*>(eid)) {
    pos->x = 8;
    pos->y = 32;
}
```

This is the easiest method. It is useful when you want to simultaneously
check for the existence of a component, and access it if it exists.

Note that the template parameter of `get_component` must be a pointer type.

#### Unsafe methods

```cpp
if (database.has_component<position>(eid)) {
    auto& pos = database.get_component<position>(eid);
    pos.x = 9;
    pos.y = 17;
}
```

It is an invalid operation to call `get_component` if the component does not exist,
so it is recommended to always check with `has_component`.

If the entity referred to by `ent_id` has been destroyed with `destroy_entity`,
then any call to `has_component` for that `ent_id` will return `false`.

#### Using a `com_id`

```cpp
auto& pos = database.get_component_by_id<position>(cid);
```

This is only valid with a `com_id` acquired from the most recent call to `add_component`
for the same type of component on the owning entity.

### Removing a component from an entity

```cpp
database.remove_component<position>(eid);
```

This invalidates the `com_id` returned from the `add_component` call that added this component.

If the entity referred to by `ent_id` has been destroyed with `destroy_entity`,
then nothing happens, it is a safe operation.

### Tags

```cpp
using is_enemy_tag = ginseng::tag<struct is_enemy_tag_t>;

database.add_component(eid, is_enemy_tag{});
```

Tag components are special in that they have no data. They have no associated `com_id`,
and it is invalid to call `get_component` for a tag type.

The benefit to using a tag is primarily that they do not require storage, and therefore consume no memory.

## Visiting

```cpp
database.visit([](position& pos, const velocity& vel) {
    pos.x += vel.x;
    pos.y += vel.y;
});
```

Visits all entities that have a `position` and `velocity` component,
and the parameters refer directly to those components.

It is safe to add or remove entities or components while visiting.
However, whether or not these new entities or components are visited is arbitrary.
If a component is removed, and a component of the same type is added later to the same entity,
it's possible that the entity will be visited twice, or not at all.
To avoid this problem, do not perform successive `remove_component` and `add_component` calls
for the same entity while visiting.

The number of entities that are checked for matching components corresponds to the number of entities
that have the first component listed in the parameters,
so it is recommended to put the most specific component first.
This is referred to as the "primary component".
Tags are not eligible to be the primary component.

For example, if you want to visit all entities that have a `position` component and a `player_input` component,
you might know that there will be very few `player_input` components compared to `position` components.

In that case, you should put the `player_input` parameter first:

```cpp
database.visit([](const player_input& input, position& pos) {
    pos.x += input.x_axis;
    pos.y += input.y_axis;
});
```

This ensures that only the entities which have a `player_input` are checked for components.

### Parameter types

There are some special parameter types other than references-to-components.

The full list is:

| Type                        | Result
|:----------------------------|:------
| `T`                         | Copy of a component of type `T`. Candidate for primary component.
| `T&`                        | Mutable reference to a component of type `T`. Not valid for tags. Candidate for primary component.
| `const T&`                  | Const reference to a component of type `T`. For tags, the reference is to a temporary object. Candidate for primary component.
| `ginseng::database::ent_id` | Gets the `ent_id` of the entity being visited.
| `ginseng::optional<T>`      | Optional reference to a component of type `T`. For tags, has no `.get()` method.
| `ginseng::require<T>`       | Require a component to exist without actually looking it up, for efficiency.
| `ginseng::deny<T>`          | Require that a component *not* exist. Recommended to only use with another primary component.

## Other Functions

### `size()`

Returns the total number of entities in the database.

### `count<Com>()`

Counts the number of entities that have a component of type `Com`.

## License

MIT

See [LICENSE.txt](https://github.com/dbralir/ginseng/blob/master/LICENSE.txt).

Copyright 2015 Jeramy Harrison
