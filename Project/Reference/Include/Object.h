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
		inline bool IsPendingKill()const&;
		inline void Kill()&;
		inline const std::wstring& GetName()const&;
		inline void SetName(std::wstring Name);
	public:
		template<typename ComponentSubType, typename... Params>
		void AddComponent(Params&&... _Params)&;

		template<typename ComponentSubType>
		auto& GetComponent()&;
	protected:
		std::wstring Name{};
	private:
		bool bPendingKill{ false };
		std::map<Component::Property/*업데이트 순서 통제*/,
			std::vector<std::unique_ptr<Component>>> _Components;
	private:
		void ComponentUpdate(const float DeltaTime)&;
	};
};

template<typename ComponentSubType, typename... Params>
inline void Engine::Object::AddComponent(Params&&... _Params)&
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


inline bool Engine::Object::IsPendingKill() const& { return bPendingKill; }

inline void Engine::Object::Kill()& { bPendingKill = true; }

inline const std::wstring& Engine::Object::GetName() const& { return Name; }

inline void Engine::Object::SetName(std::wstring Name) { this->Name = std::move(Name); }
