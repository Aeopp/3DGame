﻿#include "Renderer.h"
#include <future>
#include "GraphicDevice.h"
#include "FMath.hpp"
#include "imgui.h"
#include "UtilityGlobal.h"
#include "ResourceSystem.h"
#include "Vertexs.hpp"
#include <filesystem>
#include "Timer.h"
#include "NavigationMesh.h"
#include "FontManager.h"
#include "EffectSystem.h"

void Engine::Renderer::Initialize(const DX::SharedPtr<IDirect3DDevice9>& Device)&
{
	this->Device = Device;
	CreateStaticLightResource();
	
	_Frustum.Initialize();

	_DeferredPass.Initialize(Device.get());

	Engine::Light::LightInformation LightInfo{};
	const Vector3 LightDirection = 
		FMath::Normalize(Vector3 { 0.151f, -0.658f, -0.738f} );

	LightInfo._LightOpt = Engine::Light::LightOption::Directional;
	LightInfo.ShadowDepthMapWidth= _DeferredPass.ShadowDepth.Width;
	LightInfo.ShadowDepthMapHeight = _DeferredPass.ShadowDepth.Height;

	_DirectionalLight.Initialize(Device.get(), LightInfo);

	
	EffectSys.Initialize(Device.get());

	GaussianBlurFx.Initialize(L"GaussianBlurFx");
	auto& ResourceSys = ResourceSystem::Instance;
	VtxBuf = ResourceSys->Get<IDirect3DVertexBuffer9>(L"LightVertexBuffer");
	IdxBuf = ResourceSys->Get<IDirect3DIndexBuffer9>(L"LightIndexBuffer");
	VtxDecl = ResourceSys->Get<IDirect3DVertexDeclaration9>(L"LightVertexDecl");

	// CreateUIFromUIPath();
};



void Engine::Renderer::Update(const float DeltaTime)&
{
	EffectSys.Update(DeltaTime);
};

void Engine::Renderer::Render()&
{
	SetUpRenderInfo();
	FrustumInCheck();
	RenderReady(); 
	BackUpCurBackBuffer();
	ClearAllRenderTarget();
	BindDeferredPass();
	RenderDeferred();
	BindShadowDepthPass();
	RenderShadowDepth();
	BindVelocity();
	RenderVelocity();
	BindEmissive();
	RenderEffectEmissive();
	
	// 여기서 이미시브 렌더링 객체들

	// 
	RenderEmissiveBlur();

	RestoreBackBuffer();
	BindDeferredTarget();
	RenderDeferredTarget();
	BindMotionBlur();
	RenderMotionBlur();

	// 지연 렌더링 이후 렌더링...
	RenderAlphaTest();
	RenderAlphaBlend();
	RenderSky();
	RenderEffect();
	// RestoreBackBuffer();
	
	NavigationMesh::Instance->Render(Device.get());
	RenderDebugCollision();

	// 디버그 렌더링.... 
	_Frustum.Render(Device.get());
	if (bDebugRenderTargetRender)
	{
		RenderDeferredDebugBuffer(); 
	}

	RenderUI();

	FontManager::Instance->Render();

	// 정보 초기화 ... 
	CurBackBufSurface->Release();
	CurBackDepthStencil->Release();
	RenderObjects.clear();

	// 렌더링 정보 저장 .
	SetUpPrevRenderInfo();

	if (Engine::Global::bDebugMode)
	{
		ImGui::Begin("ShaderOption");
		if (ImGui::TreeNode("SoftParticle"))
		{
			ImGui::InputFloat("SoftParticleDepthScale", 
				&EffectSys.SoftParticleDepthScale);
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("MotionBlur"))
		{
			ImGui::SliderFloat("VelocityScale", &MotionBlurVelocityScale, 0.00f, 1.f);
			ImGui::InputFloat("MotionBlurDepthBias", &MotionBlurDepthBias, 0.00f, 1.f);
			ImGui::InputFloat("MotionBlurLengthMin ", &MotionBlurLengthMin);
			ImGui::TreePop();
		}

		ImGui::End();
	}
};

void Engine::Renderer::Regist(RenderInterface* const Target)
{
	RenderObjects[Target->GetGroup()].push_back(*Target);
}


