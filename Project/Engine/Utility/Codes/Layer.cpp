#include "Layer.h"
#include "Object.h"
#include <algorithm>
#include <iterator>

void Engine::Layer::Initialize()&
{
}

void Engine::Layer::Event()&
{
	for (auto& [Key, CurrentObjectContainer] : _ObjectMap)
	{
		for (auto& CurrentObject : CurrentObjectContainer)
		{
			CurrentObject->Event();
		};
	};
}

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
			CurrentObject->LateUpdate(DeltaTime);
			CurrentObject->ComponentUpdate(DeltaTime);
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
}

