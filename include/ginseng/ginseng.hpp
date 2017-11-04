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

using namespace std;

// Type Guid

using type_guid = size_t;

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
    using size_type = size_t;

    static constexpr size_type word_size = 64;

    dynamic_bitset()
        : sdo(0), numbits(word_size) {}

    dynamic_bitset(const dynamic_bitset&) = delete;
    dynamic_bitset& operator=(const dynamic_bitset&) = delete;

    dynamic_bitset(dynamic_bitset&& other)
        : sdo(0), numbits(word_size) {
        if (other.numbits == word_size) {
            sdo = other.sdo;
        } else {
            new (&dyna) unique_ptr<bitset<word_size>[]>(move(other.dyna));
            other.dyna.~unique_ptr<bitset<word_size>[]>();
            numbits = other.numbits;
            other.numbits = word_size;
            other.sdo = 0;
        }
    }

    dynamic_bitset& operator=(dynamic_bitset&& other) {
        if (numbits != word_size) {
            dyna.~unique_ptr<bitset<word_size>[]>();
        }
        if (other.numbits == word_size) {
            sdo = other.sdo;
        } else {
            new (&dyna) unique_ptr<bitset<word_size>[]>(move(other.dyna));
            other.dyna.~unique_ptr<bitset<word_size>[]>();
        }
        numbits = other.numbits;
        other.sdo = 0;
        other.numbits = word_size;
        return *this;
    }

    ~dynamic_bitset() {
        if (numbits != word_size) {
            dyna.~unique_ptr<bitset<word_size>[]>();
        }
    }

    size_type size() const {
        return numbits;
    }

    void resize(size_type ns) {
        if (ns > numbits) {
            auto count = (ns + word_size - 1u) / word_size;
            auto newlen = count * word_size;
            auto newptr = make_unique<bitset<word_size>[]>(count);
            if (numbits == word_size) {
                newptr[0] = sdo;
            } else {
                copy(dyna.get(), dyna.get() + (numbits / word_size), newptr.get());
                fill(newptr.get() + (numbits / word_size), newptr.get() + (newlen / word_size), 0);
            }
            dyna = move(newptr);
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
            fill(dyna.get(), dyna.get() + numbits / word_size, 0);
        }
    }

private:
    union {
        bitset<word_size> sdo;
        unique_ptr<bitset<word_size>[]> dyna;
    };
    size_type numbits;
};

// Entity

struct entity {
    dynamic_bitset components = {};
};

// Queries

/*! Has component
 *
 * When used as a visitor parameter, applies the matching logic for the parameter, but does not load the component.
 */
template <typename T>
struct require {};

/*! Visitor parameter inverter
 *
 * When used as a visitor parameter, inverts the matching logic for that parameter.
 */
template <typename T>
struct deny {};

/*! Tag component
 *
 * When a tag component is stored in an entity, only the fact that it exists is recorded, no data is stored.
 */
template <typename T>
struct tag {};

/*! Optional component
 *
 * When used as a visitor parameter, applies the matching logic for the parameter, but does not cause the visit to fail.
 *
 * Provides pointer-like access to the parameter.
 */
template <typename T>
class optional {
public:
    optional()
        : com(nullptr) {}
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
    template <typename DB, typename EntID, typename... Components>
    friend struct applier;
    optional(T* c)
        : com(c) {}
    T* com;
};

/*! Optional Tag component
 *
 * When used as a visitor parameter, applies the matching logic for the parameter, but does not cause the visit to fail.
 */
template <typename T>
class optional<tag<T>> {
public:
    optional()
        : tag(false) {}
    optional(const optional&) = default;
    optional(optional&&) = default;
    optional& operator=(const optional&) = default;
    optional& operator=(optional&&) = default;
    explicit operator bool() const {
        return tag;
    }

private:
    template <typename DB, typename EntID, typename... Components>
    friend struct applier;
    optional(bool t)
        : tag(t) {}
    bool tag;
};

// ComponentTraits

