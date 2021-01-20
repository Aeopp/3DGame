#include "Renderer.h"
#include <future>
#include "FMath.hpp"
#include "imgui.h"

void Engine::Renderer::Initialize(const DX::SharedPtr<IDirect3DDevice9>&  Device)&
{
	this->Device = Device;
	_Frustum.Initialize();
};

void Engine::Renderer::Render()&
{
	{
		// 컬링 테스트 코드...
		Matrix View, Projection, CameraWorld;
		Device->GetTransform(D3DTS_VIEW, &View);
		Device->GetTransform(D3DTS_PROJECTION, &Projection);
		CameraWorld = FMath::Inverse(View);
		const Vector3 CameraLocation = { CameraWorld._41,CameraWorld._42,CameraWorld._43 };

		if (ImGui::Button("Make Frustum"))
		{
			_Frustum.Make(CameraWorld, Projection);
		}

		Sphere _Sphere;
		_Sphere.Center = CameraLocation;
		_Sphere.Radius = 2.f;
		ImGui::Begin("Culling");
		ImGui::Text("CameraLocation: %f %f %f", _Sphere.Center.x, _Sphere.Center.y, _Sphere.Center.z);
		ImGui::End();

		if (_Frustum.IsIn(_Sphere))
		{
			ImGui::Begin("Culling");
			ImGui::Text("In");
			ImGui::End();
		}
		else
		{
			ImGui::Begin("Culling");
			ImGui::Text("Out");
			ImGui::End();
		}
	};
	
	RenderEnviroment();
	RenderNoAlpha();
	_Frustum.Render(Device.get());
	RenderObjects.clear();
};

void Engine::Renderer::Regist(RenderInterface* const Target)
{
	RenderObjects[Target->GetGroup()].push_back(*Target);
};

void Engine::Renderer::RenderNoAlpha()&
{
	Device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	Device->SetRenderState(D3DRS_ZENABLE, TRUE);
	Device->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
	Device->SetRenderState(D3DRS_LIGHTING, FALSE);
	Device->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
	Device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
#ifdef PARALLEL
	if (auto iter = RenderObjects.find(RenderInterface::Group::NoAlpha);
		iter != std::end(RenderObjects))
	{
		std::vector<std::future<void>> Futures;
		for (auto& NoAlphaRender : iter->second)
		{
			Futures.push_back(std::async(std::launch::async,
				[NoAlphaRender]() {
					NoAlphaRender.get().Render();
				}));
		}
		for (auto& Future : Futures)
		{
			Future.get();
		}
		Futures.clear();
	}
#else
	if (auto iter = RenderObjects.find(RenderInterface::Group::NoAlpha);
		iter != std::end(RenderObjects))
	{
		for (auto& NoAlphaRender : iter->second)
		{
			NoAlphaRender.get().Render();
		}
	}
#endif
}

void Engine::Renderer::RenderEnviroment()&
{
	Device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	Device->SetRenderState(D3DRS_ZENABLE, TRUE);
	Device->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
	Device->SetRenderState(D3DRS_LIGHTING, FALSE);
	Device->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
	Device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
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
