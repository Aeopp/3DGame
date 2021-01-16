#include "Scene.h"
#include <array>
#include "Component.h"
#include <iterator>

void Engine::Scene::Initialize(IDirect3DDevice9* const Device)&
{
	this->Device = Device;
}

void Engine::Scene::Event()&
{
	for (auto& [TypeKey, CurrentLayer] : LayerMap)
	{
		CurrentLayer->Event();
	}
}

void Engine::Scene::Update(const float DeltaTime)&
{
	for (auto& [TypeKey, CurrentLayer] : LayerMap)
	{
		CurrentLayer->Update(DeltaTime);
	}

	for (auto& [TypeKey, CurrentLayer] : LayerMap)
	{
		CurrentLayer->LateUpdate(DeltaTime);
	}
};

void Engine::Scene::PendingKill() & noexcept
{
	for (auto& [TypeKey,CurrentLayer] : LayerMap)
	{
		CurrentLayer->PendingKill();
	}
}





