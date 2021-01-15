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
		Layer() = default;
		virtual ~Layer()noexcept = default;
		Layer(Layer&&)noexcept = default;
	public:
		void Initialize()&;
		virtual void Update(const float DeltaTime)&;
		virtual void LateUpdate(const float DeltaTime)&;
		void PendingKill() & noexcept;
	public:
		auto& RefObjects()&;

		template<typename ObjectSubType>
		auto FindObjects()&;
		template<typename ObjectSubType>
		auto FindObject(const std::wstring& TargetName)&;
	public:
		template<typename ObjectSubType>
		auto NewObject(std::shared_ptr<ObjectSubType> Clone)&;
	private:
		std::unordered_map<std::string/*Class Type Info*/,
			std::vector<std::shared_ptr<Object>>> _ObjectMap;
	};
};

template<typename ObjectSubType>
inline auto Engine::Layer::FindObject(const std::wstring& TargetName)&
{
	static_assert(std::is_base_of_v<Object, ObjectSubType>, __FUNCTION__);

	auto TargetContainer = FindObjects<ObjectSubType>();
	auto iter = std::find_if(std::begin(TargetContainer), std::end(TargetContainer),
		[TargetName](const std::shared_ptr<ObjectSubType>& Target)
		{
			return Target->GetName() == TargetName;
		});
	
	return (*iter);
};

template<typename ObjectSubType>
inline auto Engine::Layer::NewObject(
	std::shared_ptr<ObjectSubType> Clone)&
{
	static_assert(std::is_base_of_v<Object, ObjectSubType>, __FUNCTION__);

	return std::static_pointer_cast<ObjectSubType>
		(_ObjectMap[typeid(ObjectSubType).name()].emplace_back
		(std::move(Clone) ) );
};

inline auto& Engine::Layer::RefObjects()&
{
	return _ObjectMap;
};

template<typename ObjectSubType>
inline auto Engine::Layer::FindObjects()&
{
	static_assert(std::is_base_of_v<Object, ObjectSubType>, __FUNCTION__);
	return reinterpret_cast<std::vector<std::shared_ptr<ObjectSubType>>&>					(_ObjectMap.find(typeid(ObjectSubType).name())->second);
};