struct component_tags {
    struct positive {};
    struct normal : positive {};
    struct noload : positive {};
    struct tagged : positive {};
    struct meta {};
    struct nofail : meta {};
    struct inverted : meta {};
    struct eid : meta {};
};

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

// Applier

template <typename DB, typename PrimaryComponent, typename... Components>
struct applier;

template <typename DB, typename PrimaryComponent, typename HeadCom, typename... TailComs>
struct applier<DB, PrimaryComponent, HeadCom, TailComs...> {
    using ent_id = typename DB::ent_id;
    using com_id = typename DB::com_id;

    template <typename Traits, typename Visitor, typename... Args>
    static void helper(component_tags::normal, DB& db, ent_id eid, com_id primary_cid, Visitor&& visitor, Args&&... args) {
        auto& com = db.template get_component<typename Traits::component>(eid);
        return applier<DB, PrimaryComponent, TailComs...>::try_apply(db, eid, primary_cid, std::forward<Visitor>(visitor), std::forward<Args>(args)..., com);
    }

    template <typename Traits, typename Visitor, typename... Args>
    static void helper(component_tags::noload, DB& db, ent_id eid, com_id primary_cid, Visitor&& visitor, Args&&... args) {
        if (db.template has_component<typename Traits::component>(eid)) {
            return applier<DB, PrimaryComponent, TailComs...>::try_apply(db, eid, primary_cid, std::forward<Visitor>(visitor), std::forward<Args>(args)..., require<typename Traits::component>{});
        }
    }

    template <typename Traits, typename Visitor, typename... Args>
    static void helper(component_tags::inverted, DB& db, ent_id eid, com_id primary_cid, Visitor&& visitor, Args&&... args) {
        using inner_traits = component_traits<DB, typename Traits::component>;
        return inverted_helper<Traits>(typename inner_traits::category{}, db, eid, primary_cid, std::forward<Visitor>(visitor), std::forward<Args>(args)...);
    }

    template <typename Traits, typename Visitor, typename... Args>
    static void inverted_helper(component_tags::positive, DB& db, ent_id eid, com_id primary_cid, Visitor&& visitor, Args&&... args) {
        if (!db.template has_component<typename Traits::component>(eid)) {
            return applier<DB, PrimaryComponent, TailComs...>::try_apply(db, eid, primary_cid, std::forward<Visitor>(visitor), std::forward<Args>(args)..., deny<typename Traits::component>{});
        }
    }

    template <typename Traits, typename Visitor, typename... Args>
    static void inverted_helper(component_tags::inverted, DB& db, ent_id eid, com_id primary_cid, Visitor&& visitor, Args&&... args) {
        using inner_traits = component_traits<DB, typename Traits::component>;
        using next_traits = component_traits<DB, typename inner_traits::component>;
        return helper<next_traits>(typename next_traits::category{}, db, eid, primary_cid, std::forward<Visitor>(visitor), std::forward<Args>(args)...);
    }

    template <typename Traits, typename Visitor, typename... Args>
    static void helper(component_tags::tagged, DB& db, ent_id eid, com_id primary_cid, Visitor&& visitor, Args&&... args) {
        if (db.template has_component<typename Traits::component>(eid)) {
            return applier<DB, PrimaryComponent, TailComs...>::try_apply(db, eid, primary_cid, std::forward<Visitor>(visitor), std::forward<Args>(args)..., typename Traits::component{});
        }
    }

    template <typename Traits, typename Visitor, typename... Args>
    static void helper(component_tags::nofail, DB& db, ent_id eid, com_id primary_cid, Visitor&& visitor, Args&&... args) {
        using InnerTraits = component_traits<DB, typename Traits::component>;
        return nofail_helper<Traits>(typename InnerTraits::category{}, db, eid, primary_cid, std::forward<Visitor>(visitor), std::forward<Args>(args)...);
    }

