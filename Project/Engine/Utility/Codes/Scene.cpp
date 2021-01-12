#include "Scene.h"
#include <array>
#include "Component.h"
#include <iterator>

void Engine::Scene::Initialize(IDirect3DDevice9* const Device)&
{
	this->Device = Device;
}

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





