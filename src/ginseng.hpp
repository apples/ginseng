#include <cstddef>
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <memory>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace Ginseng
{

// Base IDs

using EID = long long; // Entity ID
using CID = long long; // Component ID
using TID = long long; // Table ID

// Private Namespace

namespace _detail
{

// Template Metas

template <typename Base, typename Member, typename T
    , typename TD = typename ::std::decay<T>::type
    , bool valid = ::std::is_same<Base, TD>::value
    , bool is_const = ::std::is_const<T>::value
    , typename Type = typename ::std::conditional<is_const, const Member, Member>::type
>
using MType = typename ::std::enable_if<valid, Type>::type;

// Component Base

class ComponentBase
{
  public:
    virtual ~ComponentBase() = 0;
    
    virtual ::std::ostream& debugPrint(::std::ostream& out) const = 0;
};

ComponentBase::~ComponentBase()
{}

// Private Types

struct Types
{
    // Templates
    
    template <typename T, typename H = ::std::hash<T>>
    using Many = ::std::unordered_set<T, H>;

    template <typename... Ts>
    using Join = ::std::tuple<Ts...>;

    template <typename K, typename V, typename H = ::std::hash<K>>
    using Table = ::std::unordered_map<K, V, H>;

    template <typename... Ts>
    using Result = ::std::vector<::std::tuple<EID, Ts*...>>;
    
    template <typename... Ts>
    using RElement = typename Result<Ts...>::value_type;

    // Aggregate IDs
    
    using CDAT = ComponentBase*;
    using CTID = Join<CID, TID>;

    using ComponentData = Join<EID,::std::unique_ptr<ComponentBase>>;
    using ComponentList = Table<CID,ComponentData>;
    using ComponentTable = Join<ComponentList,Many<EID>>;
    
    // Aggregate Getters

    template <typename T>
    static MType<ComponentTable, ComponentList, T>& getList(T& table)
    {
        return ::std::get<0>(table);
    }

    template <typename T>
    static MType<ComponentTable, Many<EID>, T>& getEIDs(T& table)
    {
        return ::std::get<1>(table);
    }

    template <typename T>
    static MType<ComponentData, EID, T>& getEID(T& cd)
    {
        return ::std::get<0>(cd);
    }

    template <typename T>
    static MType<ComponentData, CDAT, T> getCDAT(T& cd)
    {
        return ::std::get<1>(cd).get();
    }

    template <typename T>
    static MType<CTID, CID, T>& getCID(T& cidtid)
    {
        return ::std::get<0>(cidtid);
    }

    template <typename T>
    static MType<CTID, TID, T>& getTID(T& cidtid)
    {
        return ::std::get<1>(cidtid);
    }

    // Custom Hashes

    struct CTIDHash
    {
        ::std::size_t operator()(const CTID& cidtid) const&
        {
            return ::std::hash<CID>{}(getCID(cidtid));
        }
    };
};

} // namespace _detail

// Component CRTP

template <typename Child>
class Component
    : public _detail::ComponentBase
{
    friend class Database;
    static TID tid;
  public:
    virtual ::std::ostream& debugPrint(::std::ostream& out) const override
    {
        out << "[ No debug message for TID " << tid << ". ]";
        return out;
    }
};

template <typename Child>
TID Component<Child>::tid = -1;

// Main Database engine

class Database final
    : private _detail::Types
{
    Table<EID,Table<TID,CID>> entities;
    Table<TID,ComponentTable> components;
    
    struct
    {
        EID eid = 0;
        TID tid = 0;
        CID cid = 0;
    } uidGen;
    
    EID createEntityID()
    {
        return ++uidGen.eid;
    }
    
    TID createTableID()
    {
        return ++uidGen.tid;
    }
    
    CID createCTID()
    {
        return ++uidGen.cid;
    }
    
    template <int N = 2, typename T>
    typename ::std::enable_if<
        (N >= ::std::tuple_size<T>::value) ,
    bool>::type fill_inspect(T& ele) const
    {
        return true;
    }
    
    template <int N = 2, typename T>
    typename ::std::enable_if<
        (N < ::std::tuple_size<T>::value) ,
    bool>::type fill_inspect(T& ele) const
    {
        using PtrType = typename ::std::tuple_element<N, T>::type;
        using Type = typename ::std::remove_pointer<PtrType>::type;
        
        EID eid = ::std::get<0>(ele);
        const Table<TID,CID>& tcids = entities.at(eid);
        
        auto iter = tcids.find(Type::tid);
        
        if (iter == end(tcids)) return false;
        
        ::std::get<N>(ele) = getComponent<Type>(iter->second);
        
        return fill_inspect<N+1>(ele);
    }
    
    template <typename T, typename... Us>
    Result<T, Us...> select_inspect() const
    {
        Result<T, Us...> rval;
        
        const ComponentTable& table = components.at(T::tid);
        const ComponentList& list = getList(table);
        
        for (auto&& p : list)
        {
            const ComponentData& cd = p.second;
            EID eid = getEID(cd);
            T* data = static_cast<T*>(getCDAT(cd));
            
            RElement<T, Us...> ele;
            
            ::std::get<0>(ele) = eid;
            ::std::get<1>(ele) = data;
            
            if (fill_inspect(ele))
            {
                rval.emplace_back(::std::move(ele));
            }
        }
        
        return rval;
    }
    
  public:
  
    enum class Selector
    {
        INSPECT
    };
    
    EID newEntity()
    {
        return createEntityID();
    }
    
    void eraseEntity(EID ent)
    {
        auto eiter = entities.find(ent);
        
        if (eiter == end(entities)) throw; // TODO
        
        Table<TID,CID>& comps = eiter->second;
        
        for (auto&& p : comps)
        {
            ComponentTable& tab = components.at(p.first);
            ComponentList& list = getList(tab);
            Many<EID>& eids = getEIDs(tab);
            
            {
                auto citer = list.find(p.second);
                list.erase(citer);
            }
            
            {
                auto liter = eids.find(ent);
                eids.erase(liter);
            }
        }
        
        entities.erase(eiter);
    }
    
    template <typename T>
    TID registerComponent()
    {
        if (T::tid != -1) throw; //TODO
        T::tid = createTableID();
        return T::tid;
    }
    
    template <typename T>
    T* newComponent(EID ent)
    {
        if (T::tid == -1) throw; //TODO
        
        CID cid = createCTID();
        
        ::std::unique_ptr<_detail::ComponentBase> ptr (new T);
        T* rval = static_cast<T*>(ptr.get());
        
        entities[ent][T::tid] = cid;
        
        ComponentTable& table = components[T::tid];
        {
            ComponentList& list = getList(table);
            {
                ComponentData dat { ent, ::std::move(ptr) };
                list.insert(::std::make_pair(cid, ::std::move(dat)));
            }
        }
        {
            Many<EID>& eset = getEIDs(table);
            eset.insert(ent);
        }
        
        return rval;
    }
    
    template <typename T>
    T* getComponent(CID cid) const
    {
        return static_cast<T*>(getCDAT(getList(components.at(T::tid)).at(cid)));
    }
    
    template <typename... Ts>
    Result<Ts...> getEntities(const Selector method = Selector::INSPECT) const
    {
        switch (method)
        {
            case Selector::INSPECT:
                return select_inspect<Ts...>();
        }
        
        throw; // TODO
    }
    
    ::std::ostream& debugPrint(::std::ostream& out) const&
    {
        out << "Entity count: " << entities.size() << ::std::endl;
        out << "Entity total: " << uidGen.eid << ::std::endl;
        for (auto&& p : entities)
        {
            EID eid = p.first;
            auto&& cids = p.second;
            
            out << ::std::setw(3) << eid;
            for (auto&& cid : cids)
            {
                out << ' ';
                out << ::std::setw(3) << cid.first;
                out << ':';
                out << ::std::setw(2) << ::std::left << cid.second;
                out << ::std::right;
            }
            out << ::std::endl;
        }
        
        out << "Components registered: " << uidGen.tid << ::std::endl;
        out << "Components used:       " << components.size() << ::std::endl;
        for (auto&& p : components)
        {
            TID tid = p.first;
            const ComponentTable& table = p.second;
            const ComponentList& list = getList(table);
            const Many<EID>& eidlist = getEIDs(table);
            
            ::std::vector<EID> eids (begin(eidlist), end(eidlist));
            ::std::sort(begin(eids), end(eids));
            
            out << "Component " << tid << ":" << ::std::endl;
            out << "    Entities:";
            for (auto&& eid : eids) out << ' ' << ::std::setw(5) << eid;
            out << ::std::endl;
            for (auto&& q : list)
            {
                CID cid = q.first;
                const ComponentData& data = q.second;
                out << "    ";
                out << ::std::setw(5) << ::std::left << cid;
                out << ::std::right;
                out << ' ' << ::std::setw(5) << getEID(data);
                out << ' ';
                getCDAT(data)->debugPrint(out);
                out << ::std::endl;
            }
        }
        
        return out;
    }
};

} // namespace Ginseng