std::weak_ptr<Engine::UI> 
Engine::Renderer::MakeUI(
	const Vector2 Scale ,
	const Vector2 Position  ,
	const std::filesystem::path& TexFullPath, 
	const float UIDepthZ)&
{
	auto CreateUI = std::make_shared<UI>();
	CreateUI->Initialize(
		Device.get(), Scale,
		Position, TexFullPath, UIDepthZ );
	_UIs.insert({ CreateUI->GetUIDepthZ(),CreateUI });
	return CreateUI;
}
void Engine::Renderer::FrustumInCheck()&
{
	CurrentLandscape.FrustumCullingCheck(_Frustum);

	for (auto& [Group, RenderEntityRefs] :RenderObjects)
	{
		if (Group == RenderInterface::Group::UI)continue;

		for (auto& RenderEntityRef : RenderEntityRefs)
		{
			bool EntityFrustumIn = RenderEntityRef.get().FrustumInCheck(_Frustum);
		}
	}
};

void Engine::Renderer::RenderReady()&
{
	auto& _Timer = Timer::Instance;
	CurrentLandscape.Tick(_Timer->GetTick());

	for (auto& [Group, RenderEntityRefs  ] :RenderObjects)
	{
		for (auto& RenderEntityRef : RenderEntityRefs)
		{
			RenderEntityRef.get().RenderReady(this);
		}
	}

	EffectSys.RenderReady(this);
}
void Engine::Renderer::SetUpRenderInfo()&
{
	Matrix View, Projection, CameraWorld;
	Device->GetTransform(D3DTS_VIEW, &View);
	Device->GetTransform(D3DTS_PROJECTION, &Projection);
	CameraWorld = FMath::Inverse(View);

	CurrentRenderInformation.CameraLocation = { CameraWorld._41,CameraWorld._42,CameraWorld._43 };
	CurrentRenderInformation.CameraLocation4D = FMath::ConvertVector4(CurrentRenderInformation.CameraLocation, 1.f);
	CurrentRenderInformation.View = View;
	CurrentRenderInformation.Projection = Projection;
	CurrentRenderInformation.ViewProjection = View * Projection;
	CurrentRenderInformation.InverseViewProjection =FMath::Inverse(CurrentRenderInformation.ViewProjection);
	
	D3DXMatrixOrthoLH(
		&CurrentRenderInformation.OrthoProjection, Engine::Global::ClientSize.first,
		Engine::Global::ClientSize.second, 0.0f, 1.f);

	CurrentRenderInformation.LightViewProjection = _DirectionalLight.CalcLightViewProjection();
	_Frustum.Make(CameraWorld, Projection);
}
void Engine::Renderer::SetUpPrevRenderInfo()&
{
	PrevRenderInformation = CurrentRenderInformation;
}
void Engine::Renderer::BackUpCurBackBuffer()&
{
		Device->GetRenderTarget(0u, &CurBackBufSurface);
		Device->GetDepthStencilSurface(&CurBackDepthStencil);
		Device->GetViewport(&CurViewPort);
	
}
void Engine::Renderer::RestoreBackBuffer()&
{
	Device->SetRenderTarget(0u, CurBackBufSurface);
	Device->SetDepthStencilSurface(CurBackDepthStencil);
	Device->SetViewport(&CurViewPort);
}
void Engine::Renderer::ClearAllRenderTarget()&
{


	_DeferredPass.ShadowDepth.ClearWithDepthStencil(D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER);
	_DeferredPass.VelocityMap.ClearWithDepthStencil(D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER);
	_DeferredPass.Albedo3_Contract1.Clear();
	_DeferredPass.Normal3_Power1.Clear();
	_DeferredPass.Velocity2_None1_Depth1.Clear();
	_DeferredPass.CavityRGB1_RimRGB1_RimInnerWidth1_RimOuterWidth1.Clear();
	_DeferredPass.MotionBlur.Clear();


	_DeferredPass.EmissiveTarget.Clear();
	_DeferredPass.EmissiveBlurX.Clear();
	_DeferredPass.EmissiveBlurY.Clear();
	
	_DeferredPass.DeferredTarget.Clear();





}
void Engine::Renderer::BindDeferredPass()&
{
	_DeferredPass.Albedo3_Contract1.BindGraphicDevice(0u);
	_DeferredPass.Normal3_Power1.BindGraphicDevice(1u);
	_DeferredPass.Velocity2_None1_Depth1.BindGraphicDevice(2u);
	_DeferredPass.CavityRGB1_RimRGB1_RimInnerWidth1_RimOuterWidth1.BindGraphicDevice(3u);

	Device->SetDepthStencilSurface(CurBackDepthStencil);
}
void Engine::Renderer::RenderDeferred()&
{
	CurrentLandscape.
		RenderDeferredAlbedoNormalWorldPosDepthSpecularRim(
			_Frustum, CurrentRenderInformation.View, CurrentRenderInformation.Projection, CurrentRenderInformation.CameraLocation4D);

	if (auto DeferredNoAlphaIter = RenderObjects.find(RenderInterface::Group::DeferredNoAlpha);
		DeferredNoAlphaIter != std::end(RenderObjects))
	{
		auto& RenderEntityRefs = DeferredNoAlphaIter->second;

		for (auto& RenderEntityRef : RenderEntityRefs)
		{
			auto& EntityRef = RenderEntityRef.get();

			EntityRef.RenderDeferredAlbedoNormalVelocityDepthSpecularRim(this);
		}
	}
}
void Engine::Renderer::BindVelocity()&
{
	_DeferredPass.VelocityMap.BindGraphicDevice(0u);
	_DeferredPass.VelocityMap.BindDepthStencil();
};

