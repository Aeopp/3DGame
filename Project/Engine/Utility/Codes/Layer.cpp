#include "Layer.h"
#include "Object.h"
#include <algorithm>
#include <iterator>



void Engine::Layer::Update(const float DeltaTime)&
{
	for (auto& [Key, CurrentObjectContainer] : _ObjectMap)
	{
		for (auto& CurrentObject : CurrentObjectContainer)
		{
			CurrentObject->Update(DeltaTime);
		};
	};
}

void Engine::Layer::LateUpdate(const float DeltaTime)&
{
	for (auto& [Key, CurrentObjectContainer] : _ObjectMap)
	{
		for (auto& CurrentObject : CurrentObjectContainer)
		{
			CurrentObject->Update(DeltaTime);
		};
	};
}

void Engine::Layer::PendingKill() & noexcept
{
	for (auto& [Key, CurrentObjectContainer] : _ObjectMap)
	{
		for (size_t Idx = 0u; Idx < CurrentObjectContainer.size();)
		{
			if (CurrentObjectContainer[Idx]->IsPendingKill())
			{
				std::swap(CurrentObjectContainer[Idx], CurrentObjectContainer.back());
				CurrentObjectContainer.pop_back();
			}
			else 
				++Idx;
		};
	};

	for (auto& [Key, CurrentComponentContainer] : _ComponentMap)
	{
		for (size_t Idx = 0u; Idx < CurrentComponentContainer.size();)
		{
			if (CurrentComponentContainer[Idx].expired())
			{
				std::swap(CurrentComponentContainer[Idx], CurrentComponentContainer.back());
				CurrentComponentContainer.pop_back();
			}
			else
				++Idx;
		};
	};
}

std::vector<std::weak_ptr<Engine::Component>>& Engine::Layer::RefComponents(const Component::Property _Property) &
{
	return _ComponentMap.find(_Property)->second;
	/*auto iter = _ComponentMap.find(_Property);
	auto& _WeakComps = iter->second;
	std::vector<std::shared_ptr<Engine::Component>> _Components;
	_Components.reserve(_WeakComps.size());
	std::transform(std::begin(_WeakComps), std::end(_WeakComps), std::back_inserter(_Components), 
		[](std::weak_ptr<Component>& _WeakComp)
		{
			return _WeakComp.lock();
		});

	return _Components;*/
}
