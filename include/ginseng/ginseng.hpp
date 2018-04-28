#ifndef GINSENG_GINSENG_HPP
#define GINSENG_GINSENG_HPP

#include <algorithm>
#include <bitset>
#include <memory>
#include <type_traits>
#include <vector>

#include <cstddef>

namespace ginseng {

namespace _detail {

// Forward Declarations

template <typename DB>
struct database_traits;

// Type Guid

using type_guid = std::size_t;

inline type_guid get_next_type_guid() noexcept {
    static type_guid x = 0;
    return ++x;
}

template <typename T>
struct type_guid_trait {
    static const type_guid value;
};

template <typename T>
const type_guid type_guid_trait<T>::value = get_next_type_guid();

template <typename T>
type_guid get_type_guid() {
    return type_guid_trait<T>::value;
}

// Dynamic Bitset

class dynamic_bitset {
public:
    using size_type = std::size_t;

    static constexpr size_type word_size = 64;

    using bitset = std::bitset<word_size>;
    using bitset_array = std::unique_ptr<bitset[]>;

    dynamic_bitset()
        : sdo(0), numbits(word_size) {}

    dynamic_bitset(const dynamic_bitset&) = delete;
    dynamic_bitset& operator=(const dynamic_bitset&) = delete;

    dynamic_bitset(dynamic_bitset&& other)
        : sdo(0), numbits(word_size) {
        if (other.numbits == word_size) {
            sdo = other.sdo;
        } else {
            sdo.~bitset();
            new (&dyna) bitset_array(std::move(other.dyna));
            other.dyna.~unique_ptr();
            numbits = other.numbits;
            other.numbits = word_size;
            other.sdo = 0;
        }
    }

    dynamic_bitset& operator=(dynamic_bitset&& other) {
        if (numbits == word_size) {
            sdo.~bitset<word_size>();
        } else {
            dyna.~unique_ptr();
        }
        if (other.numbits == word_size) {
            sdo = other.sdo;
        } else {
            new (&dyna) bitset_array(std::move(other.dyna));
            other.dyna.~unique_ptr();
        }
        numbits = other.numbits;
        other.sdo = 0;
        other.numbits = word_size;
        return *this;
    }

    ~dynamic_bitset() {
        if (numbits == word_size) {
            sdo.~bitset();
        } else {
            dyna.~unique_ptr();
        }
    }

    size_type size() const {
        return numbits;
    }

    void resize(size_type ns) {
        if (ns > numbits) {
            auto count = (ns + word_size - 1u) / word_size;
            auto newlen = count * word_size;
            auto newptr = std::make_unique<bitset[]>(count);
            if (numbits == word_size) {
                newptr[0] = sdo;
                sdo.~bitset();
                new (&dyna) bitset_array(std::move(newptr));
            } else {
                copy(dyna.get(), dyna.get() + (numbits / word_size), newptr.get());
                std::fill(newptr.get() + (numbits / word_size), newptr.get() + (newlen / word_size), 0);
                dyna = std::move(newptr);
            }
            numbits = newlen;
        }
    }

    bool get(size_type i) const {
        if (numbits == word_size) {
            return sdo[i];
        } else {
            return dyna[i / 64][i % 64];
        }
    }

    void set(size_type i) {
        resize(i + 1);
        if (numbits == word_size) {
            sdo[i] = true;
        } else {
            dyna[i / 64][i % 64] = true;
        }
    }

    void unset(size_type i) {
        resize(i + 1);
        if (numbits == word_size) {
            sdo[i] = false;
        } else {
            dyna[i / 64][i % 64] = false;
        }
    }

