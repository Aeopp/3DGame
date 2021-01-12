#pragma once
#include <unordered_map>
#include <map>
#include <memory>
#include <optional>
#include "Layer.h"
#include "Object.h"
#include "DllHelper.h"
#include "Component.h"

namespace Engine
{
	class DLL_DECL Layer abstract
	{
	public:
		virtual ~Layer()noexcept = default;
		Layer(Layer&&)noexcept = default;
	public:
		virtual void Update(const float DeltaTime)&;
		virtual void LateUpdate(const float DeltaTime)&;
		void PendingKill() & noexcept;
	public:
		template<typename ObjectSubType>
		auto& RefObjects()&;
		auto& RefObjects()&;
		template<typename ObjectSubType>
		auto& FindObject(const std::wstring& TargetName)&;
	public:
		template<typename ObjectSubType,typename...Params>
		auto& NewObject(std::wstring ObjectName,Params&&... _Params)&;
	private:
		std::unordered_map<std::string/*Class Type Info*/,
			std::vector<std::shared_ptr<Object>>> _ObjectMap;
	};
};

template<typename ObjectSubType>
inline auto& Engine::Layer::FindObject(const std::wstring& TargetName)&
{
	static_assert(std::is_base_of_v<Object, ObjectSubType>, __FUNCTION__);

	const auto& TargetContainer = RefObjects<ObjectSubType>();
	auto iter = std::find_if(std::begin(TargetContainer), std::end(TargetContainer),
		[TargetName](ObjectSubType* const Target)
		{
			return Target->GetName() == TargetName;
		});

	return iter->second;
}

template<typename ObjectSubType, typename ...Params>
inline auto& Engine::Layer::NewObject(
	std::wstring ObjectName, Params && ..._Params)&
{
	static_assert(std::is_base_of_v<Object, ObjectSubType>, __FUNCTION__);
	auto NewObjectShared = std::make_shared<ObjectSubType>();
	NewObjectShared->SetName(std::move(ObjectName));
	NewObjectShared->Initialize(std::forward<Params>(_Params)...);
	return _ObjectMap[typeid(ObjectSubType).name()].push_back
					(std::move(NewObjectShared) );
}

inline auto& Engine::Layer::RefObjects()&
{
	return _ObjectMap;
};

template<typename ObjectSubType>
inline auto& Engine::Layer::RefObjects()&
{
	static_assert(std::is_base_of_v<Object, ObjectSubType>, __FUNCTION__);

	return _ObjectMap.find(typeid(ObjectSubType).name())->second;
}