    template <typename Traits, typename Visitor, typename... Args>
    static void nofail_helper(component_tags::normal, DB& db, ent_id eid, com_id primary_cid, Visitor&& visitor, Args&&... args) {
        if (db.template has_component<typename Traits::component>(eid)) {
            auto& com = db.template get_component<typename Traits::component>(eid);
            return applier<DB, PrimaryComponent, TailComs...>::try_apply(db, eid, primary_cid, std::forward<Visitor>(visitor), std::forward<Args>(args)..., optional<typename Traits::component>(&com));
        } else {
            return applier<DB, PrimaryComponent, TailComs...>::try_apply(db, eid, primary_cid, std::forward<Visitor>(visitor), std::forward<Args>(args)..., optional<typename Traits::component>(nullptr));
        }
    }

    template <typename Traits, typename Visitor, typename... Args>
    static void nofail_helper(component_tags::tagged, DB& db, ent_id eid, com_id primary_cid, Visitor&& visitor, Args&&... args) {
        if (db.template has_component<typename Traits::component>(eid)) {
            return applier<DB, PrimaryComponent, TailComs...>::try_apply(db, eid, primary_cid, std::forward<Visitor>(visitor), std::forward<Args>(args)..., optional<typename Traits::component>(true));
        } else {
            return applier<DB, PrimaryComponent, TailComs...>::try_apply(db, eid, primary_cid, std::forward<Visitor>(visitor), std::forward<Args>(args)..., optional<typename Traits::component>(false));
        }
    }

    template <typename Traits, typename Visitor, typename... Args>
    static void helper(component_tags::eid, DB& db, ent_id eid, com_id primary_cid, Visitor&& visitor, Args&&... args) {
        return applier<DB, PrimaryComponent, TailComs...>::try_apply(db, eid, primary_cid, std::forward<Visitor>(visitor), std::forward<Args>(args)..., eid);
    }

    template <typename Visitor, typename... Args>
    static void try_apply(DB& db, ent_id eid, com_id primary_cid, Visitor&& visitor, Args&&... args) {
        using Traits = component_traits<DB, HeadCom>;
        return helper<Traits>(typename Traits::category{}, db, eid, primary_cid, std::forward<Visitor>(visitor), std::forward<Args>(args)...);
    }
};

template <typename DB, typename PrimaryComponent, typename... TailComs>
struct applier<DB, PrimaryComponent, PrimaryComponent, TailComs...> {
    using ent_id = typename DB::ent_id;
    using com_id = typename DB::com_id;

    template <typename Visitor, typename... Args>
    static void try_apply(DB& db, ent_id eid, com_id primary_cid, Visitor&& visitor, Args&&... args) {
        using Traits = component_traits<DB, PrimaryComponent>;
        auto& com = db.template get_component_by_id<typename Traits::component>(primary_cid);
        return applier<DB, PrimaryComponent, TailComs...>::try_apply(db, eid, primary_cid, std::forward<Visitor>(visitor), std::forward<Args>(args)..., com);
    }
};

template <typename DB, typename PrimaryComponent>
struct applier<DB, PrimaryComponent> {
    using ent_id = typename DB::ent_id;
    using com_id = typename DB::com_id;

    template <typename Visitor, typename... Args>
    static void try_apply(DB&, ent_id, com_id, Visitor&& visitor, Args&&... args) {
        std::forward<Visitor>(visitor)(std::forward<Args>(args)...);
    }
};

template <typename DB, typename HeadCom, typename... TailComs>
struct applier<DB, void, HeadCom, TailComs...> {
    using ent_id = typename DB::ent_id;
    using com_id = typename DB::com_id;

    template <typename Traits, typename Visitor, typename... Args>
    static void helper(component_tags::normal, DB& db, ent_id eid, Visitor&& visitor, Args&&... args) {
        auto& com = db.template get_component<typename Traits::component>(eid);
        return applier<DB, void, TailComs...>::try_apply(db, eid, std::forward<Visitor>(visitor), std::forward<Args>(args)..., com);
    }

