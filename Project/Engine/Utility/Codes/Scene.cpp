#include "Scene.h"
#include <array>
#include "Component.h"
#include <iterator>

Engine::Scene::Scene(IDirect3DDevice9& _Device) :_Device{ _Device } {}

void Engine::Scene::Update(const float DeltaTime)&
{
	for (auto& CurrentLayer : _Layers)
	{
		CurrentLayer->Update(DeltaTime);
	}

	for (auto& CurrentLayer : _Layers)
	{
		CurrentLayer->LateUpdate(DeltaTime);
	}
}
void Engine::Scene::PendingKill() & noexcept
{
	for (auto& CurrentLayer : _Layers)
	{
		CurrentLayer->PendingKill();
	}
}





