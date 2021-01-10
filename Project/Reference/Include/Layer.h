#pragma once
#include <unordered_map>
#include <map>
#include <memory>
#include "Layer.h"
#include "DllHelper.h"
#include "Component.h"

namespace Engine
{
	class DLL_DECL Layer 
	{
	public:
		virtual ~Layer()noexcept = default;
	public:
		void Update(const float DeltaTime)&;
		void LateUpdate(const float DeltaTime)&;
		void PendingKill() & noexcept;
		std::vector<std::weak_ptr<Component>>& RefComponents(const Component::Property _Property) &;
	private:
		std::unordered_map<std::wstring/*Class Type Info*/,
			std::vector<std::shared_ptr<class Object>>> _ObjectMap;
		// 컴포넌트의 소유권은 포함하는 오브젝트에게 있음.
		std::map<Component::Property,
			std::vector<std::weak_ptr<Component>>> _ComponentMap;
	};
};