    template <typename Traits, typename Visitor, typename... Args>
    static void helper(component_tags::noload, DB& db, ent_id eid, Visitor&& visitor, Args&&... args) {
        if (db.template has_component<typename Traits::component>(eid)) {
            return applier<DB, void, TailComs...>::try_apply(db, eid, std::forward<Visitor>(visitor), std::forward<Args>(args)..., require<typename Traits::component>{});
        }
    }

    template <typename Traits, typename Visitor, typename... Args>
    static void helper(component_tags::inverted, DB& db, ent_id eid, Visitor&& visitor, Args&&... args) {
        using InnerTraits = component_traits<DB, typename Traits::component>;
        return inverted_helper<Traits>(typename InnerTraits::category{}, db, eid, std::forward<Visitor>(visitor), std::forward<Args>(args)...);
    }

    template <typename Traits, typename Visitor, typename... Args>
    static void inverted_helper(component_tags::positive, DB& db, ent_id eid, Visitor&& visitor, Args&&... args) {
        if (!db.template has_component<typename Traits::component>(eid)) {
            return applier<DB, void, TailComs...>::try_apply(db, eid, std::forward<Visitor>(visitor), std::forward<Args>(args)..., deny<typename Traits::component>{});
        }
    }

    template <typename Traits, typename Visitor, typename... Args>
    static void inverted_helper(component_tags::inverted, DB& db, ent_id eid, Visitor&& visitor, Args&&... args) {
        using InnerTraits = component_traits<DB, typename Traits::component>;
        using NextTraits = component_traits<DB, typename InnerTraits::component>;
        return helper<NextTraits>(typename NextTraits::category{}, db, eid, std::forward<Visitor>(visitor), std::forward<Args>(args)...);
    }

    template <typename Traits, typename Visitor, typename... Args>
    static void helper(component_tags::tagged, DB& db, ent_id eid, Visitor&& visitor, Args&&... args) {
        if (db.template has_component<typename Traits::component>(eid)) {
            return applier<DB, void, TailComs...>::try_apply(db, eid, std::forward<Visitor>(visitor), std::forward<Args>(args)..., typename Traits::component{});
        }
    }

    template <typename Traits, typename Visitor, typename... Args>
    static void helper(component_tags::nofail, DB& db, ent_id eid, Visitor&& visitor, Args&&... args) {
        using InnerTraits = component_traits<DB, typename Traits::component>;
        return nofail_helper<Traits>(typename InnerTraits::category{}, db, eid, std::forward<Visitor>(visitor), std::forward<Args>(args)...);
    }

    template <typename Traits, typename Visitor, typename... Args>
    static void nofail_helper(component_tags::normal, DB& db, ent_id eid, Visitor&& visitor, Args&&... args) {
        if (db.template has_component<typename Traits::component>(eid)) {
            auto& com = db.template get_component<typename Traits::component>(eid);
            return applier<DB, void, TailComs...>::try_apply(db, eid, std::forward<Visitor>(visitor), std::forward<Args>(args)..., optional<typename Traits::component>(&com));
        } else {
            return applier<DB, void, TailComs...>::try_apply(db, eid, std::forward<Visitor>(visitor), std::forward<Args>(args)..., optional<typename Traits::component>(nullptr));
        }
    }

    template <typename Traits, typename Visitor, typename... Args>
    static void nofail_helper(component_tags::tagged, DB& db, ent_id eid, Visitor&& visitor, Args&&... args) {
        if (db.template has_component<typename Traits::component>(eid)) {
            return applier<DB, void, TailComs...>::try_apply(db, eid, std::forward<Visitor>(visitor), std::forward<Args>(args)..., optional<typename Traits::component>(true));
        } else {
            return applier<DB, void, TailComs...>::try_apply(db, eid, std::forward<Visitor>(visitor), std::forward<Args>(args)..., optional<typename Traits::component>(false));
        }
    }

    template <typename Traits, typename Visitor, typename... Args>
    static void helper(component_tags::eid, DB& db, ent_id eid, Visitor&& visitor, Args&&... args) {
        return applier<DB, void, TailComs...>::try_apply(db, eid, std::forward<Visitor>(visitor), std::forward<Args>(args)..., eid);
    }

