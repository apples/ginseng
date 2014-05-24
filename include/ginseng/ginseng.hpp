#ifndef GINSENG_GINSENG_HPP
#define GINSENG_GINSENG_HPP

#include <algorithm>
#include <type_traits>
#include <vector>
#include <list>
#include <iterator>
#include <tuple>
#include <memory>
#include <cstddef>

namespace Ginseng {

namespace _detail {
    
    using namespace std;
    
// Forward Declarations

    template <template <typename> class AllocatorT>
    class Database;
    
// Traits

    // PointerToReference
        
        template <typename T>
        struct PointerToReference;
        
        template <typename T>
        using PointerToReference_t = typename PointerToReference<T>::type;
        
        template <typename T>
        struct PointerToReference<T*>
        {
            using type = T&;
        };
    
    // SFINAE
    
        template <bool B>
        using SFINAE = typename enable_if<B,void>::type;

// IndexList

    template <size_t... Is>
    struct IndexList
    {};
    
    // MakeIndexList
    
        template <size_t N, size_t... Is>
        struct MakeIndexList
        {
            using type = typename MakeIndexList<N-1, N-1, Is...>::type;
        };
        
        template <size_t N>
        using MakeIndexList_t = typename MakeIndexList<N>::type;
        
        template <size_t... Is>
        struct MakeIndexList<0, Is...>
        {
            using type = IndexList<Is...>;
        };

// TypeList

    template <typename... Ts>
    struct TypeList
    {};
    
    // TypeListTuple
    
        template <typename>
        struct TypeListTuple;
        
        template <typename T>
        using TypeListTuple_t = typename TypeListTuple<T>::type;
        
        template <typename... Ts>
        struct TypeListTuple<TypeList<Ts...>>
        {
            using type = tuple<Ts...>;
        };
    
    // TypeListCat
    
        template <typename, typename>
        struct TypeListCat;
        
        template <typename T, typename U>
        using TypeListCat_t = typename TypeListCat<T,U>::type;
        
        template <typename... Ts, typename... Us>
        struct TypeListCat<TypeList<Ts...>, TypeList<Us...>>
        {
            using type = TypeList<Ts..., Us...>;
        };
    
    // TypeListFilter
    
        template <typename T, template <typename> class F, typename = void>
        struct TypeListFilter;
        
        template <typename T, template <typename> class F>
        using TypeListFilter_t = typename TypeListFilter<T,F>::type;
        
        template <typename T, typename... Us, template <typename> class F>
        struct TypeListFilter<TypeList<T,Us...>, F, SFINAE<F<T>::value>>
        {
            using type = TypeListCat_t<TypeList<T>,
                TypeListFilter_t<TypeList<Us...>, F>>;
        };
        
        template <typename T, typename... Us, template <typename> class F>
        struct TypeListFilter<TypeList<T,Us...>, F, SFINAE<!F<T>::value>>
        {
            using type = TypeListFilter_t<TypeList<Us...>, F>;
        };
    
        template <template <typename> class F>
        struct TypeListFilter<TypeList<>, F, void>
        {
            using type = TypeList<>;
        };

    // TypeListImbue
    
        template <typename T, template <typename> class U>
        struct TypeListImbue;
        
        template <typename T, template <typename> class U>
        using TypeListImbue_t = typename TypeListImbue<T,U>::type;
        
        template <typename... Ts, template <typename> class U>
        struct TypeListImbue<TypeList<Ts...>, U>
        {
            using type = TypeList<U<Ts>...>;
        };

// GUID

    using GUID = int_fast64_t;

    inline GUID& instGUID()
    {
        static GUID guid = 0;
        return guid;
    }

    inline GUID nextGUID()
    {
        GUID rv = ++instGUID();
        if (rv == numeric_limits<GUID>::max())
            throw;
        return rv;
    }

    template <typename T>
    GUID getGUID()
    {
        static GUID guid = nextGUID();
        return guid;
    }

// Component
    
    template <typename T>
    class Component
    {
        T val;
        
        Component(Component const&) = delete;
        Component(Component &&) noexcept = delete;
        Component& operator=(Component const&) = delete;
        Component& operator=(Component &&) noexcept = delete;

        public:
        
            Component(T t)
                : val(move(t))
            {}
            
            T& getVal()
            {
                return val;
            }
    };
    
