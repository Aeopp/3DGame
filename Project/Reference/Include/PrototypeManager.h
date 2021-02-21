#pragma once
#include "SingletonInterface.h"
#include "DllHelper.h"
#include <unordered_map>
#include <memory>
#include "Object.h"
#include <typeinfo>

namespace Engine
{
	class DLL_DECL PrototypeManager :public SingletonInterface<PrototypeManager>
	{
	public:
		void Initialize();
		template<typename ObjectSubType,
			typename...Params>
			void LoadPrototype(
				const std::wstring& Tag,
				Params&&... _Params)&;
		template<typename ObjectSubType>
		auto Clone(const std::wstring& TargetTag)&;

		std::shared_ptr<Engine::Object > Clone(
			const std::string& ObjectSubTypeName , 
			const std::wstring& TargetTag)&;

		void Clear() & noexcept;
		void Clear(const std::wstring& Tag);
		void ClearExceptTag(const std::wstring& Tag)&;

		std::vector< std::function<Engine::Object::SpawnReturnValue
		(const Engine::Object::SpawnParam&)>> Editor()&;
	private:
		std::unordered_map<std::wstring/*User Custom Tag*/, 
		std::unordered_map<std::string,
			std::unique_ptr<Object>>> _Container;
	};
};



inline std::shared_ptr<Engine::Object >
Engine::PrototypeManager::Clone(
	const std::string& ObjectSubTypeName, const std::wstring&TargetTag)&
{
	auto ContainerIter = _Container.find(TargetTag);

	std::shared_ptr<Engine::Object > ClonePtr{ nullptr };

	if (ContainerIter != std::end(_Container))
	{
		const auto& TargetContainer = ContainerIter->second;
		if (auto ProtoTypeIter = TargetContainer.find(ObjectSubTypeName);
			ProtoTypeIter != std::end(TargetContainer))
		{
			const auto& Prototype = ProtoTypeIter->second;

			ClonePtr = Prototype->GetCopyShared();
			ClonePtr->Clone();
		}
	}

	return ClonePtr; 
}

template<typename ObjectSubType>
inline auto Engine::PrototypeManager::Clone(
	const std::wstring& TargetTag)&
{
	static_assert(std::is_base_of_v<Object, ObjectSubType>,
		__FUNCTION__);

	const auto& TargetContainer= _Container.find(TargetTag)->second;
	const auto& Prototype= 
		TargetContainer.find(typeid(ObjectSubType).name())->second;

	auto _Clone = std::make_shared<ObjectSubType>(
		reinterpret_cast<const ObjectSubType&>(*Prototype.get())
		);
	_Clone->Clone();
	return _Clone;
}

template<typename ObjectSubType, typename ...Params>
inline void Engine::PrototypeManager::LoadPrototype(
	const std::wstring& Tag,
	Params && ..._Params)&
{
	static_assert(std::is_base_of_v<Object, ObjectSubType>,
		__FUNCTION__);

	auto _PrototypeElement = std::make_unique<ObjectSubType>();
	_PrototypeElement->PrototypeInitialize(std::forward<Params>
		(_Params)...);
	_PrototypeElement->PrototypeTag = Tag;
	_Container[Tag].insert({ typeid(ObjectSubType).name(),
		std::move(_PrototypeElement) });
}