    template <typename Visitor, typename... Args>
    static void try_apply(DB& db, ent_id eid, Visitor&& visitor, Args&&... args) {
        using Traits = component_traits<DB, HeadCom>;
        return helper<Traits>(typename Traits::category{}, db, eid, std::forward<Visitor>(visitor), std::forward<Args>(args)...);
    }
};

template <typename DB>
struct applier<DB, void> {
    using ent_id = typename DB::ent_id;
    using com_id = typename DB::com_id;

    template <typename Visitor, typename... Args>
    static void try_apply(DB& db, ent_id eid, Visitor&& visitor, Args&&... args) {
        std::forward<Visitor>(visitor)(std::forward<Args>(args)...);
    }
};

// VisitorKey

template <typename DB, typename... Components>
struct visitor_key;

template <typename DB, typename HeadCom, typename... TailComs>
struct visitor_key<DB, HeadCom, TailComs...> {
    using ent_id = typename DB::ent_id;
    using traits = component_traits<DB, HeadCom>;

    static bool helper(DB& db, ent_id eid, component_tags::positive) {
        return visitor_key<DB, TailComs...>::check(db, eid) && db.template has_component<typename traits::component>(eid);
    }

    static bool helper(DB& db, ent_id eid, component_tags::meta) {
        return visitor_key<DB, TailComs...>::check(db, eid);
    }

    static bool check(DB& db, ent_id eid) {
        return helper(db, eid, typename traits::category{});
    }
};

template <typename DB>
struct visitor_key<DB> {
    using ent_id = typename DB::ent_id;
    static bool check(DB&, ent_id) {
        return true;
    }
};

// First

template <typename T, typename... Ts>
struct first {
    using type = T;
};

template <typename... Ts>
using first_t = typename first<Ts...>::type;

// GetPrimary

template <typename T>
struct primary {
    using type = T;
};

template <typename DB, typename HeadTag, typename... Components>
struct get_primary;

template <typename DB, typename... Components>
using get_primary_t = typename get_primary<DB, typename component_traits<DB, first_t<Components...>>::category, Components...>::type;

template <typename DB, typename HeadTag, typename HeadCom, typename... Components>
struct get_primary<DB, HeadTag, HeadCom, Components...> {
    using type = get_primary_t<DB, Components...>;
};

template <typename DB, typename HeadTag, typename HeadCom>
struct get_primary<DB, HeadTag, HeadCom> {
    using type = primary<void>;
};

template <typename DB, typename HeadCom, typename... Components>
struct get_primary<DB, component_tags::normal, HeadCom, Components...> {
    using type = primary<HeadCom>;
};

template <typename DB, typename HeadCom>
struct get_primary<DB, component_tags::normal, HeadCom> {
    using type = primary<HeadCom>;
};

// FindOther

template <typename DB, typename T, bool Positive, typename... Ts>
struct find_other;

template <typename DB, typename T, typename... Ts>
using find_other_t = typename find_other<DB, T, is_base_of<component_tags::positive, typename component_traits<DB, first_t<Ts...>>::category>::value, Ts...>::type;

template <typename DB, typename T, typename U, typename... Ts>
struct find_other<DB, T, true, U, Ts...> {
    using type = true_type;
};

template <typename DB, typename T, typename... Ts>
struct find_other<DB, T, true, T, Ts...> {
    using type = find_other_t<DB, T, Ts...>;
};

template <typename DB, typename T, typename U, typename... Ts>
struct find_other<DB, T, false, U, Ts...> {
    using type = find_other_t<DB, T, Ts...>;
};

template <typename DB, typename T, typename U>
struct find_other<DB, T, false, U> {
    using type = false_type;
};

template <typename DB, typename T>
struct find_other<DB, T, true, T> {
    using type = false_type;
};

template <typename DB, typename T>
struct find_other<DB, T, false, T> {};

// VisitorTraits