void Engine::Renderer::RenderVelocity()&
{
	CurrentLandscape.RenderVelocity(this);

	if (auto DeferredNoAlphaIter = RenderObjects.find(RenderInterface::Group::DeferredNoAlpha);
		DeferredNoAlphaIter != std::end(RenderObjects))
	{
		auto& RenderEntityRefs = DeferredNoAlphaIter->second;

		for (auto& RenderEntityRef : RenderEntityRefs)
		{
			auto& EntityRef = RenderEntityRef.get();
			if (EntityRef.bMotionBlur)
			{
				EntityRef.RenderVelocity(this);
			}
		}
	}
};

void Engine::Renderer::BindDeferredTarget()&
{
	_DeferredPass.DeferredTarget.BindGraphicDevice(1u);
};

void Engine::Renderer::RenderDeferredTarget()&
{
	_DirectionalLight.Render(Device.get(),
		CurrentRenderInformation.CameraLocation, CurrentRenderInformation.View, CurrentRenderInformation.Projection,
		_DeferredPass.Albedo3_Contract1.GetTexture(),
		_DeferredPass.Normal3_Power1.GetTexture(),
		_DeferredPass.Velocity2_None1_Depth1.GetTexture(),
		_DeferredPass.CavityRGB1_RimRGB1_RimInnerWidth1_RimOuterWidth1.GetTexture(),
		_DeferredPass.ShadowDepth.GetTexture(),
		FogColor,
		FogDistance);
};

void Engine::Renderer::BindMotionBlur()&
{
	_DeferredPass.MotionBlur.BindGraphicDevice(1u);
}

void Engine::Renderer::RenderMotionBlur()&
{
	_DirectionalLight.MotionBlurRender(Device.get(),this);
};

void Engine::Renderer::BindShadowDepthPass()&
{
	D3DVIEWPORT9 ShadowViewPort{};
	ShadowViewPort.Height = _DeferredPass.ShadowDepth.Height;
	ShadowViewPort.Width = _DeferredPass.ShadowDepth.Width;
	ShadowViewPort.X = 0u;
	ShadowViewPort.Y = 0u;
	ShadowViewPort.MinZ = 0.0f;
	ShadowViewPort.MaxZ = 1.0f;
	Device->SetViewport(&ShadowViewPort);

	_DeferredPass.ShadowDepth.BindGraphicDevice(0u);
	_DeferredPass.ShadowDepth.BindDepthStencil();

}
void Engine::Renderer::RenderShadowDepth()&
{
	CurrentLandscape.
		RenderShadowDepth(CurrentRenderInformation.LightViewProjection);

	for ( auto& [Group ,RenderEntityRefs  ] :  RenderObjects)
	{
		if (Group == RenderInterface::Group::UI)continue;

		for (auto& RenderEntityRef : RenderEntityRefs)
		{
			auto& EntityRef = RenderEntityRef.get();

			if (EntityRef.bShadowDepth)
			{
				EntityRef.RenderShadowDepth(this);
			}
		}
	}
}

void Engine::Renderer::BindEmissive()&
{
	_DeferredPass.EmissiveTarget.BindGraphicDevice(0u);
}

void Engine::Renderer::BindEmissiveBlurX()&
{
	_DeferredPass.EmissiveBlurX.BindGraphicDevice(0u);
}

void Engine::Renderer::BindEmissiveBlurY()&
{
	_DeferredPass.EmissiveBlurY.BindGraphicDevice(0u);
}

