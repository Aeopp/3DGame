#include "Renderer.h"
#include <future>
#include "GraphicDevice.h"
#include "FMath.hpp"
#include "imgui.h"
#include "UtilityGlobal.h"
#include "ResourceSystem.h"
#include "Vertexs.hpp"
#include "Timer.h"

void Engine::Renderer::Initialize(const DX::SharedPtr<IDirect3DDevice9>& Device)&
{
	this->Device = Device;
	CreateStaticLightResource();

	_Frustum.Initialize();
	_DeferredPass.Initialize(Device.get());

	Engine::Light::LightInformation LightInfo{};
	LightInfo.Direction = { 0.031f,-0.796f,-0.604f, 0 };
	LightInfo.Location = { 0.0f,2201.835f,1545.454f, 1};
	LightInfo.ShadowFar = 5000.f;
	LightInfo.ShadowDepthBias = 0.0001f;
	LightInfo.OrthoProjectionSize = 3000.f;
	LightInfo._LightOpt = Engine::Light::LightOption::Directional;
	LightInfo.ShadowDepthMapSize = _DeferredPass.ShadowDepth.Width;
	
	_DirectionalLight.Initialize(Device.get(), LightInfo);
};

void Engine::Renderer::Update(const float DeltaTime)&
{
	
};

