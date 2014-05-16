#ifndef GINSENG_HPP
#define GINSENG_HPP

#include <cstdint>
#include <algorithm>
#include <memory>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <map>
#include <vector>

namespace Ginseng {

namespace _detail {
    using namespace ::std;

// Base IDs

    using EID = int_fast64_t; // Entity ID
    using CID = int_fast64_t; // Component ID
    using TID = int_fast64_t; // Table ID

// TODO C++14 Buffer

    template<class T> struct _Unique_if {
        typedef unique_ptr<T> _Single_object;
    };

    template<class T> struct _Unique_if<T[]> {
        typedef unique_ptr<T[]> _Unknown_bound;
    };

    template<class T, size_t N> struct _Unique_if<T[N]> {
        typedef void _Known_bound;
    };

    template<class T, class... Args>
        typename _Unique_if<T>::_Single_object
        make_unique(Args&&... args) {
            return unique_ptr<T>(new T(forward<Args>(args)...));
        }

    template<class T>
        typename _Unique_if<T>::_Unknown_bound
        make_unique(size_t n) {
            typedef typename remove_extent<T>::type U;
            return unique_ptr<T>(new U[n]());
        }

    template<class T, class... Args>
        typename _Unique_if<T>::_Known_bound
        make_unique(Args&&...) = delete;

    template <bool B, typename T = void>
    using enable_if_t = typename enable_if<B,T>::type;

    template <size_t N, typename T>
    using tuple_element_t = typename tuple_element<N,T>::type;

    template <typename T>
    using remove_pointer_t = typename remove_pointer<T>::type;

// Type Info

    inline TID nextTID()
    {
        static TID tid = 0;
        return ++tid;
    }

// Component Tags

    template <typename... Ts>
    struct Not
    {};

// Traits

    // IntList

        template <int... Is>
        struct IntList
        {};

        // MakeIntList

            template <int I, int... S>
            struct MakeIntList;

            template <int... I>
            using MakeIntList_t = typename MakeIntList<I...>::type;

            template <int I, int... S>
            struct MakeIntList
            {
                using type = MakeIntList_t<I-1, I-1, S...>;
            };

            template <int... S>
            struct MakeIntList<0, S...>
            {
                using type = IntList<S...>;
            };

    // TupleTail

        template <typename T>
        struct TupleTail;

        template <typename T>
        using TupleTail_t = typename TupleTail<T>::type;

        template <template <typename...> class Tup, typename H, typename... T>
        struct TupleTail<Tup<H,T...>>
        {
            using type = Tup<T...>;
        };

    // refTupleTail

        template <typename T, template <int...> class IL, int I, int... Is>
        static auto _refTupleTail(T& tup, IL<I,Is...>)
        -> decltype(tie(get<Is>(tup)...))
        {
            return tie(get<Is>(tup)...);
        }

        template <typename T>
        auto refTupleTail(T& t)
        -> decltype(_refTupleTail(t, MakeIntList_t<tuple_size<T>::value>{}))
        {
            return _refTupleTail(t, MakeIntList_t<tuple_size<T>::value>{});
        }

    template <typename... Ts>
    struct Count
        : integral_constant<int, sizeof...(Ts)>
    {};

    template <typename... Ts>
    struct TypeList
    {};

    // TypeListCat

        template <typename T, typename U>
        struct TypeListCat;

        template <typename... Ts>
        using TypeListCat_t = typename TypeListCat<Ts...>::type;

        template <typename... Ts, typename... Us>
        struct TypeListCat<TypeList<Ts...>, TypeList<Us...>>
        {
            using type = TypeList<Ts..., Us...>;
        };

    // TypeListSize

        template <typename>
        struct TypeListSize;

        template <typename... Ts>
        struct TypeListSize<TypeList<Ts...>>
            : Count<Ts...>
        {};

    // IsNot

        template <typename>
        struct IsNot;

        template <typename T>
        struct IsNot
            : false_type
        {};

        template <typename... Ts>
        struct IsNot<Not<Ts...>>
            : true_type
        {};

    // RemoveNot

        template <typename...>
        struct RemoveNot;

        template <typename... Ts>
        using RemoveNot_t = typename RemoveNot<Ts...>::type;

        template <typename T, typename... Us>
        struct RemoveNot<T, Us...>
        {
            using type = TypeListCat_t<TypeList<T>, RemoveNot_t<Us...>>;
        };

        template <typename... Ts, typename... Us>
        struct RemoveNot<Not<Ts...>, Us...>
        {
            using type = RemoveNot_t<Us...>;
        };

        template <>
        struct RemoveNot<>
        {
            using type = TypeList<>;
        };