    void zero() {
        if (numbits == word_size) {
            sdo = 0;
        } else {
            std::fill(dyna.get(), dyna.get() + numbits / word_size, 0);
        }
    }

private:
    union {
        bitset sdo;
        bitset_array dyna;
    };
    size_type numbits;
};

// Entity

struct entity {
    dynamic_bitset components = {};
};

// False Type

template <typename T>
struct false_t : std::false_type {};

// Queries

/*! Tag component
 *
 * When a tag component is stored in an entity, only the fact that it exists is recorded, no data is stored.
 */
template <typename T>
struct tag {};

/*! Require component
 *
 * When used as a visitor parameter, applies the matching logic for the parameter, but does not load the component.
 */
template <typename T>
struct require {};

/*! Deny component
 *
 * When used as a visitor parameter, entities that have the component will fail to match.
 */
template <typename T>
struct deny {};

/*! Optional component
 *
 * When used as a visitor parameter, loads the component if it exists, and does not cause matching to fail.
 *
 * Provides pointer-like access to the parameter.
 */
template <typename T>
class optional {
public:
    optional()
        : com(nullptr) {}
    explicit optional(T& c)
        : com(&c) {}
    optional(const optional&) = default;
    optional(optional&&) = default;
    optional& operator=(const optional&) = default;
    optional& operator=(optional&&) = default;
    T* operator->() const {
        return com;
    }
    T& operator*() const {
        return *com;
    }
    explicit operator bool() const {
        return com;
    }
    T& get() const {
        return *com;
    }

private:
    T* com;
};

/*! Optional Tag component
 *
 * When used as a visitor parameter, checks if the tag exists, but does not cause matching to fail.
 */
template <typename T>
class optional<tag<T>> {
public:
    optional()
        : tag(false) {}
    explicit optional(bool t)
        : tag(t) {}
    optional(const optional&) = default;
    optional(optional&&) = default;
    optional& operator=(const optional&) = default;
    optional& operator=(optional&&) = default;
    explicit operator bool() const {
        return tag;
    }

private:
    bool tag;
};

template <typename T>
class optional<require<T>> {
public:
    static_assert(false_t<T>::value, "Optional require parameters not allowed.");
};

template <typename T>
class optional<deny<T>> {
public:
    static_assert(false_t<T>::value, "Optional deny parameters not allowed.");
};

template <typename T>
class optional<optional<T>> {
public:
    static_assert(false_t<T>::value, "Optional optional parameters not allowed.");
};

// Component Tags

namespace component_tags {

struct unit {};
struct positive {};
struct normal : positive {};
struct noload : positive, unit {};
struct tagged : positive, unit {};
struct meta {};
struct nofail : meta {};
struct eid : meta {};
struct inverted : noload {};

} // namespace component_tags

// Component Traits

template <typename DB, typename Component>
struct component_traits {
    using category = component_tags::normal;
    using component = Component;
};

template <typename DB, typename Component>
struct component_traits<DB, require<Component>> {
    using category = component_tags::noload;
    using component = Component;
};

template <typename DB, typename Component>
struct component_traits<DB, tag<Component>> {
    using category = component_tags::tagged;
    using component = tag<Component>;
};

template <typename DB, typename Component>
struct component_traits<DB, optional<Component>> {
    using category = component_tags::nofail;
    using component = Component;
};

template <typename DB, typename Component>
struct component_traits<DB, deny<Component>> {
    using category = component_tags::inverted;
    using component = Component;
};

template <typename DB>
struct component_traits<DB, typename DB::ent_id> {
    using category = component_tags::eid;
    using component = void;
};

// First

template <typename T, typename... Ts>
struct first {
    using type = T;
};

template <typename... Ts>
using first_t = typename first<Ts...>::type;

// Primary

template <typename T>
struct primary {
    using type = T;
};

// Database Traits

template <typename DB>
struct database_traits {

    using ent_id = typename DB::ent_id;
    using com_id = typename DB::com_id;

    template <typename C>
    using component_traits = component_traits<DB, C>;

    // GetPrimary

    template <typename HeadTag, typename... Components>
    struct get_primary;

    template <typename... Components>
    using get_primary_t = typename get_primary<typename component_traits<first_t<Components...>>::category, Components...>::type;

    template <typename HeadTag, typename HeadCom, typename... Components>
    struct get_primary<HeadTag, HeadCom, Components...> {
        using type = get_primary_t<Components...>;
    };

    template <typename HeadTag, typename HeadCom>
    struct get_primary<HeadTag, HeadCom> {
        using type = primary<void>;
    };

    template <typename HeadCom, typename... Components>
    struct get_primary<component_tags::normal, HeadCom, Components...> {
        using type = primary<HeadCom>;
    };

    template <typename HeadCom>
    struct get_primary<component_tags::normal, HeadCom> {
        using type = primary<HeadCom>;
    };

    // VisitorKey

    template <typename Primary, typename... Coms>
    struct visitor_key;

