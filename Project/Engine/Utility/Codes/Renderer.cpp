#include "Renderer.h"
#include <future>
#include "FMath.hpp"
#include "imgui.h"
#include "UtilityGlobal.h"

void Engine::Renderer::Initialize(const DX::SharedPtr<IDirect3DDevice9>& Device)&
{
	this->Device = Device;
	_Frustum.Initialize();
	_DeferredPass.Initialize(Device.get());
};

void Engine::Renderer::Render()&
{
	if (Engine::Global::bDebugMode)
	{
		// ImGui::TreeNode("Frustum Culling");
	}

	Matrix View, Projection, CameraWorld;
	Device->GetTransform(D3DTS_VIEW, &View);
	Device->GetTransform(D3DTS_PROJECTION, &Projection);
	CameraWorld = FMath::Inverse(View);
	const Vector3 CameraLocation3D{ CameraWorld._41,CameraWorld._42,CameraWorld._43 };
	const Vector4  CameraLocation = FMath::ConvertVector4(CameraLocation3D, 1.f);
	
	_Frustum.Make(CameraWorld, Projection);
	// 디퍼드 
	{
		IDirect3DSurface9* CurBackBufSurface{ nullptr };
		Device->GetRenderTarget(0u, &CurBackBufSurface);

		_DeferredPass.Albedo.BindGraphicDevice(1u);
		_DeferredPass.Normal.BindGraphicDevice(2u);
		_DeferredPass.WorldLocations.BindGraphicDevice(3u);

		CurBackBufSurface->Release();
	};

	_Sky.Render(CameraLocation3D, Device.get());
	Device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	RenderLandscape(_Frustum, View  , Projection , CameraLocation);

	{
		RenderLandscape(_Frustum, View, Projection, CameraLocation);
		RenderLandscape(_Frustum, View, Projection, CameraLocation);
		RenderLandscape(_Frustum, View, Projection, CameraLocation);
		RenderLandscape(_Frustum, View, Projection, CameraLocation);
		RenderLandscape(_Frustum, View, Projection, CameraLocation);
		RenderLandscape(_Frustum, View, Projection, CameraLocation);
		RenderLandscape(_Frustum, View, Projection, CameraLocation);
		RenderLandscape(_Frustum, View, Projection, CameraLocation);
		RenderLandscape(_Frustum, View, Projection, CameraLocation);
	};
	
	
	RenderEnviroment(View, Projection, CameraLocation);
	RenderNoAlpha(View,Projection,CameraLocation  );
	if (Engine::Global::bDebugMode)
	{
		RenderDebugCollision(View, Projection, CameraLocation);
	}
	
	_Frustum.Render(Device.get());
	
	RenderUI(View, Projection, CameraLocation);
	
	
	_DeferredPass.Albedo.RenderDebugBuffer();
	_DeferredPass.Normal.RenderDebugBuffer();
	_DeferredPass.WorldLocations.RenderDebugBuffer();
	

	RenderObjects.clear();

	if (Engine::Global::bDebugMode)
	{
		// ImGui::TreePop();
	}
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
	const Vector4& CameraLocation4D)&
{
	Device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	Device->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
	Device->SetRenderState(D3DRS_ZENABLE, TRUE);
	Device->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
	Device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
	CurrentLandscape.Render(RefFrustum , View, Projection , CameraLocation4D);
}

void Engine::Renderer::RenderDebugCollision(const Matrix& View, const Matrix& Projection,
	const Vector4& CameraLocation)&
{
	if (Engine::Global::bDebugMode)
	{
		Device->SetVertexShader(nullptr);
		Device->SetPixelShader(nullptr);
		Device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
		Device->SetRenderState(D3DRS_LIGHTING, FALSE);
		Device->SetRenderState(D3DRS_FILLMODE,D3DFILL_WIREFRAME);
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
						_RefRender.Render(View,Projection,CameraLocation);
					}
				}
				else
				{
					_RefRender.Render(View, Projection, CameraLocation);
				}
			}
		}
#endif
	}
}

void Engine::Renderer::RenderNoAlpha(const Matrix& View, const Matrix& Projection,
	const Vector4& CameraLocation)&
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
					_RefRender.Render(View, Projection, CameraLocation);
				}
			}
			else
			{
				_RefRender.Render(View, Projection, CameraLocation);
			}
		}
	}
#endif
}

void Engine::Renderer::RenderEnviroment(const Matrix& View, const Matrix& Projection,
										const Vector4& CameraLocation                   )&
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
					_RefRender.Render(View, Projection, CameraLocation);
				}
			}
			else
			{
				_RefRender.Render(View, Projection, CameraLocation);
			}
		}
	}
#endif
}

void Engine::Renderer::RenderUI(const Matrix& View, const Matrix& Projection,
	const Vector4& CameraLocation)&
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
			_RefRender.Render(View, Projection, CameraLocation);
		}
	}
#endif
};

void Engine::Renderer::SkyInitialize(const std::filesystem::path& FullPath)&
{
	_Sky.Initialize(FullPath, Device.get() );
}