    // GetNots

        template <typename...>
        struct GetNots;

        template <typename... Ts>
        using GetNots_t = typename GetNots<Ts...>::type;

        template <typename T, typename... Us>
        struct GetNots<T, Us...>
        {
            using type = GetNots_t<Us...>;
        };

        template <typename ...Ts, typename... Us>
        struct GetNots<Not<Ts...>, Us...>
        {
            using type = TypeListCat_t<TypeList<Ts...>, GetNots_t<Us...>>;
        };

        template <>
        struct GetNots<>
        {
            using type = TypeList<>;
        };

    // ExpandTags

        template <typename...>
        struct ExpandTags;

        template <typename... Ts>
        using ExpandTags_t = typename ExpandTags<Ts...>::type;

        template <typename T, typename... Us>
        struct ExpandTags<T, Us...>
        {
            using type = TypeListCat_t<TypeList<T>, ExpandTags_t<Us...>>;
        };

        template <typename... Ts, typename... Us>
        struct ExpandTags<Not<Ts...>, Us...>
        {
            using type = TypeListCat_t<TypeList<Ts...>, ExpandTags_t<Us...>>;
        };

        template <>
        struct ExpandTags<>
        {
            using type = TypeList<>;
        };

    // AddPointer

        template <typename...>
        struct AddPointer;

        template <typename... Ts>
        using AddPointer_t = typename AddPointer<Ts...>::type;

        template <template <typename...> class Tup, typename... Ts>
        struct AddPointer<Tup<Ts...>>
        {
            using type = Tup<Ts*...>;
        };

    // ToTuple

        template <typename...>
        struct ToTuple;

        template <typename... Ts>
        using ToTuple_t = typename ToTuple<Ts...>::type;

        template <typename... Ts>
        struct ToTuple<TypeList<Ts...>>
        {
            using type = tuple<Ts...>;
        };

    // Filters

        // IsOneOf

            template <typename, typename...>
            struct IsOneOf;

            template <typename T, typename U, typename... Vs>
            struct IsOneOf<T, U, Vs...>
                : IsOneOf<T, Vs...>::value
            {};

            template <typename T, typename... Vs>
            struct IsOneOf<T, T, Vs...>
                : true_type
            {};

            template <typename T>
            struct IsOneOf<T>
                : false_type
            {};

// Containers

    template <typename T, typename H = hash<T>>
    using Many = unordered_set<T, H>;

    template <typename K, typename V, typename H = hash<K>>
    using Table = unordered_map<K, V, H>;

    template <typename K, typename V>
    using SlowTable = map<K, V>;

    template <typename T>
    using Vec = vector<T>;

    // Quick Erasure

        struct ErasureBase
        {
            virtual ~ErasureBase() = 0;
        };

        inline ErasureBase::~ErasureBase()
        {}

        template <typename T>
        struct Erase
            : ErasureBase
        {
            T t;

            template <typename... Us>
            Erase(Us&&... us)
                : t(forward<Us>(us)...)
            {}
        };

// Component Base

    class ComponentBase
    {
    public:
        ComponentBase() = default;
        ComponentBase(ComponentBase const&) = default;
        ComponentBase(ComponentBase&&) = default;
        ComponentBase& operator=(ComponentBase const&) = default;
        ComponentBase& operator=(ComponentBase&&) = default;

        virtual ~ComponentBase() = 0;
    };

    inline ComponentBase::~ComponentBase()
    {}

// Component CRTP

    template <typename Child>
    class Component
        : public ComponentBase
    {
    public:
        static TID getTID()
        {
            static TID tid = nextTID();
            return tid;
        }

        shared_ptr<Child> child;

        Component(shared_ptr<Child> sptr)
            : child(move(sptr))
        {}
    };

// Types

    class Database;

    namespace Types
    {
        template <typename T>
        struct EntityData
        {
            Table<TID, typename Table<CID,T>::iterator> coms;
        };

        template <typename T>
        class Entity;

        template <typename T>
        typename Entity<T>::Iter getIter(Entity<T> ent);

        template <typename T>
        class Entity
        {
            friend class _detail::Database;
            using Iter = typename Table<EID,EntityData<T>>::iterator;

            friend Iter getIter<>(Entity);

            Database* database;
            Iter iter;

            Entity(Database* db, Iter i) : database(db), iter(i) {}

        public:

            struct Hash
            {
                hash<EID>::result_type operator()(Entity const& e) const
                {
                    return hash<EID>{}(e.getID());
                }
            };

            Entity() = default;

            bool operator<(Entity const& e) const
            {
                return (iter->first < e.iter->first);
            }

            bool operator==(Entity const& e) const
            {
                return (iter->first == e.iter->first);
            }

