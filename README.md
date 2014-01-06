# Ginseng

An entity component framework designed for use in games.

# Disclaimer

This is an academic project, and it is not guaranteed to be suitable
for any particular purpose. It is nothing more than a basic framework,
created with purely academic intent.

This is also in active development,
and is nowhere near finished.
Breaking changes will happen a lot.

With that said, I feel that this framework is solid.
When used within its limitations, it should not fail.

# Features

## Fully type-safe

Types and functions are meticulously metaprogrammed
to ensure that the types you put in are the types you get out.
Dynamic polymorphism is kept to a minimum, and is always used safely.
CRTP is used to verify types, and does not require `dynamic_cast`.

## Efficient

Database structures are represented by `std::unordered_map`,
using integral types as keys.
This is considered a highly efficient associative container,
and has amortized constant-time operations.

## Extensible

The only requirement for components is that they inherit from
`Ginseng::Component<>`.
Default implementations are provided for any virtual functions,
so it is never necessary (although usually desirable) to override them.

## Serializable

The entire database is easily serializable,
as are individual components and entities.
This makes it easy to save data to a file
or send it over a network.

# Contact

If you encounter any bugs or have a feature request,
feel free to [submit an issue][github-issues].

[github-issues]: https://github.com/dbralir/ginseng/issues
