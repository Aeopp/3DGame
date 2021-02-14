#include "Renderer.h"
#include <future>
#include "FMath.hpp"
#include "imgui.h"
#include "UtilityGlobal.h"

void Engine::Renderer::Initialize(const DX::SharedPtr<IDirect3DDevice9>& Device)&
{
	this->Device = Device;
	_Frustum.Initialize();
};

void Engine::Renderer::Render()&
{
	Matrix View, Projection, CameraWorld;
	Device->GetTransform(D3DTS_VIEW, &View);
	Device->GetTransform(D3DTS_PROJECTION, &Projection);
	CameraWorld = FMath::Inverse(View);
	const Vector3 CameraLocation = { CameraWorld._41,CameraWorld._42,CameraWorld._43 };
	_Frustum.Make(CameraWorld, Projection);
	RenderLandscape(_Frustum, View  , Projection , CameraLocation);
	RenderEnviroment();
	RenderNoAlpha();
	if (Engine::Global::bDebugMode)
	{
		RenderDebugCollision();
	}
	_Frustum.Render(Device.get());
	RenderUI();
	RenderObjects.clear();
};

void Engine::Renderer::Regist(RenderInterface* const Target)
{
	RenderObjects[Target->GetGroup()].push_back(*Target);
};

Engine::Landscape& Engine::Renderer::RefLandscape()&
{
	return   CurrentLandscape;
};

void Engine::Renderer::RenderLandscape(
	Frustum& RefFrustum,
	const Matrix& View,const Matrix& Projection ,
	const Vector3& CameraLocation)&
{
	Device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	Device->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
	Device->SetRenderState(D3DRS_ZENABLE, TRUE);
	Device->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
	Device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
	CurrentLandscape.Render(RefFrustum , View, Projection , CameraLocation);
}

void Engine::Renderer::RenderDebugCollision()&
{
	if (Engine::Global::bDebugMode)
	{
		Device->SetVertexShader(nullptr);
		Device->SetPixelShader(nullptr);
		Device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
		Device->SetRenderState(D3DRS_LIGHTING, FALSE);
		Device->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
		Device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
#ifdef PARALLEL
		if (auto iter = RenderObjects.find(RenderInterface::Group::DebugCollision);
			iter != std::end(RenderObjects))
		{
			std::vector<std::future<void>> Futures;
			for (auto& _RenderEntity : iter->second)
			{
				Futures.push_back(std::async(std::launch::async,
					[_Frustum,_RenderEntity]() {
						RenderInterface& _RefRender = _RenderEntity.get();
						const Sphere CullingCheckSphere = _RefRender.GetCullingSphere();
						if (_Frustum.IsIn(CullingCheckSphere))
						{
							_RefRender.Render();
						};
					}));
			}
			for (auto& Future : Futures)
			{
				Future.get();
			}
			Futures.clear();
		}
#else
		if (auto iter = RenderObjects.find(RenderInterface::Group::DebugCollision);
			iter != std::end(RenderObjects))
		{
			for (auto& _RenderEntity : iter->second)
			{
				RenderInterface& _RefRender = _RenderEntity.get();
				if (_RefRender.bCullingOn)
				{
					if (_Frustum.IsIn(_RefRender.GetCullingSphere()))
					{
						_RefRender.Render();
					}
				}
				else
				{
					_RefRender.Render();
				}
			}
		}
#endif
	}
}

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
				[_Frustum, _RenderEntity]() {
					RenderInterface& _RefRender = _RenderEntity.get();
					const Sphere CullingCheckSphere = _RefRender.GetCullingSphere();
					if (_Frustum.IsIn(CullingCheckSphere))
					{
						_RefRender.Render();
};
					}));
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
		for (auto& _RenderEntity : iter->second)
		{
			RenderInterface& _RefRender = _RenderEntity.get();
			if (_RefRender.bCullingOn)
			{
				if (_Frustum.IsIn(_RefRender.GetCullingSphere()))
				{
					_RefRender.Render();
				}
			}
			else
			{
				_RefRender.Render();
			}
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
				[_Frustum , _RenderEntity]() {
					RenderInterface& _RefRender = _RenderEntity.get();
					const Sphere CullingCheckSphere = _RefRender.GetCullingSphere();
					if (_Frustum.IsIn(CullingCheckSphere))
					{
						_RefRender.Render();
					};
				}));
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
		for (auto& _RenderEntity : iter->second)
		{
			RenderInterface& _RefRender = _RenderEntity.get();
			if (_RefRender.bCullingOn)
			{
				if (_Frustum.IsIn(_RefRender.GetCullingSphere()))
				{
					_RefRender.Render();
				}
			}
			else
			{
				_RefRender.Render();
			}
		}
	}
#endif
}

void Engine::Renderer::RenderUI()&
{
	Device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	Device->SetRenderState(D3DRS_ZENABLE, FALSE);
	Device->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
	Device->SetRenderState(D3DRS_LIGHTING, FALSE);
	Device->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
	Device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
#ifdef PARALLEL
	if (auto iter = RenderObjects.find(RenderInterface::Group::UI);
		iter != std::end(RenderObjects))
	{
		std::vector<std::future<void>> Futures;
		for (auto& _RenderEntity : iter->second)
		{
			Futures.push_back(std::async(std::launch::async,
				[_RenderEntity]() {
					RenderInterface& _RefRender = _RenderEntity.get();
					_RefRender.Render();
				}));
		}));
	}
	for (auto& Future : Futures)
	{
		Future.get();
	}
	Futures.clear();
}
#else
	if (auto iter = RenderObjects.find(RenderInterface::Group::UI);
		iter != std::end(RenderObjects))
	{
		for (auto& _RenderEntity : iter->second)
		{
			RenderInterface& _RefRender = _RenderEntity.get();
			_RefRender.Render();
		}
	}
#endif
}