template <typename DB, typename... Components>
struct visitor_traits_impl {
    using ent_id = typename DB::ent_id;
    using com_id = typename DB::com_id;
    using key = visitor_key<DB, Components...>;
    using primary_component = get_primary_t<DB, Components...>;
    using has_other_components = find_other_t<DB, typename primary_component::type, Components...>;

    template <typename Visitor>
    static void apply(DB& db, ent_id eid, Visitor&& visitor) {
        applier<DB, typename primary_component::type, Components...>::try_apply(db, eid, std::forward<Visitor>(visitor));
    }

    template <typename Visitor>
    static void apply(DB& db, ent_id eid, com_id primary_cid, Visitor&& visitor) {
        applier<DB, typename primary_component::type, Components...>::try_apply(db, eid, primary_cid, std::forward<Visitor>(visitor));
    }
};

template <typename DB, typename Visitor>
struct visitor_traits : visitor_traits<DB, decltype(&decay_t<Visitor>::operator())> {};

template <typename DB, typename R, typename... Ts>
struct visitor_traits<DB, R (&)(Ts...)> : visitor_traits_impl<DB, std::decay_t<Ts>...> {};

template <typename DB, typename Visitor, typename R, typename... Ts>
struct visitor_traits<DB, R (Visitor::*)(Ts...)> : visitor_traits_impl<DB, std::decay_t<Ts>...> {};

template <typename DB, typename Visitor, typename R, typename... Ts>
struct visitor_traits<DB, R (Visitor::*)(Ts...) const> : visitor_traits_impl<DB, std::decay_t<Ts>...> {};

template <typename DB, typename Visitor, typename R, typename... Ts>
struct visitor_traits<DB, R (Visitor::*)(Ts...)&> : visitor_traits_impl<DB, std::decay_t<Ts>...> {};

template <typename DB, typename Visitor, typename R, typename... Ts>
struct visitor_traits<DB, R (Visitor::*)(Ts...) const &> : visitor_traits_impl<DB, std::decay_t<Ts>...> {};

template <typename DB, typename Visitor, typename R, typename... Ts>
struct visitor_traits<DB, R (Visitor::*)(Ts...) &&> : visitor_traits_impl<DB, std::decay_t<Ts>...> {};

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

    void assign(size_type entid, T com) {
        if (entid >= entid_to_comid.size()) {
            entid_to_comid.resize(entid + 1, -1);
        }

        auto comid = comid_to_entid.size();

        entid_to_comid[entid] = comid;
        comid_to_entid.push_back(entid);
        components.push_back(move(com));
    }

    virtual void remove(size_type entid) override final {
        auto last = comid_to_entid.size() - 1;
        auto comid = entid_to_comid[entid];

        entid_to_comid[entid] = -1;
        entid_to_comid[comid_to_entid[last]] = comid;

        comid_to_entid[comid] = comid_to_entid[last];
        comid_to_entid.pop_back();

        components[comid] = move(components[last]);
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
    vector<size_type> entid_to_comid;
    vector<size_type> comid_to_entid;
    vector<T> components;
};

template <typename T>
class component_set_impl<tag<T>> final : public component_set {
public:
    virtual ~component_set_impl() = default;
    virtual void remove(size_type entid) override final {}
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
    using ent_id = size_t;

    /*! Component ID.
     */
    using com_id = size_t;

    // Entity functions