    template <typename Primary, typename... Coms>
    struct visitor_key<primary<Primary>, Coms...> {
        template <typename Com>
        static bool check(DB& db, ent_id eid, component_tags::positive) {
            using component = typename component_traits<Com>::component;
            if constexpr (std::is_same_v<Primary, component>) {
                return true;
            } else {
                return db.template has_component<component>(eid);
            }
        }

        template <typename Com>
        static bool check(DB& db, ent_id eid, component_tags::inverted) {
            using component = typename component_traits<Com>::component;
            using inverted_key = visitor_key<primary<Primary>, component>;
            return !inverted_key::check(db, eid);
        }

        template <typename Com>
        static bool check(DB& db, ent_id eid, component_tags::meta) {
            return true;
        }

        template <typename Com>
        using tag_t = typename component_traits<Com>::category;

        static bool check(DB& db, ent_id eid) {
            return (check<Coms>(db, eid, tag_t<Coms>{}) && ...);
        }
    };

    // VisitorTraits

    template <typename... Components>
    struct visitor_traits_impl {
        using ent_id = typename DB::ent_id;
        using com_id = typename DB::com_id;
        using primary_component = get_primary_t<Components...>;
        using key = visitor_key<primary_component, Components...>;

        template <typename Com, typename Primary>
        static Com& get_com(component_tags::normal, DB& db, const ent_id& eid, const com_id& primary_cid, primary<Primary>) {
            if constexpr (std::is_same_v<Com, Primary>) {
                return db.template get_component_by_id<Com>(primary_cid);
            } else {
                return db.template get_component<Com>(eid);
            }
        }

        template <typename Com, typename Primary>
        static Com get_com(component_tags::nofail, DB& db, const ent_id& eid, const com_id& primary_cid, primary<Primary>) {
            using traits = component_traits<Com>;
            using inner_component = typename traits::component;
            using inner_traits = component_traits<inner_component>;
            using inner_category = typename inner_traits::category;
            return get_com_nofail<inner_component>(db, eid, primary_cid, primary<Primary>{}, inner_category{});
        }

        template <typename Com, typename Primary>
        static optional<Com> get_com_nofail(component_tags::normal, DB& db, const ent_id& eid, const com_id& primary_cid, primary<Primary>) {
            if constexpr (std::is_same_v<Com, Primary>) {
                return db.template get_component_by_id<Com>(primary_cid);
            } else {
                if (db.template has_component<Com>(eid)) {
                    return optional<Com>(db.template get_component<Com>(eid));
                } else {
                    return optional<Com>();
                }
            }
        }

        template <typename Com, typename Primary>
        static optional<Com> get_com_nofail(component_tags::tagged, DB& db, const ent_id& eid, const com_id& primary_cid, primary<Primary>) {
            (void)primary_cid;
            return db.template has_component<Com>(eid);
        }

        template <typename Com, typename Primary>
        static const ent_id& get_com(component_tags::eid, DB& db, const ent_id& eid, const com_id& primary_cid, primary<Primary>) {
            (void)db;
            (void)primary_cid;
            return eid;
        }

        template <typename Com, typename Primary>
        static Com get_com(component_tags::unit, DB& db, const ent_id& eid, const com_id& primary_cid, primary<Primary>) {
            (void)db;
            (void)eid;
            (void)primary_cid;
            return {};
        }

        template <typename Com>
        using tag_t = typename component_traits<Com>::category;

        template <typename Visitor>
        static auto apply(DB& db, ent_id eid, com_id primary_cid, Visitor&& visitor) {
            return std::forward<Visitor>(visitor)(get_com<Components>(tag_t<Components>{}, db, eid, primary_cid, primary_component{})...);
        }

        template <typename Visitor>
        static auto apply(DB& db, ent_id eid, com_id primary_cid, Visitor&& visitor, primary<void>) {
            return std::forward<Visitor>(visitor)(get_com<Components>(tag_t<Components>{}, db, eid, primary_cid, primary<void>{})...);
        }
    };

    template <typename Visitor>
    struct visitor_traits : visitor_traits<decltype(&std::decay_t<Visitor>::operator())> {};

    template <typename R, typename... Ts>
    struct visitor_traits<R (&)(Ts...)> : visitor_traits_impl<std::decay_t<Ts>...> {};