void Engine::Renderer::RenderDeferredDebugBuffer()&
{
	if (Engine::Global::bDebugMode)
	{
		_DeferredPass.Albedo3_Contract1.RenderDebugBuffer();
		_DeferredPass.Normal3_Power1.RenderDebugBuffer();
		_DeferredPass.Velocity2_None1_Depth1.RenderDebugBuffer();
		_DeferredPass.CavityRGB1_RimRGB1_RimInnerWidth1_RimOuterWidth1.RenderDebugBuffer();


		_DeferredPass.VelocityMap.RenderDebugBuffer();
		_DeferredPass.ShadowDepth.RenderDebugBuffer();
		_DeferredPass.DeferredTarget.RenderDebugBuffer();
		_DeferredPass.MotionBlur.RenderDebugBuffer();

		_DeferredPass.EmissiveTarget.RenderDebugBuffer();
		_DeferredPass.EmissiveBlurX.RenderDebugBuffer();
		_DeferredPass.EmissiveBlurY.RenderDebugBuffer();

	}
}
void Engine::Renderer::RenderSky()&
{
	_Sky.Render(_Frustum, CurrentRenderInformation.View, CurrentRenderInformation.Projection, CurrentRenderInformation.CameraLocation4D, Device.get(),
		_DeferredPass.Velocity2_None1_Depth1.GetTexture());
}

void Engine::Renderer::RenderEffect()&
{
	EffectSys.Render(this);
}

void Engine::Renderer::RenderEffectEmissive()&
{
	EffectSys.RenderEmissive(this);
}

Engine::Landscape& Engine::Renderer::RefLandscape()&
{
	return   CurrentLandscape;
};


