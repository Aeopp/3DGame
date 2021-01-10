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
		// ������Ʈ�� �������� �����ϴ� ������Ʈ���� ����.
		std::map<Component::Property,
			std::vector<std::weak_ptr<Component>>> _ComponentMap;
	};
};