            EID getID() const
            {
                return iter->first;
            }

            Database* getDB() const
            {
                return database;
            }
        };

        template <typename T>
        typename Entity<T>::Iter getIter(Entity<T> ent)
        {
            return ent.iter;
        }

        struct ComponentData
        {
            unique_ptr<Entity<ComponentData>> ent; // Disgusting, this shouldn't have to be a pointer.
            unique_ptr<ComponentBase> com;
            ComponentData(Entity<ComponentData> const& e, unique_ptr<ComponentBase>&& c)
                : ent(new Entity<ComponentData>(e))
                , com(move(c))
            {}
        };

        struct ComponentTable
        {
            Table<CID,ComponentData> coms;
            Many<Entity<ComponentData>, Entity<ComponentData>::Hash> ents;
        };
    }

    using Entity         = Types::Entity<Types::ComponentData>;
    using EntityData     = Types::EntityData<Types::ComponentData>;
    using ComponentData  = Types::ComponentData;
    using ComponentTable = Types::ComponentTable;

// Query results

    template <typename... Ts>
    using RElement =
        ToTuple_t
        <
            TypeListCat_t
            <
                TypeList<Entity>,
                AddPointer_t< RemoveNot_t<Ts...> >
            >
        >;

    template <typename... Ts>
    using Result = vector<RElement<Ts...>>;

    // fill_memo_vec

        // fill_memo_vec_push

            template <typename>
            struct fill_memo_vec_push;

            template <typename T>
            struct fill_memo_vec_push
            {
                static void proc(Vec<TID>& vt)
                {
                    return vt.emplace_back(Component<T>::getTID());
                }
            };

            template <typename... Ts>
            struct fill_memo_vec_push<Not<Ts...>>
            {
                static void proc(Vec<TID>& vt)
                {
                    return vt.emplace(end(vt), {Component<Ts>::getTID()...});
                }
            };

        template <int, typename...>
        struct fill_memo_vec;

        template <int I, typename T, typename... Us>
        struct fill_memo_vec<I, T, Us...>
        {
            static void proc(Vec<TID>& vt)
            {
                fill_memo_vec_push<T>::proc(vt);
                return fill_memo_vec<I+1, Us...>::proc(vt);
            }
        };

        template <int I>
        struct fill_memo_vec<I>
        {
            static void proc(Vec<TID>& vt)
            {}
        };

    // fill_components

        struct fill_components
        {
            template <int I = 0, typename... Ts>
            static enable_if_t <I < Count<Ts...>::value>
            proc(tuple<Ts*...>& tup, decltype(EntityData::coms) const& etab)
            {
                using C = tuple_element_t<I, tuple<Ts...>>;
                TID tid = Component<C>::getTID();

                auto iter = etab.find(tid);
                if (iter != etab.end())
                {
                    get<I>(tup) = static_cast<Component<C>*>(iter->second->second.com.get())->child.get();
                }

                return proc<I+1>(tup, etab);
            }

            template <int I = 0, typename... Ts>
            static enable_if_t <I >= Count<Ts...>::value>
            proc(tuple<Ts*...>& tup, decltype(EntityData::coms) const& etab)
            {}
        };

    // getComponents

        template <typename... Ts>
        tuple<Ts*...> getComponents(Entity ent)
        {
            tuple<Ts*...> rv;

            fill_components::proc<0>(rv, getIter(ent)->second.coms);

            return rv;
        }

// Main Database engine

    class Database
    {
        Table<EID,EntityData> entities;
        Table<TID,ComponentTable> components;
        SlowTable<Vec<TID>,unique_ptr<ErasureBase>> memos;

        struct
        {
            EID eid = 0;
            CID cid = 0;
        } uidGen;

        EID createEntityID()
        {
            return ++uidGen.eid;
        }

        CID createCID()
        {
            return ++uidGen.cid;
        }

    // Helpers

        // fill_inspect
            // Fills the tuple with components from N by inspecting the entity.

            template <typename U, int N = 2, typename T, int M = N-tuple_size<T>::value>
            enable_if_t<
                (N < tuple_size<T>::value),
            bool> fill_inspect(T& ele) const
            {
                using PtrType = tuple_element_t<N, T>;
                using Type = Component<remove_pointer_t<PtrType>>;

                auto const& coms = get<0>(ele).iter->second.coms;
                auto iter = coms.find(Type::getTID());

                if (iter == coms.end())
                    return false;

                get<N>(ele) = static_cast<Type*>(iter->second->second.com.get())->child.get();

                return fill_inspect<U, N+1>(ele);
            }

