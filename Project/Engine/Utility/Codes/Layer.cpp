#include "Layer.h"
#include "Object.h"
#include <algorithm>
#include <iterator>
#include "imgui.h"
#include "UtilityGlobal.h"
void Engine::Layer::Initialize()&
{
}

void Engine::Layer::Event()&
{
	if (Engine::Global::bDebugMode)
	{
		ImGui::Begin("Object Information");
	};

	for (auto& [Key, CurrentObjectContainer] : _ObjectMap)
	{
		for (auto& CurrentObject : CurrentObjectContainer)
		{
			CurrentObject->Event();
		};
	};

	

	if (Engine::Global::bDebugMode)
	{
		ImGui::End();
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

