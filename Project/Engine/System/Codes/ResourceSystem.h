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
		inline auto Create(std::tuple<Params...> Tuple, Result(*Method)(Params...), const std::wstring& ResourceName);;
	private:
		template<typename TupleType, typename ResourceType, int32 Idx>
		inline auto CreateImplementation(TupleType& Tuple, const std::wstring& ResourceName);;
	private:
		std::unordered_map<std::wstring,
			std::unordered_map<std::wstring, DX::UniquePtr<IUnknown>>> Container;
	};
};

template<typename ResourceType>
inline auto Engine::ResourceSystem::Get(const std::wstring& Name)
{
	return static_cast<ResourceType* const>(Container[typeid(ResourceType).hash_code()][Name].get());
}

template<typename ResourceType, typename Result, typename ...Params>
inline auto Engine::ResourceSystem::Create(std::tuple<Params...> Tuple, Result(*Method)(Params...), const std::wstring& ResourceName)
{
	using TupleType = std::tuple<Params...>;
	std::apply(Method, Tuple);
	return CreateImplementation<TupleType, ResourceType, std::tuple_size_v<TupleType>-1>(Tuple, ResourceName);
}

template<typename TupleType, typename ResourceType, int32 Idx>
inline auto Engine::ResourceSystem::CreateImplementation(TupleType& Tuple, const std::wstring& ResourceName)
{
	using TargetType = std::remove_pointer_t<std::remove_pointer_t<std::decay_t<std::tuple_element_t<Idx, TupleType>>>>;

	if constexpr (std::is_same_v<TargetType, ResourceType>
		|| Idx < 0)
	{
		return  static_cast<TargetType* const>((Container[typeid(TargetType).hash_code()][ResourceName] = DX::MakeUnique(*std::get<Idx>(Tuple))).get());
	}
	else
	{
		return CreateImplementation<TupleType, ResourceType, Idx - 1>(Tuple, ResourceName);
	}
}
