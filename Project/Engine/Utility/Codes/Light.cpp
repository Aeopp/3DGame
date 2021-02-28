#include "Light.h"
#include "imgui.h"
#include "FMath.hpp"
#include "ResourceSystem.h"
#include "UtilityGlobal.h"
#include <array>
#include "StringHelper.h"
#include "Timer.h"
#include "Vertexs.hpp"

static uint32 LightID = 0u;

Engine::Light::Light() : 
	ID{ LightID++ } 
{
}

void Engine::Light::Initialize(
	IDirect3DDevice9* const Device,
	const LightInformation& SetLightInformation)&
{
	auto& ResourceSys = ResourceSystem::Instance;

	VtxBuf = ResourceSys->Get<IDirect3DVertexBuffer9>(L"LightVertexBuffer");
	IdxBuf = ResourceSys->Get<IDirect3DIndexBuffer9>(L"LightIndexBuffer");
	VtxDecl = ResourceSys->Get<IDirect3DVertexDeclaration9>(L"LightVertexDecl");

	_DeferredLight.Initialize(L"DeferredLightFx");

	_LightInfo = SetLightInformation;
};

void Engine::Light::Render(IDirect3DDevice9* const Device, const Vector3& CameraLocation, 
	const Matrix& View, const Matrix& Projection, IDirect3DTexture9* Albedo3_Contract1, IDirect3DTexture9* Normal3_Power1, IDirect3DTexture9* WorldPos3_Depth1, IDirect3DTexture9* CavityRGB1_CavityAlpha1, IDirect3DTexture9* RimRGB1_InnerWidth1_OuterWidth1)&
{
	auto Fx = _DeferredLight.GetHandle();
	uint32 Pass = 0u;
	Fx->Begin(&Pass, NULL);
	Fx->SetMatrix("View", &View);
	Fx->SetMatrix("Projection", &Projection);
	const Vector4 CameraLocation4D = FMath::ConvertVector4(CameraLocation, 1.f);

	Fx->SetVector("CameraLocation", &CameraLocation4D);
	Fx->SetVector("LightDirection", &_LightInfo.Direction);
	Fx->SetVector("LightColor",  &_LightInfo.LightColor);

	for (uint32 i = 0; i < Pass; ++i)
	{
		Fx->BeginPass(i);
		Fx->SetTexture("Albedo3_Contract1", Albedo3_Contract1);
		Fx->SetTexture("Normal3_Power1", Normal3_Power1);
		Fx->SetTexture("WorldPos3_Depth1", WorldPos3_Depth1);
		Fx->SetTexture("CavityRGB1_CavityAlpha1", CavityRGB1_CavityAlpha1);
		Fx->SetTexture("RimRGB1_InnerWidth1_OuterWidth1", RimRGB1_InnerWidth1_OuterWidth1);
		Fx->CommitChanges();
		Device->SetIndices(IdxBuf);
		Device->SetStreamSource(0u, VtxBuf, 0u, sizeof(Vertex::Screen));
		 Device->SetVertexDeclaration(VtxDecl);
		Device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0u, 0u, 4u, 0u, 2u);
		Fx->EndPass();
	};
	
	Fx->End();
};


