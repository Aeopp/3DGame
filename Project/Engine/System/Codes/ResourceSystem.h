#pragma once
#include "SingletonInterface.h"
#include "DllHelper.h"
#include "DxHelper.h"
#include <d3d9.h>
#include "TypeAlias.h"

namespace Engine
{
	class DLL_DECL ResourceSystem : public SingletonInterface<ResourceSystem>
	{
	public:
		void Initialize()&;
		template<typename ResourceType>
		inline auto Get(const std::wstring& Name);
		template<typename ResourceType, typename Result, typename... Params>
		inline auto Create(
			const std::wstring& ResourceName , 
			Result(*Method)(Params...) , 
			Params&&... _Params);
	private:
		template<typename TupleType, typename ResourceType, int32 Idx>
		inline auto CreateImplementation(TupleType& Tuple, const std::wstring& ResourceName);;
	private:
		std::unordered_map<size_t,
			std::unordered_map<std::wstring, DX::UniquePtr<IUnknown>>> Container;
	};
};

template<typename ResourceType>
inline auto Engine::ResourceSystem::Get(const std::wstring& Name)
{
	return static_cast<ResourceType* const>(Container[typeid(ResourceType).hash_code()][Name].get());
}

template<typename ResourceType, typename Result, typename ...Params>
inline auto Engine::ResourceSystem::Create(
	const std::wstring& ResourceName,
	Result(*Method)(Params...),
	Params&&... _Params)
{
	using TupleType = std::tuple<Params...>;
	auto Tuple = std::make_tuple(std::forward<Params>(_Params)...);
	std::apply(Method, Tuple);
	return CreateImplementation<TupleType, ResourceType,			std::tuple_size_v<TupleType>-1>(Tuple, ResourceName);
}

template<typename TupleType, typename ResourceType, int32 Idx>
inline auto Engine::ResourceSystem::CreateImplementation(
	TupleType& Tuple, const std::wstring& ResourceName)
{
	using TargetType = std::remove_pointer_t<std::remove_pointer_t<std::decay_t<std::tuple_element_t<Idx, TupleType>>>>;

	if constexpr (std::is_same_v<TargetType, ResourceType>
		|| Idx < 0)
	{
		/*return   Container[typeid(TargetType).hash_code()].insert({ ResourceName  ,  DX::MakeUnique(*std::get<Idx>(Tuple)) });*/
		return 1;
	}
	else
	{
		return 2;
		/*return CreateImplementation<TupleType, ResourceType, Idx - 1>(Tuple, ResourceName);*/
	}
}