void Engine::Renderer::Render()&
{
	Matrix View, Projection, CameraWorld;
	Device->GetTransform(D3DTS_VIEW, &View);
	Device->GetTransform(D3DTS_PROJECTION, &Projection);
	CameraWorld = FMath::Inverse(View);
	const Vector3 CameraLocation3D{ CameraWorld._41,CameraWorld._42,CameraWorld._43 };
	const Vector4  CameraLocation = FMath::ConvertVector4(CameraLocation3D, 1.f);
	auto& _Timer = Timer::Instance;

	_Frustum.Make(CameraWorld, Projection);
	CurrentLandscape.FrustumCullingCheck(_Frustum);
	CurrentLandscape.Tick(_Timer->GetTick());

	IDirect3DSurface9* CurBackBufSurface{ nullptr };
	IDirect3DSurface9* CurBackDepthStencil{ nullptr };
	D3DVIEWPORT9 CurViewPort{}; 
	{
		Device->GetRenderTarget(0u, &CurBackBufSurface);
		Device->GetDepthStencilSurface(&CurBackDepthStencil);
		Device->GetViewport(&CurViewPort);
	};

	{		
		_DeferredPass.ShadowDepth.ClearWithDepthStencil(D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER);
		_DeferredPass.Albedo3_Contract1.Clear();
		_DeferredPass.Normal3_Power1.Clear();
		_DeferredPass.WorldLocation3_Depth1.Clear();
		_DeferredPass.CavityRGB1_RimRGB1_RimInnerWidth1_RimOuterWidth1.Clear();
	};

	{
		Device->SetDepthStencilSurface(CurBackDepthStencil);
	}
	// 디퍼드  1 Pass
	{
		 _DeferredPass.Albedo3_Contract1.BindGraphicDevice(0u);
		 _DeferredPass.Normal3_Power1.BindGraphicDevice(1u);
		 _DeferredPass.WorldLocation3_Depth1.BindGraphicDevice(2u);
		 _DeferredPass.CavityRGB1_RimRGB1_RimInnerWidth1_RimOuterWidth1.BindGraphicDevice(3u);

		 CurrentLandscape.RenderDeferredAlbedoNormalWorldPosDepthSpecularRim(
			 _Frustum, View, Projection, CameraLocation);
		 CurrentLandscape.RenderDeferredRim(_Frustum, View, Projection, CameraLocation);
	};

	const Matrix LightViewProjection = _DirectionalLight.CalcLightViewProjection();

	// 후처리 쉐도우
	{
		D3DVIEWPORT9 ShadowViewPort{};
		ShadowViewPort.Height = _DeferredPass.ShadowDepth.Height;
		ShadowViewPort.Width= _DeferredPass.ShadowDepth.Width;
		ShadowViewPort.X = 0u;
		ShadowViewPort.Y = 0u;
		ShadowViewPort.MinZ = 0.0f;
		ShadowViewPort.MaxZ = 1.0f;
		Device->SetViewport(&ShadowViewPort);
		
		_DeferredPass.ShadowDepth.BindGraphicDevice(0u);
		_DeferredPass.ShadowDepth.BindDepthStencil();

		CurrentLandscape.
			RenderShadowDepth(LightViewProjection);
	};

	{
		Device->SetRenderTarget(0u, CurBackBufSurface);
		Device->SetDepthStencilSurface(CurBackDepthStencil);
		Device->SetViewport(&CurViewPort);
	};

	{
		_DirectionalLight.Render(Device.get(),
			CameraLocation3D, View, Projection, _DeferredPass.Albedo3_Contract1.GetTexture(),
			_DeferredPass.Normal3_Power1.GetTexture(),
			_DeferredPass.WorldLocation3_Depth1.GetTexture(),
			_DeferredPass.CavityRGB1_RimRGB1_RimInnerWidth1_RimOuterWidth1.GetTexture(),
			_DeferredPass.ShadowDepth.GetTexture() ,
			FogColor,
			FogDistance );
	};
	

	{
		CurrentLandscape.Render(_Frustum, View, Projection, CameraLocation ,
			_DeferredPass.ShadowDepth.GetTexture() , LightViewProjection ,
			_DirectionalLight._LightInfo.ShadowDepthMapSize,
			_DirectionalLight._LightInfo.ShadowDepthBias  ,
			FogColor,
			FogDistance);
		RenderEnviroment(View, Projection, CameraLocation);
		RenderNoAlpha(View, Projection, CameraLocation);
		if (Engine::Global::bDebugMode)
		{
			RenderDebugCollision(View, Projection, CameraLocation);
		}
		_Frustum.Render(Device.get());
		RenderUI(View, Projection, CameraLocation);

		_Sky.Render(_Frustum, View, Projection, CameraLocation, Device.get(),
			_DeferredPass.WorldLocation3_Depth1.GetTexture());
	};
	

	{
		_DeferredPass.Albedo3_Contract1.RenderDebugBuffer();
		_DeferredPass.Normal3_Power1.RenderDebugBuffer();
		_DeferredPass.WorldLocation3_Depth1.RenderDebugBuffer();
		_DeferredPass.CavityRGB1_RimRGB1_RimInnerWidth1_RimOuterWidth1.RenderDebugBuffer();

		_DeferredPass.RimRGB1_InnerWidth1_OuterWidth1_NULL1.RenderDebugBuffer();
		_DeferredPass.ShadowDepth.RenderDebugBuffer();
	};
	
	{
		CurBackBufSurface->Release();
		CurBackDepthStencil->Release();
	};
	

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


void Engine::Renderer::CreateStaticLightResource()&
{
	Engine::ShaderFx::Load(Device.get(), 
		Engine::Global::ResourcePath / L"Shader" / L"DeferredLightFx.hlsl", L"DeferredLightFx");

	static std::wstring LightVertexBufferTag = L"LightVertexBuffer";
	auto& ResourceSys = ResourceSystem::Instance;

	IDirect3DVertexBuffer9* LightVtxBuf{ nullptr };
	Device->CreateVertexBuffer(sizeof(Vertex::Screen) * 4,
		D3DUSAGE_WRITEONLY, NULL, D3DPOOL_MANAGED,
		&LightVtxBuf, nullptr);

	ResourceSys->Insert<IDirect3DVertexBuffer9>(LightVertexBufferTag,
		LightVtxBuf);

	IDirect3DIndexBuffer9* LightIdxBuf{};
	Device->CreateIndexBuffer(sizeof(uint32) * 6u,
		D3DUSAGE_WRITEONLY,
		D3DFMT_INDEX32,
		D3DPOOL_MANAGED,
		&LightIdxBuf, nullptr);

	static std::wstring LightIndexBufferTag = L"LightIndexBuffer";

	ResourceSys->Insert<IDirect3DIndexBuffer9>(LightIndexBufferTag, LightIdxBuf);

	static std::wstring LightVertexDeclTag = L"LightVertexDecl";
	ResourceSys->Insert<IDirect3DVertexDeclaration9>(LightVertexDeclTag,
		Vertex::Screen::GetVertexDecl(Device.get()));

	Vertex::Screen* VtxBufPtr{ nullptr };
	LightVtxBuf->Lock(0, 0, reinterpret_cast<void**>(&VtxBufPtr), NULL);

	VtxBufPtr[0].NDC= { -1.f, 1.f, 0.0f };
	VtxBufPtr[0].UV2D= { 0.f, 0.f };

	VtxBufPtr[1].NDC = { 1.f, 1.f, 0.0f};
	VtxBufPtr[1].UV2D = { 1.f, 0.f };

	VtxBufPtr[2].NDC = { 1.f, -1.f, 0.0f};
	VtxBufPtr[2].UV2D =  { 1.f, 1.f}  ;

	VtxBufPtr[3].NDC = { -1.f, -1.f, 0.0f};
	VtxBufPtr[3].UV2D = { 0.f, 1.f };

	LightVtxBuf->Unlock(); 


	uint32* IdxBufPtr = NULL;

	LightIdxBuf->Lock(0, 0, (void**)&IdxBufPtr, 0);

	IdxBufPtr[0]= 0u;
	IdxBufPtr[1]= 1u;
	IdxBufPtr[2]= 2u;
				   
	IdxBufPtr[3]= 0u;
	IdxBufPtr[4]= 2u;
	IdxBufPtr[5]= 3u;

	LightIdxBuf->Unlock();


	Engine::ShaderFx::Load(Device.get(),
		Engine::Global::ResourcePath / L"Shader" / L"DebugBufferRenderFx.hlsl",
		L"DebugBufferRenderFx");
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