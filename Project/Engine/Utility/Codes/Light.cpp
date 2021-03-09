#include "Light.h"
#include "imgui.h"
#include "FMath.hpp"
#include "ResourceSystem.h"
#include "UtilityGlobal.h"
#include <array>
#include "StringHelper.h"
#include "Timer.h"
#include "Renderer.h"

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

void Engine::Light::Render(
	IDirect3DDevice9* const Device, 
	const Vector3& CameraLocation, 
	const Matrix& View, 
	const Matrix& Projection, 
	IDirect3DTexture9* Albedo3_Contract1, IDirect3DTexture9* Normal3_Power1,
	IDirect3DTexture9* WorldPos3_Depth1, 
	IDirect3DTexture9* CavityRGB1_RimRGB1_RimInnerWidth1_RimOuterWidth1, 
	IDirect3DTexture9* ShadowDepth ,
	const Vector3& FogColor,
	const float FogDistance )&
{
	auto& _Renderer = Engine::Renderer::Instance;

	const auto& CurRenderInfo = _Renderer->GetCurrentRenderInformation();
	const auto& PrevRenderInfo = _Renderer->GetPrevRenderInformation();

	auto Fx = _DeferredLight.GetHandle();
	uint32 Pass = 0u;
	Fx->Begin(&Pass, NULL);
	Fx->SetMatrix("View", &View);
	Fx->SetMatrix("Projection", &Projection);
	Fx->SetMatrix("InverseViewProjection", &CurRenderInfo.InverseViewProjection);
	Fx->SetMatrix("ViewProjection", &CurRenderInfo.ViewProjection);

	Fx->SetMatrix("PrevView", &PrevRenderInfo.View);
	Fx->SetMatrix("PrevProjection", &PrevRenderInfo.Projection);
	Fx->SetMatrix("PrevInverseViewProjection", &PrevRenderInfo.InverseViewProjection );
	Fx->SetMatrix("PrevViewProjection", &PrevRenderInfo.ViewProjection);

	const Matrix LightViewProjection =CalcLightViewProjection();
	Fx->SetFloat("ShadowDepthMapWidth", _LightInfo.ShadowDepthMapWidth);
	Fx->SetFloat("ShadowDepthMapHeight", _LightInfo.ShadowDepthMapHeight);
	Fx->SetFloatArray("FogColor", FogColor,3u);
	Fx->SetFloat("FogDistance", FogDistance);
	Fx->SetMatrix("LightViewProjection", &LightViewProjection);
	Fx->SetFloat("ShadowDepthBias", _LightInfo.ShadowDepthBias);
	
	///
	Fx->SetVector("LightLocation",&_LightInfo.Location);

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
		Fx->SetTexture("CavityRGB1_RimRGB1_RimInnerWidth1_RimOuterWidth1", CavityRGB1_RimRGB1_RimInnerWidth1_RimOuterWidth1);
		Fx->SetTexture("ShadowDepth", ShadowDepth);
		Fx->SetTexture("VelocityMap", _Renderer->RefDeferredPass().VelocityMap.GetTexture());
		Fx->CommitChanges();
		Device->SetIndices(IdxBuf);
		Device->SetStreamSource(0u, VtxBuf, 0u, sizeof(Vertex::Screen));
		 Device->SetVertexDeclaration(VtxDecl);
		Device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0u, 0u, 4u, 0u, 2u);
		Fx->EndPass();
	};
	
	Fx->End();
}
Matrix Engine::Light::CalcLightViewProjection() const&
{
	Matrix LightView, LightProjection;
	static const Vector3 Up = { 0,1,0 };
	const  Vector3 LightLocation3D =
	{ _LightInfo.Location.x,_LightInfo.Location.y,_LightInfo.Location.z };
	const  Vector3 LightDirection3D =
	{ _LightInfo.Direction.x,_LightInfo.Direction.y,_LightInfo.Direction.z };
	const Vector3 At = LightLocation3D + LightDirection3D;

	D3DXMatrixLookAtLH(&LightView, &LightLocation3D, &At, &Up);

	D3DXMatrixOrthoLH(&LightProjection, _LightInfo.ShadowDepthMapWidth * _LightInfo.ShadowOrthoProjectionFactor, _LightInfo.ShadowDepthMapHeight * _LightInfo.ShadowOrthoProjectionFactor,
		0.1f, _LightInfo.ShadowFar);

	return LightView * LightProjection;
};