    template <typename Visitor, typename R, typename... Ts>
    struct visitor_traits<R (Visitor::*)(Ts...)> : visitor_traits_impl<std::decay_t<Ts>...> {};

    template <typename Visitor, typename R, typename... Ts>
    struct visitor_traits<R (Visitor::*)(Ts...) const> : visitor_traits_impl<std::decay_t<Ts>...> {};

    template <typename Visitor, typename R, typename... Ts>
    struct visitor_traits<R (Visitor::*)(Ts...)&> : visitor_traits_impl<std::decay_t<Ts>...> {};

    template <typename Visitor, typename R, typename... Ts>
    struct visitor_traits<R (Visitor::*)(Ts...) const &> : visitor_traits_impl<std::decay_t<Ts>...> {};

    template <typename Visitor, typename R, typename... Ts>
    struct visitor_traits<R (Visitor::*)(Ts...) &&> : visitor_traits_impl<std::decay_t<Ts>...> {};
};

// Component Set

class component_set {
public:
    using size_type = std::size_t;
    virtual ~component_set() = 0;
    virtual void remove(size_type entid) = 0;
};

inline component_set::~component_set() = default;

template <typename T>
class component_set_impl final : public component_set {
public:
    virtual ~component_set_impl() = default;

    size_type assign(size_type entid, T com) {
        if (entid >= entid_to_comid.size()) {
            entid_to_comid.resize(entid + 1, -1);
        }

        auto comid = comid_to_entid.size();

        entid_to_comid[entid] = comid;
        comid_to_entid.push_back(entid);
        components.push_back(std::move(com));

        return comid;
    }

    virtual void remove(size_type entid) override final {
        auto last = comid_to_entid.size() - 1;
        auto comid = entid_to_comid[entid];

        entid_to_comid[entid] = -1;
        entid_to_comid[comid_to_entid[last]] = comid;

        comid_to_entid[comid] = comid_to_entid[last];
        comid_to_entid.pop_back();

        components[comid] = std::move(components[last]);
        components.pop_back();
    }

    size_type get_comid(size_type entid) const {
        return entid_to_comid[entid];
    }

    T& get_com(size_type comid) {
        return components[comid];
    }

    size_type get_entid(size_type comid) const {
        return comid_to_entid[comid];
    }

    auto size() const {
        return components.size();
    }

private:
    std::vector<size_type> entid_to_comid;
    std::vector<size_type> comid_to_entid;
    std::vector<T> components;
};

template <typename T>
class component_set_impl<tag<T>> final : public component_set {
public:
    virtual ~component_set_impl() = default;
    virtual void remove(size_type entid) override final {}
};

// Opaque index

template <typename Tag, typename Friend, typename Index>
class opaque_index {
public:
    opaque_index() = default;

    bool operator==(const opaque_index& other) {
        return index == other.index;
    }

    Index get_index() const {
        return index;
    }

private:
    friend Friend;

    opaque_index(Index i)
        : index(i) {}

    operator Index() const {
        return index;
    }

    opaque_index& operator++() {
        ++index;
        return *this;
    }

    Index index;
};

/*! Database
 *
 * An Entity component Database. Uses the given allocator to allocate
 * components, and may also use the same allocator for internal data.
 *
 * @warning
 * This container does not perform any synchronization. Therefore, it is not
 * considered "thread-safe".
 */
class database {
public:
    // IDs

    /*! Entity ID.
     */
    using ent_id = opaque_index<struct ent_id_tag, database, std::vector<entity>::size_type>;

    /*! Component ID.
     */
    using com_id = opaque_index<struct com_id_tag, database, component_set::size_type>;

    /*! Creates a new Entity.
     *
     * Creates a new Entity that has no components.
     *
     * @return ID of the new Entity.
     */
    ent_id create_entity() {
        ent_id eid;

        if (free_entities.empty()) {
            eid = entities.size();
            entities.emplace_back();
        } else {
            eid = free_entities.back();
            free_entities.pop_back();
        }

        entities[eid].components.set(0);

        return eid;
    }

    /*! Destroys an Entity.
     *
     * Destroys the given Entity and all associated components.
     *
     * @param eid ID of the Entity to erase.
     */
    void destroy_entity(ent_id eid) {
        for (dynamic_bitset::size_type i = 1; i < entities[eid].components.size(); ++i) {
            if (entities[eid].components.get(i)) {
                component_sets[i]->remove(eid);
            }
        }

        entities[eid].components.zero();
        free_entities.push_back(eid);
    }

