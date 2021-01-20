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
		//   프로토타입 초기화 단계에서 컴포넌트를 생성하지 마세요.
		void PrototypeInitialize()&;
		virtual void Clone()&;
		virtual void Event()&;
		virtual void Update(const float DeltaTime)&;
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
		auto AddComponent(Params&&... _Params)&;
		void ComponentUpdate(const float DeltaTime)&;
		template<typename ComponentSubType>
		auto GetComponent()&;
		template<typename ComponentSubType>
		bool IsContain()&;
	public:
		virtual void HitNotify(Object* const Target, const Vector3 PushDir,
			const float CrossAreaScale)&;
	protected:
		std::wstring Name{};
	private:
		bool bPendingKill{ false };
		std::map<Component::Property/*업데이트 순서 통제*/,
			std::unordered_map<std::string,
			std::shared_ptr<Component>>>_Components;
	};
};

template<typename ComponentSubType, typename... Params>
inline auto Engine::Object::AddComponent(Params&&... _Params)&
{
	static_assert(std::is_base_of_v<Component, ComponentSubType>,
		__FUNCTION__);

	auto _Component = std::make_shared<ComponentSubType>();
	_Component->Initialize(std::forward<Params>(_Params)...);

	return static_cast<ComponentSubType* const>((_Components[ComponentSubType::TypeProperty]
		[typeid(ComponentSubType).name()] = (_Component)).get());
};

template<typename ComponentSubType>
inline bool Engine::Object::IsContain()&
{
	return _Components[ComponentSubType::TypeProperty].contains(typeid(ComponentSubType).name());
}

template<typename ComponentSubType>
inline auto Engine::Object::GetComponent()&
{
	static_assert(std::is_base_of_v<Component, ComponentSubType>,
		__FUNCTION__);

	return static_cast<ComponentSubType* const> (_Components.find(ComponentSubType::TypeProperty)->second
		.find(typeid(ComponentSubType).name())->second.get());
};

inline bool Engine::Object::IsPendingKill() const& { return bPendingKill; }

inline void Engine::Object::Kill()& { bPendingKill = true; }

inline const std::wstring& Engine::Object::GetName() const& { return Name; }

inline void Engine::Object::SetName(const std::wstring& Name) { this->Name = Name; }
