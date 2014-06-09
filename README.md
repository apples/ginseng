# Ginseng

An entity component framework designed for use in games.

# Disclaimer

This is an academic project, and it is not guaranteed to be suitable
for any particular purpose.

With that said, I feel that this framework is solid.
I have used it in many of my other projects.

# Features

## Fully type-safe

Any type can be easily stored as a component,
including built-in types, such as `int` and `float`.

## Efficient

From a high-level perspective,
the database is represented as a list of entities,
where each entity is a vector of pointer-to-components.

I believe that this is the most efficient representation,
without sacrificing ease-of-use.

A custom allocator may be used to allocate components
and entity list nodes.

## Extensible

Along with being able to use any type as a componenent,
the database is simple enough that extra functionality
can be composed on top of it.

# Contact

If you encounter any bugs or have a feature request,
feel free to [submit an issue][github-issues].

[github-issues]: https://github.com/dbralir/ginseng/issues
