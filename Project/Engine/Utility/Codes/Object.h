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
		void PrototypeInitialize()&;
		virtual void Clone()&;
		virtual void Update(const float DeltaTime)& abstract;
		virtual void LateUpdate(const float DeltaTime)&;
	public:
		Object() = default;
		Object(Object&&)noexcept = default;
		Object(const Object&) = default;
		virtual ~Object() noexcept = default;
	public:
		inline bool IsPendingKill()const&;
		inline void Kill()&;
		inline const std::wstring& GetName()const&;
		inline void SetName(const std::wstring& Name);
	public:
		template<typename ComponentSubType, typename... Params>
		auto& AddComponent(Params&&... _Params)&;

		template<typename ComponentSubType>
		auto& GetComponent()&;
	protected:
		std::wstring Name{};
	private:
		bool bPendingKill{ false };
		std::map<Component::Property/*업데이트 순서 통제*/,
			std::unordered_map<std::wstring,		std::shared_ptr<Component>>>_Components;
	private:
		void ComponentUpdate(const float DeltaTime)&;
	};
};

template<typename ComponentSubType, typename... Params>
inline auto& Engine::Object::AddComponent(Params&&... _Params)&
{
	static_assert(std::is_base_of_v<Component, ComponentSubType>,
		__FUNCTION__);

	auto& _Component = std::make_shared<ComponentSubType>();
	_Component->Initialize(std::forward<Params>(_Params)...);

	return _Components[ComponentSubType::TypeProperty]
					[typeid(ComponentSubType).name()] = std::move(_Component);
};

template<typename ComponentSubType>
inline auto& Engine::Object::GetComponent()&
{
	static_assert(std::is_base_of_v<Component, ComponentSubType>,
		__FUNCTION__);

	return *(_Components.find(ComponentSubType::TypeProperty)->second
					.find(typeid(ComponentSubType).name())->second);
};

inline bool Engine::Object::IsPendingKill() const& { return bPendingKill; }

inline void Engine::Object::Kill()& { bPendingKill = true; }

inline const std::wstring& Engine::Object::GetName() const& { return Name; }

inline void Engine::Object::SetName(const std::wstring& Name) { this->Name = Name; }