    using AbstractComponent = void;

// GUIDPair

    template <typename T>
    class GUIDPair
    {
        pair<GUID,T> val;
        
        public:
        
            GUIDPair(GUID guid, T t)
                : val(guid, move(t))
            {}
            
            GUID getGUID() const noexcept
            {
                return val.first;
            }
            
            T& getVal() noexcept
            {
                return val.second;
            }
    };
    
    template <typename T>
    constexpr bool operator<(GUIDPair<T>const& a, GUIDPair<T>const& b) noexcept
    {
        return a.getGUID() < b.getGUID();
    }

    template <typename T>
    constexpr bool operator<(GUIDPair<T> const& a, GUID guid) noexcept
    {
        return a.getGUID() < guid;
    }

// Entity

    class Entity
    {
        template <template <typename> class AllocatorT>
        friend class Database;
        
        using ComponentData = GUIDPair<shared_ptr<AbstractComponent>>;
        using ComponentVec = vector<ComponentData>;
        
        ComponentVec components;
        
        public:
        
            Entity() = default;
            Entity(Entity const&) = delete;
            Entity(Entity &&) noexcept = default;
            Entity& operator=(Entity const&) = delete;
            Entity& operator=(Entity &&) = default;
    };

// Queries

    // Not
    
        template <typename... Ts>
        struct Not
        {};
    
        // IsNot
        
            template <typename T>
            struct IsNot : false_type
            {};
            
            template <typename... Ts>
            struct IsNot<Not<Ts...>> : true_type
            {};
        
        // FlattenNots
        
            template <typename T>
            struct FlattenNots;
            
            template <typename T>
            using FlattenNots_t = typename FlattenNots<T>::type;
            
            template <typename... Ts, typename... Us>
            struct FlattenNots<TypeList<Not<Ts...>, Us...>>
            {
                using type = TypeListCat_t<TypeList<Ts...>,
                    FlattenNots_t<TypeList<Us...>>>;
            };
            
            template <>
            struct FlattenNots<TypeList<>>
            {
                using type = TypeList<>;
            };
    
    // IsPositive
    
        template <typename T>
        struct IsPositive : true_type
        {};
        
        template <typename... Ts>
        struct IsPositive<Not<Ts...>> : false_type
        {};
        
    // QueryTraits
    
        template <typename DB, typename... Ts>
        struct QueryTraits
        {
            using types = TypeList<Ts...>;
            using components = TypeListFilter_t<types, IsPositive>;
            
            using EntID = typename DB::EntID;
            using ComID = typename DB::ComID;
            
            template <typename T>
            using Ref = typename add_lvalue_reference<T>::type;
            
            template <typename T>
            using Ptr = T*;
            
            template <typename T>
            using CIDPair = pair<T,ComID>;
            
            using component_refs = TypeListImbue_t<components, Ref>;
            using component_dats = TypeListImbue_t<component_refs, CIDPair>;
            
            using result_item = TypeListCat_t<TypeList<EntID>,component_dats>;
            using result_element = TypeListTuple_t<result_item>;
            
            using result = vector<result_element>;
            
            using nots = FlattenNots_t<TypeListFilter_t<types, IsNot>>;
            
            using component_ptrs = TypeListImbue_t<components, Ptr>;
            using component_ptr_cids = TypeListImbue_t<component_ptrs, CIDPair>;
            
            using ptr_tup = TypeListTuple_t<component_ptr_cids>;
            
            // fillTmp
            
                template <size_t I=0>
                static typename enable_if<
                    I < tuple_size<ptr_tup>::value,
                bool>::type fillTmp(EntID eid, ptr_tup& tmp)
                {
                    using TE = typename tuple_element<I,ptr_tup>::type;
                    using ComPtr = decltype(declval<TE>().first);
                    using Com = typename remove_pointer<ComPtr>::type;
                    
                    get<I>(tmp) = eid.get<Com>();
                    
                    if (!get<I>(tmp).first) return false;
                    
                    return fillTmp<I+1>(eid,tmp);
                }
                
                template <size_t I=0>
                static typename enable_if<
                    I >= tuple_size<ptr_tup>::value,
                bool>::type fillTmp(EntID eid, ptr_tup& tmp)
                {
                    return true;
                }
                
            
            // pushTmp
                
