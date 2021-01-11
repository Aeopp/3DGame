#pragma once
#include "SingletonInterface.h"
#include "DllHelper.h"
#include <unordered_map>
#include <memory>
#include "Object.h"
#include <typeinfo>

namespace Engine
{
	class DLL_DECL PrototypeManager :public SingletonInterface<PrototypeManager >
	{
	public:
		template<typename ObjectSubType,
			typename...Params>
			void LoadPrototype(Params&&... _Params)&;
		template<typename ObjectSubType>
		auto Clone()&;
	private:
		std::unordered_map<std::wstring,
			std::unique_ptr<Object>> _Container;
	};
};


template<typename ObjectSubType>
inline auto Engine::PrototypeManager::Clone()&
{
	static_assert(std::is_base_of_v<Object, ObjectSubType>,
		__FUNCTION__);

	return std::make_shared<ObjectSubType>(
		_Container.find(typeid(ObjectSubType).name())->second.get());
}

template<typename ObjectSubType, typename ...Params>
inline void Engine::PrototypeManager::LoadPrototype(Params && ..._Params)&
{
	static_assert(std::is_base_of_v<Object, ObjectSubType>,
		__FUNCTION__);

	auto _PrototypeElement = std::make_unique<ObjectSubType>();
	_PrototypeElement->Initialize(std::forward<Params>
		(_Params)...);

	_Container.insert({ typeid(ObjectSubType).name(),
		std::move(_PrototypeElement) });
}