    /*! Create new component.
     *
     * Creates a new component from the given value and associates it with
     * the given Entity.
     * If a component of the same type already exists, it will be
     * overwritten.
     *
     * @param eid Entity to attach new component to.
     * @param com Component value.
     * @return ID of component.
     */
    template <typename T>
    com_id create_component(ent_id eid, T&& com) {
        using com_type = std::decay_t<T>;
        auto guid = get_type_guid<com_type>();
        auto& ent_coms = entities[eid].components;
        auto& com_set = get_or_create_com_set<com_type>();

        com_id cid;

        if (guid < ent_coms.size() && ent_coms.get(guid)) {
            cid = com_set.get_comid(eid);
            com_set.get_com(cid) = std::forward<T>(com);
        } else {
            cid = com_set.assign(eid, std::forward<T>(com));
            ent_coms.set(guid);
        }

        return cid;
    }

    /*! Create new Tag component.
     *
     * Creates a new Tag component associates it with the given Entity.
     *
     * @param eid Entity to attach new Tag component to.
     * @param com Tag value.
     */
    template <typename T>
    void create_component(ent_id eid, tag<T> com) {
        auto guid = get_type_guid<tag<T>>();
        auto& ent_coms = entities[eid].components;

        get_or_create_com_set<tag<T>>();

        ent_coms.set(guid);
    }

    template <typename T>
    void create_component(ent_id eid, require<T> com) = delete;

    template <typename T>
    void create_component(ent_id eid, deny<T> com) = delete;

    template <typename T>
    void create_component(ent_id eid, optional<T> com) = delete;

    /*! Destroy a component.
     *
     * Destroys the given component and disassociates it from its Entity.
     *
     * @warning
     * All ComIDs associated with components of the component's Entity will
     * be invalidated.
     *
     * @tparam Com Type of the component to erase.
     *
     * @param eid ID of the entity.
     */
    template <typename Com>
    void destroy_component(ent_id eid) {
        auto guid = get_type_guid<Com>();
        auto& com_set = *get_com_set<Com>();
        com_set.remove(eid);
        entities[eid].components.unset(guid);
    }

    /*! Get a component.
     *
     * Gets a reference to the component of the given type
     * that is associated with the given entity.
     *
     * @warning
     * Behavior is undefined when the entity has no associated
     * component of the given type.
     *
     * @tparam Com Type of the component to get.
     *
     * @param eid ID of the entity.
     * @return Reference to the component.
     */
    template <typename Com>
    Com& get_component(ent_id eid) {
        auto& com_set = *get_com_set<Com>();
        auto cid = com_set.get_comid(eid);
        return com_set.get_com(cid);
    }

    /*! Get a component by its ID.
     *
     * Gets a reference to the component of the given type that has the given ID.
     *
     * @warning
     * Behavior is undefined when no component is associated with the given ID.
     *
     * @tparam Com Type of the component to get.
     *
     * @param cid ID of the component.
     * @return Reference to the component.
     */
    template <typename Com>
    Com& get_component_by_id(com_id cid) {
        auto& com_set = *get_com_set<Com>();
        return com_set.get_com(cid);
    }

    /*! Checks if an entity has a component.
     *
     * Returns whether or not the entity has a component of the
     * associated type.
     *
     * @tparam Com Type of the component to check.
     *
     * @param eid ID of the entity.
     * @return True if the component exists.
     */
    template <typename Com>
    bool has_component(ent_id eid) {
        auto guid = get_type_guid<Com>();
        auto& ent_coms = entities[eid].components;
        return guid < ent_coms.size() && ent_coms.get(guid);
    }

