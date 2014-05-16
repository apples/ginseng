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
	
// Algorithms

	template <typename I1, typename I2, typename O, typename T, typename P>
	O transform_if(I1 first, I2 last, O out, T trans, P pred)
	{
		for (; first != last; ++first)
		{
			if (pred(first))
			{
				*out = trans(first);
				++out;
			}
		}
		return out;
	}

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
	
	class ComponentBase
	{
		ComponentBase(ComponentBase const&) = delete;
		ComponentBase(ComponentBase &&) noexcept = delete;
		ComponentBase& operator=(ComponentBase const&) = delete;
		ComponentBase& operator=(ComponentBase &&) noexcept = delete;

		protected:
			ComponentBase() = default;
			~ComponentBase() = default;
	};
	
	template <typename T>
	class Component : public ComponentBase
	{
		T val;
		
		public:
		
			Component(T t)
				: val(move(t))
			{}
			
			T& getVal()
			{
				return val;
			}
	};

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
		
		using ComponentData = GUIDPair<shared_ptr<ComponentBase>>;
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

// Database

template <template <typename> class AllocatorT = allocator>
class Database
{
	template <typename T>
	using AllocList = list<T, AllocatorT<T>>;
	
	AllocList<Entity> entities;
	
	public:
	
		// IDs
			
			class ComID; // forward declaration needed for EntID
			
			class EntID
			{
				friend class Database;
				
				typename AllocList<Entity>::iterator iter;
				
				public:
				
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
				
					template <typename... Ts>
					tuple<decltype(get<Ts>())...> getComs() const
					{
						return {get<Ts>()...};
					}
			};

			class ComID
			{
				friend class Database;
				
				EntID eid;
				Entity::ComponentVec::iterator iter;
				
				public:
					
					template <typename T>
					T& cast() const
					{
						auto& sptr = iter->getVal();
						auto ptr = static_cast<Component<T>*>(sptr.get());
						return ptr->getVal();
					}
			};
		
		// Entity functions
			
			EntID makeEntity()
			{
				EntID rv;
				rv.iter = entities.emplace(end(entities));
				return rv;
			}
			
			void eraseEntity(EntID eid)
			{
				entities.erase(eid.iter);
			}
			
			EntID emplaceEntity(Entity ent)
			{
				EntID rv;
				rv.iter = entities.emplace(end(entities), move(ent));
				return rv;
			}
			
			Entity displaceEntity(EntID eid)
			{
				Entity rv = move(*eid.iter);
				entities.erase(eid.iter);
				return rv;
			}
			
		// Component functions
		
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
			
			void eraseComponent(ComID cid)
			{
				auto& comvec = cid.eid.iter->components;
				comvec.erase(cid.iter);
			}
			
			ComID emplaceComponent(EntID eid, Entity::ComponentData dat)
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
			
			Entity::ComponentData displaceComponent(ComID cid)
			{
				Entity::ComponentData rv = move(*cid.iter);
				auto& comvec = cid.eid.iter->components;
				comvec.erase(cid.iter);
				return rv;
			}
		
		// query
		
			template <typename... Ts>
			typename QueryTraits<Database, Ts...>::result query()
			{
				using Traits = QueryTraits<Database, Ts...>;
				using Result = typename Traits::result;
				using Nots = typename Traits::nots;
				using Tmp = typename Traits::ptr_tup;
				
				Result rv;
				Tmp tmp;
				
				auto check_negatives = [](EntID eid)
				{
					return QueryHelper_noNots<Nots>::noNots(eid);
				};
				
				auto fill_tmp = [&](EntID eid)
				{
					return Traits::fillTmp(eid,tmp);
				};
				
				auto push_tmp = [&](EntID eid)
				{
					return Traits::pushTmp(eid,tmp,rv);
				};
				
				auto inspect_and_push = [&](EntID eid)
				{
					if (check_negatives(eid) && fill_tmp(eid))
					{
						push_tmp(eid);
					}
				};
				
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
