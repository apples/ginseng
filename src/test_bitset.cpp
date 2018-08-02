#include "catch.hpp"

#include <ginseng/ginseng.hpp>

using dynamic_bitset = ginseng::_detail::dynamic_bitset;
constexpr auto word_size = dynamic_bitset::word_size;

TEST_CASE("dynamic_bitset is initialized to zero", "[dynamic_bitset]") {
    dynamic_bitset db;

    REQUIRE(db.size() == word_size);

    for (int i = 0; i < word_size; ++i) {
        REQUIRE(db.get(i) == false);
    }

    REQUIRE(db.size() == word_size);
}

TEST_CASE("setting an existing bit does not change size", "[dynamic_bitset]") {
    {
        dynamic_bitset db;

        REQUIRE(db.size() == word_size);

        for (int i = 0; i < word_size; ++i) {
            REQUIRE(db.get(i) == false);
            db.set(i);
            REQUIRE(db.get(i) == true);
        }

        REQUIRE(db.size() == word_size);
    }
    {
        dynamic_bitset db;

        db.resize(word_size * 2);

        REQUIRE(db.size() == word_size * 2);

        for (int i = 0; i < word_size * 2; ++i) {
            REQUIRE(db.get(i) == false);
            db.set(i);
            REQUIRE(db.get(i) == true);
        }

        REQUIRE(db.size() == word_size * 2);
    }
}

TEST_CASE("setting a bit past the end resizes to the next multiple of word size", "[dynamic_bitset]") {
    dynamic_bitset db;

    REQUIRE(db.size() == word_size);

    db.set(word_size);

    REQUIRE(db.size() == word_size * 2);
}

TEST_CASE("resize does not change existing bits", "[dynamic_bitset]") {
    dynamic_bitset db;

    db.set(0);

    REQUIRE(db.size() == word_size);

    db.set(word_size);

    REQUIRE(db.size() == word_size * 2);

    REQUIRE(db.get(0) == true);
    REQUIRE(db.get(word_size) == true);
}

TEST_CASE("unsetting a bit past the end has no effect", "[dynamic_bitset]") {
    dynamic_bitset db;

    db.set(0);
    db.set(word_size - 1);

    REQUIRE(db.size() == word_size);

    db.unset(word_size);

    REQUIRE(db.size() == word_size);
    REQUIRE(db.get(0) == true);
    REQUIRE(db.get(word_size - 1) == true);
}

TEST_CASE("zero works", "[dynamic_bitset]") {
    dynamic_bitset db;

    for (int i = 0; i < word_size; ++i) {
        db.set(i);
    }

    for (int i = 0; i < word_size; ++i) {
        REQUIRE(db.get(i) == true);
    }

    db.zero();

    REQUIRE(db.size() == word_size);

    for (int i = 0; i < word_size; ++i) {
        REQUIRE(db.get(i) == false);
    }

    for (int i = 0; i < word_size * 2; ++i) {
        db.set(i);
    }

    for (int i = 0; i < word_size * 2; ++i) {
        REQUIRE(db.get(i) == true);
    }

    db.zero();

    REQUIRE(db.size() == word_size * 2);

    for (int i = 0; i < word_size * 2; ++i) {
        REQUIRE(db.get(i) == false);
    }
}

TEST_CASE("dynamic_bitset is moved properly", "[dynamic_bitset]") {
    {
        dynamic_bitset db1;

        db1.set(5);

        dynamic_bitset db2 = std::move(db1);

        REQUIRE(db1.size() == word_size);
        REQUIRE(db2.size() == word_size);
        REQUIRE(db2.get(5) == true);
    }
    {
        dynamic_bitset db1;

        db1.set(5);
        db1.set(word_size + 5);

        dynamic_bitset db2 = std::move(db1);

        REQUIRE(db1.size() == word_size);
        REQUIRE(db2.size() == word_size * 2);
        REQUIRE(db2.get(5) == true);
        REQUIRE(db2.get(word_size + 5) == true);
    }
}

TEST_CASE("setting bits under the word size does not disable sdo", "[dynamic_bitset]") {
    dynamic_bitset db;

    REQUIRE(db.using_sdo());

    for (int i = 0; i < word_size; ++i) {
        db.set(i);
        REQUIRE(db.using_sdo() == true);
    }
}

TEST_CASE("setting bits over the word size disables sdo", "[dynamic_bitset]") {
    dynamic_bitset db;

    db.set(word_size + 10);
    REQUIRE(db.using_sdo() == false);

    db.set(0);
    REQUIRE(db.using_sdo() == false);
}

TEST_CASE("unsetting bits past the end does not disable sdo", "[dynamic_bitset]") {
    dynamic_bitset db;

    db.unset(word_size + 10);
    REQUIRE(db.using_sdo() == true);
}