            template <typename U, int N = 2, typename T, int M = N-tuple_size<T>::value>
            enable_if_t<
                (N >= tuple_size<T>::value) and
                (M <  tuple_size<U>::value),
            bool> fill_inspect(T& ele) const
            {
                using PtrType = tuple_element_t<M, U>;
                using Type = Component<remove_pointer_t<PtrType>>;

                auto const& coms = get<0>(ele).iter->second.coms;
                auto iter = coms.find(Type::getTID());

                if (iter != coms.end())
                    return false;

                return fill_inspect<U, N+1>(ele);
            }

            template <typename U, int N = 2, typename T, int M = N-tuple_size<T>::value>
            enable_if_t<
                (N >= tuple_size<T>::value) and
                (M >= tuple_size<U>::value),
            bool> fill_inspect(T& ele) const
            {
                return true;
            }

        // select_inspect
            // Gathers all entities containing the first component, forwards to fill_inspect.

            template <typename T, typename... Us>
            Result<T, Us...> select_inspect() const
            {
                static_assert(not IsNot<T>::value, "First component must be positive!");

                Result<T, Us...> rval;

                auto iter = components.find(Component<T>::getTID());
                if (iter == components.end())
                    return rval;

                const ComponentTable& table = iter->second;

                for (auto const& p : table.coms)
                {
                    const ComponentData& cd = p.second;
                    T* data = static_cast<Component<T>*>(cd.com.get())->child.get();

                    RElement<T, Us...> ele;

                    get<0>(ele) = *cd.ent;
                    get<1>(ele) = data;

                    if (fill_inspect<typename ToTuple<typename GetNots<Us...>::type>::type>(ele))
                    {
                        rval.emplace_back(move(ele));
                    }
                }

                return rval;
            }

    public:

        enum class Selector
        {
            INSPECT
        };

        Entity newEntity()
        {
            EID eid = createEntityID();
            return {this, entities.emplace(eid, EntityData{}).first};
        }

        void eraseEntity(Entity ent)
        {
            memos.clear();

            for (auto&& p : ent.iter->second.coms)
            {
                ComponentTable& tab = components.at(p.first);
                tab.coms.erase(tab.coms.find(p.second->first));
                tab.ents.erase(tab.ents.find(ent));
            }

            entities.erase(ent.iter);
        }

        template <typename T>
        T& addComponent(Entity ent, shared_ptr<T> sptr)
        {
            TID tid = Component<T>::getTID();

            for (auto i=memos.begin(), ie=memos.end(); i!=ie;)
            {
                auto a = i->first.begin();
                auto b = i->first.end();
                auto iter = find(a, b, tid);
                if (iter != b) i = memos.erase(i);
                else ++i;
            }

            CID cid = createCID();

            auto ptr = make_unique<Component<T>>(move(sptr));
            T& rval = *ptr->child;

            ComponentTable& table = components[tid];
            ComponentData dat { ent, move(ptr) };

            auto comiter = table.coms.emplace(cid, move(dat)).first;
            table.ents.emplace(ent);

            ent.iter->second.coms.emplace(tid, comiter);

            return rval;
        }

        template <typename... Ts>
        Result<Ts...> const& getEntities(const Selector method = Selector::INSPECT)
        {
            auto unerase = [](decltype(memos.begin()) iter)-> Result<Ts...>&
            {
                return static_cast<Erase<Result<Ts...>>*>(iter->second.get())->t;
            };

            auto sorter = [](RElement<Ts...> const& a, RElement<Ts...> const& b)
            {
                return (refTupleTail(a) < refTupleTail(b));
            };

            Vec<TID> vt;

            vt.reserve(TypeListSize<ExpandTags_t<Ts...>>::value);
            fill_memo_vec<0, Ts...>::proc(vt);
            stable_partition(begin(vt), begin(vt), [](TID tid){return (tid>0);});

            auto iter = memos.find(vt);
            if (iter != memos.end())
                return unerase(iter);

            Result<Ts...> rv;
            unique_ptr<ErasureBase> up;

            switch (method)
            {
                case Selector::INSPECT:
                    rv = select_inspect<Ts...>();
                    sort(begin(rv), end(rv), sorter);
                    up.reset(new Erase<Result<Ts...>>(move(rv)));
                    iter = memos.emplace(move(vt), move(up)).first;
                    break;

                default:
                    throw; //TODO
            }

            return unerase(iter);
        }

        decltype(entities.size()) numEntities() const
        {
            return entities.size();
        }
    };

} // namespace _detail

// Public types

    using _detail::Entity;
    using _detail::Database;
    using _detail::Not;

// Public interface

    using _detail::getComponents;

} // namespace Ginseng

#endif