    /*! Creates a new Entity.
     *
     * Creates a new Entity that has no components.
     *
     * @return EntID of the new Entity.
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
     * @param eid EntID of the Entity to erase.
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

    // Component functions

    /*! Create new component.
     *
     * Creates a new component from the given value and associates it with
     * the given Entity.
     * If a component of the same type already exists, it will be
     * overwritten.
     *
     * @param eid Entity to attach new component to.
     * @param com Component value.
     */
    template <typename T>
    void create_component(ent_id eid, T&& com) {
        auto guid = get_type_guid<T>();

        auto& ent_coms = entities[eid].components;

        auto& com_set = get_or_create_com_set<T>();

        if (guid < ent_coms.size() && ent_coms.get(guid)) {
            auto cid = com_set.get_comid(eid);
            com_set.get_com(cid) = forward<T>(com);
        } else {
            com_set.assign(eid, forward<T>(com));
            ent_coms.set(guid);
        }
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
     * @param eid EntID of the entity.
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
     * @param eid EntID of the entity.
     * @return Reference to the component.
     */
    template <typename Com>
    Com& get_component(ent_id eid) {
        auto& com_set = *get_com_set<Com>();
        auto cid = com_set.get_comid(eid);
        return com_set.get_com(cid);
    }

    /*! Checks if an entity has a component.
     *
     * Returns whether or not the entity has a component of the
     * associated type.
     *
     * @tparam Com Type of the component to check.
     *
     * @param eid EntID of the entity.
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
     * - Component Require: `require<T>` value, matches entities that have component `T`, but does not load the component.
     * - Component Optional: `optional<T>` value, checks if a component exists, and provides a way to access it, does not fail.
     * - Inverted: `deny<T>` value, matches entities that do *not* match component `T`.
     * - EntID: Matches all entities, provides the `ent_id` of the current entity.
     *
     * Component Data and Maybe parameters will refer to the entity's matching component.
     * EntID parameters will contain the entity's EntID.
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
        using tratis = visitor_traits<database, Visitor>;
        using primary_component = typename tratis::primary_component;

        return visit_helper(forward<Visitor>(visitor), primary_component{});
    }

    // status functions

    /*! Get the number of entities in the Database.
     *
     * @return Number of entities in the Database.
     */
    auto size() const {
        return entities.size() - free_entities.size();
    }

private:
    template <typename DB, typename PrimaryComponent, typename... Components>
    friend struct applier;

    template <typename Com>
    Com& get_component_by_id(com_id cid) {
        auto& com_set = *get_com_set<Com>();
        return com_set.get_com(cid);
    }

    template <typename Com>
    component_set_impl<Com>* get_com_set() {
        auto guid = get_type_guid<Com>();
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
            com_set = make_unique<component_set_impl<Com>>();
        }
        auto com_set_impl = static_cast<component_set_impl<Com>*>(com_set.get());
        return *com_set_impl;
    }

    template <typename Visitor, typename Component>
    void visit_helper(Visitor&& visitor, primary<Component>) {
        using traits = visitor_traits<database, Visitor>;
        using has_other_components = typename traits::has_other_components;

        return visit_helper_primary(forward<Visitor>(visitor), primary<Component>{}, has_other_components{});
    }

    template <typename Visitor, typename Component>
    void visit_helper_primary(Visitor&& visitor, primary<Component>, true_type) {
        using traits = visitor_traits<database, Visitor>;
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

    template <typename Visitor, typename Component>
    void visit_helper_primary(Visitor&& visitor, primary<Component>, false_type) {
        using traits = visitor_traits<database, Visitor>;

        if (auto com_set_ptr = get_com_set<Component>()) {
            auto& com_set = *com_set_ptr;

            for (com_id cid = 0; cid < com_set.size(); ++cid) {
                auto eid = com_set.get_entid(cid);
                if (entities[eid].components.get(0)) {
                    traits::apply(*this, eid, cid, visitor);
                }
            }
        }
    }

    template <typename Visitor>
    void visit_helper(Visitor&& visitor, primary<void>) {
        using traits = visitor_traits<database, Visitor>;
        using key = typename traits::key;

        for (auto eid = 0; eid < entities.size(); ++eid) {
            if (key::check(*this, eid)) {
                traits::apply(*this, eid, visitor);
            }
        }
    }

    vector<entity> entities;
    vector<ent_id> free_entities;
    vector<unique_ptr<component_set>> component_sets;
};

} // namespace _detail

using _detail::database;
using _detail::require;
using _detail::optional;
using _detail::deny;
using _detail::tag;

} // namespace Ginseng

#endif // GINSENG_GINSENG_HPP
