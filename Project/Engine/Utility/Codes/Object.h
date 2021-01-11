#pragma once
#include <map>
#include <vector>
#include <unordered_map>
#include <memory>
#include "Component.h"

namespace Engine
{
	class DLL_DECL Object abstract
	{
	public:
		void Initialize()&;
		virtual void Update(const float DeltaTime)& abstract;
		virtual void LateUpdate(const float DeltaTime)&;
	public:
		Object(Object&&)noexcept = default;
		virtual ~Object() noexcept = default;
	public:
		inline bool IsPendingKill()const& { return bPendingKill; };
		inline void Kill()& { bPendingKill = true; };

		template<typename ComponentSubType, typename... Params>
		void MakeComponent(Params&&... _Params)&;

		template<typename ComponentSubType>
		auto& GetComponent()&;
	private:
		bool bPendingKill{ false };
		std::map<Component::Property/*업데이트 순서 통제*/,
			std::vector<std::unique_ptr<Component>>> _Components;
	private:
		void ComponentUpdate(const float DeltaTime)&;
	};
};

template<typename ComponentSubType, typename... Params>
inline void Engine::Object::MakeComponent(Params&&... _Params)&
{
	static_assert(std::is_base_of_v<Component, ComponentSubType>,
		__FUNCTION__);

	auto& _Component = _Components[ComponentSubType::TypeProperty].emplace_back(
		std::make_unique<ComponentSubType>());

	_Component->Initialize(std::forward<Params>(_Params)...);
}

template<typename ComponentSubType>
inline auto& Engine::Object::GetComponent()&
{
	static_assert(std::is_base_of_v<Component, ComponentSubType>,
		__FUNCTION__);

	const auto FindTypeId = typeid(ComponentSubType);

	for (auto& CurrentComponent : _Components.find(
		ComponentSubType::TypeProperty)->second)
	{
		if (typeid(decltype(CurrentComponent)::element_type)
			== FindTypeId)
			return CurrentComponent;
	};
}



