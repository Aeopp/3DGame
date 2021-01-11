#include "Renderer.h"

void Engine::Renderer::Initialize(IDirect3DDevice9* const Device)&
{
	this->Device = Device;
};

void Engine::Renderer::Render()&
{
	RenderEnviroment();

	RenderObjects.clear();
};

void Engine::Renderer::Regist(RenderInterface* const Target)
{
	RenderObjects[Target->GetGroup()].push_back(*Target);
};

void Engine::Renderer::RenderEnviroment()&
{
	auto iter = RenderObjects.find(RenderInterface::Group::Enviroment);
	if (iter != std::end(RenderObjects))
	{
		for (auto& Enviroment : iter->second)
		{
			Enviroment.get().Render();
		}
	}
}