                template <size_t I,
                    typename CPtr = decltype(get<I>(declval<ptr_tup>()).first),
                    typename CRef = PointerToReference_t<CPtr>,
                    typename Pair = pair<CRef,ComID>>
                static Pair pushTmpHelperGet(ptr_tup const& tmp)
                {
                    CRef first = *get<I>(tmp).first;
                    ComID cid = get<I>(tmp).second;
                    
                    return Pair(first,cid);
                }
                
                template <size_t... Is>
                static void pushTmpHelper(EntID eid,
                    ptr_tup const& tmp, result& rv,
                    IndexList<Is...>)
                {
                    rv.emplace_back(eid, pushTmpHelperGet<Is>(tmp)...);
                }
                        
                static void pushTmp(EntID eid, ptr_tup const& tmp, result& rv)
                {
                    using IL = MakeIndexList_t<tuple_size<ptr_tup>::value>;
                    return pushTmpHelper(eid, tmp, rv, IL{});
                }
        };
        
        // QueryResult_t
        
            template <typename DB, typename... Ts>
            using QueryResult_t = typename QueryTraits<DB, Ts...>::result;
        
    // QueryHelper
    
        // getComs
            
            template <typename T>
            struct QueryHelper_getComs;
            
            template <typename... Ts>
            struct QueryHelper_getComs<TypeList<Ts...>>
            {
                template <typename EID>
                static auto getComs(EID eid)
                -> decltype(eid.template getComs<Ts...>())
                {
                    return eid.getComs<Ts...>();
                }
            };
        
        // noNots
        
            template <typename T>
            struct QueryHelper_noNots;
            
            template <typename T, typename... Ts>
            struct QueryHelper_noNots<TypeList<T, Ts...>>
            {
                template <typename EID>
                static bool noNots(EID eid)
                {
                    T* ptr = eid.get<T>().first;
                    if (ptr) return false;
                    return QueryHelper_noNots<TypeList<Ts...>>::noNots(eid);
                }
            };
            
            template <>
            struct QueryHelper_noNots<TypeList<>>
            {
                template <typename EID>
                static bool noNots(EID eid)
                {
                    return true;
                }
            };

/*! Database
 * 
 * An Entity component Database. Uses the given allocator to allocate
 * components, and may also use the same allocator for internal data.
 * 
 * @warning
 * This container does not perform any synchronization. Therefore, it is not
 * considered "thread-safe".
 * 
 * @tparam AllocatorT Component allocator.
 */
template <template <typename> class AllocatorT = allocator>
class Database
{
    template <typename T>
    using AllocList = list<T, AllocatorT<T>>;
    
    AllocList<Entity> entities;
    
    public:
    
    // IDs
        
        class ComID; // forward declaration needed for EntID
        
        /*! Entity ID
         * 
         * A handle to an Entity. Very lightweight.
         */
        class EntID
        {
            friend class Database;
            
            typename AllocList<Entity>::iterator iter;
            
            public:
            
                /*! Query the Entity for a component.
                 * 
                 * Queries the Entity for the given component type.
                 * If found, returns a pair containing a pointer to the
                 * component and a valid ComID handle to the component.
                 * Otherwise, returns `nullptr` and an undefined ComID.
                 * 
                 * @tparam T Explicit type of component.
                 * @return Pair of pointer-to-component and ComID.
                 */
                template <typename T>
                pair<T*,ComID> get() const
                {
                    T* ptr = nullptr;
                    ComID cid;
                    
                    GUID guid = getGUID<T>();
                    auto& comvec = iter->components;
                    
                    auto pos=lower_bound(begin(comvec), end(comvec), guid);
                    
                    cid.eid = *this;
                    
                    if (pos->getGUID() == guid)
                    {
                        cid.iter = pos;
                        ptr = &cid.template cast<T>();
                    }
                    
                    return {ptr,cid};
                }
                
                /*! Query the Entity for multiple components.
                 * 
                 * Returns a tuple containing results equivalent to multiple
                 * calls to get().
                 * 
                 * For example, if `eid.getComs<X,Y,Z>()` is called, it is
                 * equivalent to calling
                 * `std::make_tuple(get<X>(),get<Y>(),get<Z>())`.
                 * 
                 * @tparam Ts Explicit component types.
                 * @return Tuple of results equivalent to get().
                 */
                template <typename... Ts>
                tuple<decltype(get<Ts>())...> getComs() const
                {
                    return {get<Ts>()...};
                }
        };

