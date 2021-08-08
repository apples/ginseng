# Ginseng

Ginseng is an entity-component-system (ECS) library designed for use in games.

The main advantage over similar libraries is that the component types do not need to be listed or registered.
Component types are detected dynamically.

Any function-like object can be used as a system.
The function's parameters are used to determine the required components.

Here's a pseudo-example of what it looks like to use Ginseng:

```cpp
auto db = ginseng::database{};

// entity
auto goomba = db.create_entity();

// component
db.add_component(goomba, component::position{10, 20});
db.add_component(goomba, component::sprite{"goomba.png"});
db.add_component(goomba, component::behavior{"walk_left"});

// system
db.visit([](component::position& pos, const component::behavior& behavior) {
    if (behavior.state == "walk_left") {
        pos.x -= 1;
    } else {
        pos.x += 1;
    }
});
```

## Documentation

Full documentation can be found at [https://ginseng.readthedocs.io/](https://ginseng.readthedocs.io/).

## Features

- Fully type safe!
- No dynamic casts.
- No intrusive inheritance.
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

## License

MIT

See [LICENSE.txt](https://github.com/dbralir/ginseng/blob/master/LICENSE.txt).

Copyright 2015-2021 Apples