void Engine::Renderer::CreateStaticLightResource()&
{
	auto& ResourceSys = ResourceSystem::Instance;

	if (FAILED(D3DXCreateTextureFromFile(Device.get(), 
		 (Engine::Global::ResourcePathA/"Texture"/"dissolve.tga").c_str(), &DissolveTexture)))
	{
		throw std::exception(__FUNCTION__);
	}

	if (FAILED(D3DXCreateTextureFromFile(Device.get(),
		(Engine::Global::ResourcePathA / "Texture" / "blue_burn.tga").c_str(), &BlueBurnTexture)))
	{
		throw std::exception(__FUNCTION__);
	}

	
	if (FAILED(D3DXCreateTextureFromFile(Device.get(),
		(Engine::Global::ResourcePathA / "Texture" / "burn.tga").c_str(), &BurnTexture)))
	{
		throw std::exception(__FUNCTION__);
	}

	
	ResourceSys->Insert<IDirect3DTexture9>(L"Texture_Burn", BurnTexture);
	ResourceSys->Insert<IDirect3DTexture9>(L"Texture_BlueBurn", BlueBurnTexture);
	ResourceSys->Insert<IDirect3DTexture9>(L"Texture_Dissolve", DissolveTexture);

	Engine::ShaderFx::Load(Device.get(), 
		Engine::Global::ResourcePath / L"Shader" / L"DeferredLightFx.hlsl", L"DeferredLightFx");

	Engine::ShaderFx::Load(Device.get(),
		Engine::Global::ResourcePath / L"Shader" / L"GaussianBlurFx.hlsl", L"GaussianBlurFx");

	

	Engine::ShaderFx::Load(Device.get(), Engine::Global::ResourcePath / L"Shader" / L"MotionBlurFx.hlsl", L"MotionBlurFx");


	static std::wstring LightVertexBufferTag = L"LightVertexBuffer";
	

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


void Engine::Renderer::RenderDebugCollision()&
{
	if (Engine::Global::bDebugMode)
	{
		Device->SetVertexShader(nullptr);
		Device->SetPixelShader(nullptr);
		Device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
		Device->SetRenderState(D3DRS_LIGHTING, FALSE);
		Device->SetRenderState(D3DRS_FILLMODE,D3DFILL_WIREFRAME);
		if (auto iter = RenderObjects.find(RenderInterface::Group::DebugCollision);
			iter != std::end(RenderObjects))
		{
			for (auto& _RenderEntity : iter->second)
			{
				RenderInterface& _RefRender = _RenderEntity.get();

				if (_Frustum.IsIn(_RefRender.GetCullingSphere()))
				{
					_RefRender.Render(this);
				}
				else
				{
					_RefRender.Render(this);
				}
			}
		}
	}
}

void Engine::Renderer::RenderAlphaBlend()&
{
	CurrentLandscape.Render(_Frustum, CurrentRenderInformation.View, CurrentRenderInformation.Projection, CurrentRenderInformation.CameraLocation4D,
		_DeferredPass.ShadowDepth.GetTexture(), CurrentRenderInformation.LightViewProjection,
		_DirectionalLight._LightInfo.ShadowDepthMapWidth,
		_DirectionalLight._LightInfo.ShadowDepthMapHeight,
		_DirectionalLight._LightInfo.ShadowDepthBias,
		FogColor,
		FogDistance);


	if (auto iter = RenderObjects.find(RenderInterface::Group::AlphaBlend);
		iter != std::end(RenderObjects))
	{
		for (auto& _RenderEntity : iter->second)
		{
			RenderInterface& _RefRender = _RenderEntity.get();
			_RefRender.Render(this);
		}
	}
}

void Engine::Renderer::RenderAlphaTest()&
{
	if (auto iter = RenderObjects.find(RenderInterface::Group::AlphaTest);
		iter != std::end(RenderObjects))
	{
		for (auto& _RenderEntity : iter->second)
		{
			RenderInterface& _RefRender = _RenderEntity.get();
			_RefRender.Render(this);
		}
	}
}


void Engine::Renderer::RenderUI()&
{
	if (auto iter = RenderObjects.find(RenderInterface::Group::UI);
		iter != std::end(RenderObjects))
	{
		for (auto& _RenderEntity : iter->second)
		{
			RenderInterface& _RefRender = _RenderEntity.get();
			_RefRender.Render(this);
		}
	}

	for (auto&  [UIDepthZ,CurUI ]: _UIs)
	{
		CurUI->Render(this);
	}


	for (const auto&  [ Factor, Position ]: HPBarWorldUIInfos)
	{
		HPBarUI->bRender = true;
		HPBarUI->AlphaFactor = 1.f;
		HPBarUI->Flag = 2u;
		HPBarUI->XScale = Factor;

		Matrix Billboard = FMath::Inverse(GetCurrentRenderInformation().View);
		Billboard._41 = 0.0f;
		Billboard._42 = 0.0f;
		Billboard._43 = 0.0f;
		const Vector3 UILocation = Position +
			Vector3{ 0, 10,0 };

		HPBarUI->WorldUI =
			FMath::Scale({ 10.f,0.8f,1 }) * Billboard *
			FMath::Translation(UILocation);

		HPBarUI->WorldUI->_42 += 22.f;

		HPBarUI->Render(this);
	}

	HPBarWorldUIInfos.clear();
}
void Engine::Renderer::RenderEmissiveBlur()&
{
	BindEmissiveBlurX();

	auto Fx =GaussianBlurFx.GetHandle();

	const auto& CurRenderInfo = GetCurrentRenderInformation();
	const auto& PrevRenderInfo = GetPrevRenderInformation();

	uint32 Pass = 0u;
	Fx->Begin(&Pass, NULL);
	static constexpr uint32 BlurXPassIdx = 0u;
	{
		const float MapSizeX = static_cast<float> (_DeferredPass.EmissiveTarget.Width);
		const float MapSizeY = static_cast<float> (_DeferredPass.EmissiveTarget.Height);

		Fx->SetFloat("TexSizeX", MapSizeX);
		Fx->SetFloat("TexSizeY", MapSizeY);

		Fx->BeginPass(BlurXPassIdx);
		Fx->SetTexture("Emissive", _DeferredPass.EmissiveTarget.GetTexture());
		Fx->CommitChanges();
		Device->SetIndices(IdxBuf);
		Device->SetStreamSource(0u, VtxBuf, 0u, sizeof(Vertex::Screen));
		Device->SetVertexDeclaration(VtxDecl);
		Device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0u, 0u, 4u, 0u, 2u);
		Fx->EndPass();
	}

	BindEmissiveBlurY();

	{
		static constexpr uint32 BlurYPassIdx = 1u;
		const float MapSizeX = static_cast<float> (_DeferredPass.EmissiveBlurX.Width);
		const float MapSizeY = static_cast<float> (_DeferredPass.EmissiveBlurX.Height);

		Fx->SetFloat("TexSizeX", MapSizeX);
		Fx->SetFloat("TexSizeY", MapSizeY);
		Fx->BeginPass(BlurYPassIdx);
		Fx->SetTexture("Emissive",
			_DeferredPass.EmissiveBlurX.GetTexture());
		Fx->CommitChanges();
		Device->SetIndices(IdxBuf);
		Device->SetStreamSource(0u, VtxBuf, 0u, sizeof(Vertex::Screen));
		Device->SetVertexDeclaration(VtxDecl);
		Device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0u, 0u, 4u, 0u, 2u);
		Fx->EndPass();
	}


	Fx->End();
}


void Engine::Renderer::SkyInitialize(const std::filesystem::path& FullPath)&
{
	_Sky.Initialize(FullPath, Device.get() );
}