        /*! Component ID
         * 
         * A handle to a type-erased component. Very lightweight.
         */
        class ComID
        {
            friend class Database;
            
            EntID eid;
            Entity::ComponentVec::iterator iter;
            
            public:
                
                /*! Access component data.
                 * 
                 * Reverses type erasure on the component's data.
                 * The specified type must match the component's real type,
                 * otherwise behaviour is undefined.
                 * 
                 * @tparam Explicit component data type.
                 * @return Reference to component data.
                 */
                template <typename T>
                T& cast() const
                {
                    auto& sptr = iter->getVal();
                    auto ptr = static_cast<Component<T>*>(sptr.get());
                    return ptr->getVal();
                }
                
                /*! Get parent's EntID.
                 * 
                 * Returns a handle to the parent Entity.
                 * 
                 * @return Handle to parent Entity.
                 */
                EntID const& getEID() const
                {
                    return eid;
                }
        };
    
    // Entity functions
        
        /*! Creates a new Entity.
         * 
         * Creates a new Entity that has no components.
         * 
         * @return EntID of the new Entity.
         */
        EntID makeEntity()
        {
            EntID rv;
            rv.iter = entities.emplace(end(entities));
            return rv;
        }
        
        /*! Destroys an Entity.
         * 
         * Destroys the given Entity and all associated components.
         * 
         * @warning
         * All components associated with the Entity are destroyed.
         * This means that all references and ComIDs associated with those
         * components are invalidated.
         * 
         * @param eid EntID of the Entity to erase.
         */
        void eraseEntity(EntID eid)
        {
            entities.erase(eid.iter);
        }
        
        /*! Emplace an Entity into this Database.
         * 
         * Move the given Entity into this Database.
         * 
         * @param ent Entity rvalue to move.
         * @return EntID to the new Entity.
         */
        EntID emplaceEntity(Entity&& ent)
        {
            EntID rv;
            rv.iter = entities.emplace(end(entities), move(ent));
            return rv;
        }
        
        /*! Displace an Entity out of this Database.
         * 
         * Moves the given Entity out of this Database.
         * 
         * @warning
         * All EntIDs associated with the Entity are invalidated.
         * 
         * @param eid EntID to Entity to displace.
         * @return Entity value.
         */
        Entity displaceEntity(EntID eid)
        {
            Entity rv = move(*eid.iter);
            entities.erase(eid.iter);
            return rv;
        }
        
    // Component functions
    
        /*! Create new component.
         * 
         * Creates a new component from the given value and associates it with
         * the given Entity.
         * If a component of the same type already exists, it will be
         * overwritten.
         * 
         * @warning
         * All ComIDs associated with components of the given Entity will be
         * invalidated.
         * 
         * @param eid Entity to attach new component to.
         * @param com Component value.
         * @return Pair of reference-to-component and ComID.
         */
        template <typename T>
        pair<T&,ComID> makeComponent(EntID eid, T com)
        {
            ComID cid;
            GUID guid = getGUID<T>();
            AllocatorT<T> alloc;
            auto& comvec = eid.iter->components;
            
            auto pos = lower_bound(begin(comvec), end(comvec), guid);
            
            cid.eid = eid;
            
            if (pos != end(comvec) && pos->getGUID() == guid)
            {
                cid.iter = pos;
                cid.template cast<T>() = move(com);
            }
            else
            {
                auto ptr = allocate_shared<Component<T>>(alloc, move(com));
                cid.iter = comvec.emplace(pos, guid, move(ptr));
            }
            
            T& comval = cid.template cast<T>();
            
            return pair<T&,ComID>(comval,cid);
        }
        
        /*! Erase a component.
         * 
         * Destroys the given component and disassociates it from its Entity.
         * 
         * @warning
         * All ComIDs associated with components of the component's Entity will
         * be invalidated.
         * 
         * @param cid ComID of the component to erase.
         */
        void eraseComponent(ComID cid)
        {
            auto& comvec = cid.eid.iter->components;
            comvec.erase(cid.iter);
        }
        
