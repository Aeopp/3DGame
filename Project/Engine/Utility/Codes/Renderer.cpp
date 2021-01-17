#include "Renderer.h"
#include <future>
#include "FMath.hpp"
#include "imgui.h"

void Engine::Renderer::Initialize(const DX::SharedPtr<IDirect3DDevice9>&  Device)&
{
	this->Device = Device;
};

void Engine::Renderer::Render()&
{
	Matrix View, Projection;
	Device->GetTransform(D3DTS_VIEW, &View);
	Device->GetTransform(D3DTS_PROJECTION, &Projection);
	if (ImGui::Button("Make Frustum"))
	{
		_Frustum.Make(FMath::Inverse(View), Projection);
	}
	Sphere _Sphere;
	_Sphere.Center = { -View._41,-View._42,-View._43 };
	_Sphere.Radius = 10.f;
	ImGui::Begin("Culling");
	if (_Frustum.IsIn(_Sphere)) { ImGui::Text("In"); }
	else ImGui::Text("In");
	ImGui::End();
	//RenderEnviroment( );
	_Frustum.Render(Device.get());
	RenderObjects.clear();
};

void Engine::Renderer::Regist(RenderInterface* const Target)
{
	RenderObjects[Target->GetGroup()].push_back(*Target);
};

void Engine::Renderer::RenderEnviroment()&
{
#ifdef PARALLEL
	if (auto iter = RenderObjects.find(RenderInterface::Group::Enviroment);
		iter != std::end(RenderObjects))
	{
		std::vector<std::future<void>> Futures;
		for (auto& Enviroment : iter->second)
		{
			Futures.push_back(std::async(std::launch::async,
				[Enviroment]() {
					Enviroment.get().Render();
				}));
		}
		for (auto& Future : Futures)
		{
			Future.get();
		}
		Futures.clear();
	}
#else
	if (auto iter = RenderObjects.find(RenderInterface::Group::Enviroment);
		iter != std::end(RenderObjects))
	{
		for (auto& Enviroment : iter->second)
		{
			Enviroment.get().Render();
		}
	}
#endif
}