    /*! Visit the Database.
     *
     * Visit the Entities in the Database that match the given function's parameter types.
     *
     * The following parameter categories are accepted:
     *
     * - Component Data: Any `T` except rvalue-references, matches entities that have component `T`.
     * - Component Tag: `tag<T>` value, matches entities that have component `tag<T>`.
     * - Component Require: `require<T>` value, matches entities that have component `T`, but does not load it.
     * - Component Optional: `optional<T>` value, checks if a component exists, and loads it, does not fail.
     * - Inverted: `deny<T>` value, matches entities that do *not* match component `T`.
     * - Entity ID: `ent_id`, matches all entities, provides the `ent_id` of the current entity.
     *
     * Component Data and Optional parameters will refer to the entity's matching component.
     * Entity ID parameters will contain the entity's EntID.
     * Other parameters will be their default value.
     *
     * Entities that do not match all given parameter conditions will be skipped.
     *
     * @warning Entities are visited in no particular order, so adding and removing entities from the visitor
     *          function could result in non-deterministic behavior.
     *
     * @tparam Visitor Visitor function type.
     * @param visitor Visitor function.
     */
    template <typename Visitor>
    void visit(Visitor&& visitor) {
        using db_traits = database_traits<database>;
        using traits = typename db_traits::visitor_traits<Visitor>;
        using primary_component = typename traits::primary_component;

        return visit_helper( std::forward<Visitor>(visitor), primary_component{});
    }

    template <typename Visitor>
    void visit_pairs(Visitor&& visitor) {
        using db_traits = database_traits<database>;
        using traits = typename db_traits::visitor_traits<Visitor>;
        using key = typename traits::key;

        for (auto eid = 0; eid < entities.size(); ++eid) {
            if (entities[eid].components.get(0) && key::check(*this, eid)) {
                auto inner_visitor = traits::apply(*this, eid, {}, visitor, primary<void>{});
                using inner_traits = typename db_traits::visitor_traits<decltype(inner_visitor)>;
                using inner_key = typename inner_traits::key;
                for (auto inner_eid = eid + 1; inner_eid < entities.size(); ++inner_eid) {
                    if (entities[inner_eid].components.get(0) && inner_key::check(*this, inner_eid)) {
                        inner_traits::apply(*this, inner_eid, {}, inner_visitor, primary<void>{});
                    }
                }
            }
        }
    }

    /*! Get the number of entities in the Database.
     *
     * @return Number of entities in the Database.
     */
    auto size() const {
        return entities.size() - free_entities.size();
    }

    bool exists(ent_id eid) const {
        return entities[eid].components.get(0);
    }

private:
    template <typename Com>
    component_set_impl<Com>* get_com_set() {
        auto guid = get_type_guid<Com>();
        if (guid >= component_sets.size()) {
            return nullptr;
        }
        auto& com_set = component_sets[guid];
        auto com_set_impl = static_cast<component_set_impl<Com>*>(com_set.get());
        return com_set_impl;
    }

    template <typename Com>
    component_set_impl<Com>& get_or_create_com_set() {
        auto guid = get_type_guid<Com>();
        if (component_sets.size() <= guid) {
            component_sets.resize(guid + 1);
        }
        auto& com_set = component_sets[guid];
        if (!com_set) {
            com_set = std::make_unique<component_set_impl<Com>>();
        }
        auto com_set_impl = static_cast<component_set_impl<Com>*>(com_set.get());
        return *com_set_impl;
    }

    template <typename Visitor, typename Component>
    void visit_helper(Visitor&& visitor, primary<Component>) {
        using db_traits = database_traits<database>;
        using traits = typename db_traits::visitor_traits<Visitor>;
        using key = typename traits::key;

        if (auto com_set_ptr = get_com_set<Component>()) {
            auto& com_set = *com_set_ptr;

            for (com_id cid = 0; cid < com_set.size(); ++cid) {
                auto eid = com_set.get_entid(cid);
                if (key::check(*this, eid)) {
                    traits::apply(*this, eid, cid, visitor);
                }
            }
        }
    }

    template <typename Visitor>
    void visit_helper(Visitor&& visitor, primary<void>) {
        using db_traits = database_traits<database>;
        using traits = typename db_traits::visitor_traits<Visitor>;
        using key = typename traits::key;

        for (auto eid = 0; eid < entities.size(); ++eid) {
            if (entities[eid].components.get(0) && key::check(*this, eid)) {
                traits::apply(*this, eid, {}, visitor);
            }
        }
    }

    std::vector<entity> entities;
    std::vector<ent_id> free_entities;
    std::vector<std::unique_ptr<component_set>> component_sets;
};

} // namespace _detail

using _detail::database;
using _detail::require;
using _detail::optional;
using _detail::deny;
using _detail::tag;

} // namespace Ginseng

#endif // GINSENG_GINSENG_HPP