        /*! Emplace component data into this Database.
         * 
         * Moves the given component data into this Database and associates it
         * with the given Entity.
         * 
         * @warning
         * All ComIDs of components associated with the given Entity are
         * invalidated.
         * 
         * @param eid Entity to attach component to.
         * @param dat Component data to move.
         * @return ComID to the new component.
         */
        ComID emplaceComponent(EntID eid, Entity::ComponentData&& dat)
        {
            ComID rv;
            auto& comvec = eid.iter->components;
            GUID guid = dat.getGUID();
            
            auto pos = lower_bound(begin(comvec), end(comvec), dat);
            
            rv.eid = eid;
            
            if (pos != end(comvec) && pos->getGUID() == guid)
            {
                rv.iter = pos;
                pos->getVal() = move(dat.getVal());
            }
            else
            {
                rv.iter = comvec.emplace(pos, move(dat));
            }
            
            return rv;
        }
        
        /*! Displace a component out of this Database.
         * 
         * Moves the given component out of this Database.
         * 
         * @warning
         * All ComIDs associated with the Entity associated with the given
         * component are invalidated.
         * 
         * @param cid ComID of the component to displace.
         * @return Component data.
         */
        Entity::ComponentData displaceComponent(ComID cid)
        {
            Entity::ComponentData rv = move(*cid.iter);
            auto& comvec = cid.eid.iter->components;
            comvec.erase(cid.iter);
            return rv;
        }
    
    // query
    
        /*! Query the Database.
         * 
         * Queries the Database for Entities that match the given template
         * properties.
         * 
         * Template properties can be one of the following:
         * 
         * - A component type.
         * - A list of component types in `Not<...>`.
         * 
         * If a property is a component type, only Entities that contain that
         * component will be returned.
         * 
         * If a property is a list of component types in `Not<...>`, only
         * Entities that do not contain those types will be returned.
         * 
         * A vector of query elements is returned.
         * 
         * A query element is a tuple containing an EntID and a series of
         * component elements.
         * 
         * A component element is a pair of reference-to-component and ComID.
         * 
         * There will be a component element for each component type given in
         * the property list.
         * 
         * For example, if `db.query<X,Y,Not<Z>>()` is called, its return type
         * will be determined as follows:
         * 
         * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
         * template <typename T> using ComEle = std::pair<T&,ComID>;
         * using QueryEle = std::tuple<EntID,ComEle<X>,ComEle<Y>>;
         * using QueryResult = std::vector<QueryEle>;
         * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
         * 
         * It is important to note that the return type of
         * `db.query<X,Y,Not<Z>>()` will be the same as the return type of
         * `db.query<X,Y>()`.
         * 
         * @warning
         * The result value will be invalidated whenever the Database is
         * modified in any way.
         * 
         * This function may perform memoization as a speed optimization.
         * Memoization data may be reset if the Database is modified in any way.
         * 
         * @tparam Ts Query properties.
         * @return Query results.
         */
        template <typename... Ts>
        typename QueryTraits<Database, Ts...>::result query()
        {
            using Traits = QueryTraits<Database, Ts...>;
            using Result = typename Traits::result;
            using Nots = typename Traits::nots;
            using Tmp = typename Traits::ptr_tup;
            
            Result rv;
            Tmp tmp;
            
            // Checks the eid for negative query properties.
            auto check_negatives = [](EntID eid)
            {
                return QueryHelper_noNots<Nots>::noNots(eid);
            };
            
            // Fills tmp with pointers, returns true on success.
            auto fill_tmp = [&](EntID eid)
            {
                return Traits::fillTmp(eid,tmp);
            };
            
            // Convert tmp pointers into references and push it onto rv.
            auto push_tmp = [&](EntID eid)
            {
                return Traits::pushTmp(eid,tmp,rv);
            };
            
            // Inspects eid for query match and pushes it onto rv if it matches.
            auto inspect_and_push = [&](EntID eid)
            {
                if (check_negatives(eid) && fill_tmp(eid))
                {
                    push_tmp(eid);
                }
            };
            
            // Query loop
            for (auto i=begin(entities), e=end(entities); i!=e; ++i)
            {
                EntID eid;
                eid.iter = i;
                inspect_and_push(eid);
            }
            
            return rv;
        }
};

} // namespace _detail

using _detail::Database;
using _detail::Not;

} // namespace Ginseng

#endif // GINSENG_GINSENG_HPP
