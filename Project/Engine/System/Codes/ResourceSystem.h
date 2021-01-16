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

		template<typename ResourceType, typename ResourceCreateMethodType, typename... Params>
		inline auto Emplace(
			const std::wstring& ResourceName,
			ResourceCreateMethodType _ResourceCreate,
			Params&&... _Params);

		template<typename ResourceType>
		inline auto Create(const std::wstring& ResourceName,
			IUnknown* const ResourcePtr);
	private:
		template<typename TupleType, typename ResourceType, int32 Idx>
		inline auto EmplaceImplementation(TupleType& Tuple, const std::wstring& ResourceName);;
	private:
		std::unordered_map<size_t,
			std::unordered_map<std::wstring, DX::SharedPtr<IUnknown>>> Container;
	};
};

template<typename ResourceType>
inline auto Engine::ResourceSystem::Get(const std::wstring& Name)
{
	return static_cast<ResourceType* const>(Container[typeid(ResourceType).hash_code()][Name].get());
}

template<typename ResourceType, typename ResourceCreateMethodType, typename ...Params>
inline auto Engine::ResourceSystem::Emplace(
	const std::wstring& ResourceName,
	ResourceCreateMethodType _ResourceCreate,
	Params&&... _Params)
{
	auto Tuple = std::make_tuple(std::forward<Params>(_Params)...);
	using TupleType = decltype(Tuple);
	std::apply(_ResourceCreate, Tuple);
	return EmplaceImplementation<TupleType, ResourceType, std::tuple_size_v<TupleType>-1>(Tuple, ResourceName);
}

template<typename ResourceType>
inline auto Engine::ResourceSystem::Create(
	const std::wstring& ResourceName,
	IUnknown* const ResourcePtr)
{
	return static_cast<ResourceType* const> (Container[typeid(ResourceType).hash_code()].emplace
	(ResourceName, DX::MakeShared(ResourcePtr)).first->second.get());
};

template<typename TupleType, typename ResourceType, int32 Idx>
inline auto Engine::ResourceSystem::EmplaceImplementation(
	TupleType& Tuple, const std::wstring& ResourceName)
{
	using TargetType = std::remove_pointer_t<std::remove_pointer_t<std::decay_t<std::tuple_element_t<Idx, TupleType>>>>;

	if constexpr (std::is_same_v<TargetType, ResourceType>
		|| Idx < 0)
	{
		return static_cast<ResourceType* const> (Container[typeid(TargetType).hash_code()].emplace(ResourceName,
			DX::MakeShared(*std::get<Idx>(Tuple))).first->second.get());
	}
	else
	{
		return CreateImplementation<TupleType, ResourceType, Idx - 1>(Tuple, ResourceName);
	}
}