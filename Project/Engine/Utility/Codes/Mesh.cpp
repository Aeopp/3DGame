#include "Mesh.h"
#include "Renderer.h"
#include "StringHelper.h"
#include "Transform.h"

void Engine::Mesh::Initialize(
	IDirect3DDevice9* const Device,
	const RenderInterface::Group _Group)&
{
	Super::Initialize();
	this->Device = Device;
	RenderInterface::SetUpRenderingInformation(_Group);
};

void Engine::Mesh::Event(Object* Owner)&
{
	Super::Event(Owner);
	RenderInterface::Regist();
	this->Owner = Owner;
}
void Engine::Mesh::Render(Engine::Renderer* const _Renderer)&
{
	if (nullptr == Device)
		return;
	if (bCurrentFrustumIn == false)return;

	auto Fx =ForwardShaderFx.GetHandle();
	auto& Renderer = *Engine::Renderer::Instance;
	const auto& RenderInfo = Renderer.GetCurrentRenderInformation();
	const auto& _RefDeferredPass = Renderer.RefDeferredPass(); 
	Fx->SetMatrix("World", &OwnerWorld);
	Fx->SetFloat("FogDistance", Renderer.FogDistance);
	Fx->SetFloatArray("FogColor", Renderer.FogColor, 3u);
	Fx->SetMatrix("View", &RenderInfo.View);
	Fx->SetMatrix("Projection", &RenderInfo.Projection);
	Fx->SetVector("LightDirection", &Renderer._DirectionalLight._LightInfo.Direction);
	Fx->SetVector("LightColor", &Renderer._DirectionalLight._LightInfo.LightColor);
	Fx->SetVector("CameraLocation", &RenderInfo.CameraLocation4D);
	Fx->SetTexture("ShadowDepthMap",  Renderer.RefDeferredPass().ShadowDepth.GetTexture());
	Fx->SetMatrix("LightViewProjection", &RenderInfo.LightViewProjection);
	Fx->SetFloat("ShadowDepthMapWidth", Renderer._DirectionalLight._LightInfo.ShadowDepthMapWidth);
	Fx->SetFloat("ShadowDepthMapHeight", Renderer._DirectionalLight._LightInfo.ShadowDepthMapHeight );
	Fx->SetFloat("ShadowDepthBias", Renderer._DirectionalLight._LightInfo.ShadowDepthBias);

	Device->SetVertexDeclaration(VtxDecl);
	uint32 PassNum = 0u;
	Fx->Begin(&PassNum, 0);

	for (auto& CurrentRenderMesh : MeshContainer)
	{
		Device->SetStreamSource(0, CurrentRenderMesh.VertexBuffer, 0, CurrentRenderMesh.Stride);
		Device->SetIndices(CurrentRenderMesh.IndexBuffer);

		Fx->SetVector("RimAmtColor", &CurrentRenderMesh.MaterialInfo.RimAmtColor);
		Fx->SetFloat("RimOuterWidth", CurrentRenderMesh.MaterialInfo.RimOuterWidth);
		Fx->SetFloat("RimInnerWidth", CurrentRenderMesh.MaterialInfo.RimInnerWidth);
		Fx->SetFloat("Power", CurrentRenderMesh.MaterialInfo.Power);
		Fx->SetFloat("SpecularIntencity", CurrentRenderMesh.MaterialInfo.SpecularIntencity);
		Fx->SetFloat("Contract", CurrentRenderMesh.MaterialInfo.Contract);
		Fx->SetFloat("DetailScale", CurrentRenderMesh.MaterialInfo.DetailScale);
		Fx->SetFloat("DetailDiffuseIntensity", CurrentRenderMesh.MaterialInfo.DetailDiffuseIntensity);
		Fx->SetFloat("DetailNormalIntensity", CurrentRenderMesh.MaterialInfo.DetailNormalIntensity);
		Fx->SetFloat("CavityCoefficient", CurrentRenderMesh.MaterialInfo.CavityCoefficient);
		Fx->SetFloat("AlphaAddtive", CurrentRenderMesh.MaterialInfo.AlphaAddtive);
		
		CurrentRenderMesh.MaterialInfo.BindingTexture(Fx);
		Fx->CommitChanges();

		for (uint32 i = 0; i < PassNum; ++i)
		{
			Fx->BeginPass(i);

			Device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0u, 0u,
				CurrentRenderMesh.VtxCount, 0u, CurrentRenderMesh.PrimitiveCount);

			Fx->EndPass();
		}
	}
	Fx->End();
};
void Engine::Mesh::RenderDeferredAlbedoNormalVelocityDepthSpecularRim(
	Engine::Renderer* const _Renderer)&
{
	if (nullptr == Device)
		return;
	if (bCurrentFrustumIn == false)
		return;

	const auto& RenderInfo  = _Renderer->GetCurrentRenderInformation();
	const auto& PrevRenderInfo = _Renderer->GetPrevRenderInformation();

	Device->SetVertexDeclaration(VtxDecl);
	ID3DXEffect* Fx = nullptr;

	if (_DissolveInfo.has_value())
	{
		Fx = DeferredDissolveFx.GetHandle();
		Fx->SetFloat("BurnSize", _DissolveInfo->BurnSize);
		Fx->SetFloat("EmissionAmount", _DissolveInfo->EmissionAmount);
		Fx->SetFloat("SliceAmount",_DissolveInfo->SliceAmount); 
		Fx->SetTexture("DissolveMap", _Renderer->GetDissolveTexture());

		if (_DissolveInfo->bBlueBurn)
		{
			Fx->SetTexture("BurnMap", _Renderer->GetBlueBurnTexture());
		}
		else
		{
			Fx->SetTexture("BurnMap", _Renderer->GetBurnTexture());
		}
	}
	else
	{
		Fx = DeferredDefaultFx.GetHandle();
	}

	Fx->SetMatrix("View", &RenderInfo.View);
	Fx->SetMatrix("Projection", &RenderInfo.Projection);
	Fx->SetMatrix("World", &OwnerWorld);

	const Matrix PrevWorldViewProjection =PrevWorld* PrevRenderInfo.ViewProjection;
	Fx->SetMatrix("PrevWorldViewProjection", &PrevWorldViewProjection);
	Fx->SetFloat("OutlineRedFactor", OutlineRedFactor);

	uint32 Pass = 0u;
	Fx->Begin(&Pass,NULL);
	{
		for (uint32 i = 0; i < Pass; ++i)
		{
			Fx->BeginPass(i);
			{
				for (auto& CurMesh : MeshContainer)
				{
					Device->SetStreamSource(0, CurMesh.VertexBuffer, 0, CurMesh.Stride);
					Device->SetIndices(CurMesh.IndexBuffer);
					Fx->SetFloat("DetailScale", CurMesh.MaterialInfo.DetailScale);
					Fx->SetFloat("Contract", CurMesh.MaterialInfo.Contract);
					Fx->SetFloat("Power", CurMesh.MaterialInfo.Power);
					Fx->SetFloat("SpecularIntencity", CurMesh.MaterialInfo.SpecularIntencity);
					Fx->SetFloat("DetailDiffuseIntensity", CurMesh.MaterialInfo.DetailDiffuseIntensity);
					Fx->SetFloat("DetailNormalIntensity", CurMesh.MaterialInfo.DetailNormalIntensity);
					Fx->SetFloat("CavityCoefficient", CurMesh.MaterialInfo.CavityCoefficient);
					Fx->SetFloat("RimOuterWidth", CurMesh.MaterialInfo.RimOuterWidth);
					Fx->SetFloat("RimInnerWidth", CurMesh.MaterialInfo.RimInnerWidth);
					Fx->SetVector("RimAmtColor", &CurMesh.MaterialInfo.RimAmtColor);
					

					CurMesh.MaterialInfo.BindingTexture(Fx);
					Fx->CommitChanges();

					Device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0u, 0u, CurMesh.VtxCount,0u, CurMesh.PrimitiveCount);
				}
			}
			Fx->EndPass();
		}
	}
	Fx->End();
}; 
void Engine::Mesh::RenderShadowDepth(Engine::Renderer* const _Renderer)&
{	
	if (nullptr == Device)
		return;

	Device->SetVertexDeclaration(VtxDecl);
	auto Fx = DepthShadowFx.GetHandle();
	const auto& RenderInfo = _Renderer->GetCurrentRenderInformation();
	Fx->SetMatrix("LightViewProjection", &RenderInfo.LightViewProjection);
	Fx->SetMatrix("World", &OwnerWorld);

	uint32 Pass = 0u;
	Fx->Begin(&Pass, NULL);
	{
		for (uint32 i = 0; i < Pass; ++i)
		{
			Fx->BeginPass(i);
			{
				for (auto& CurMesh : MeshContainer)
				{
					Device->SetStreamSource(0, CurMesh.VertexBuffer, 0, CurMesh.Stride);
					Device->SetIndices(CurMesh.IndexBuffer);
					Fx->CommitChanges();

					Device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0u, 0u, CurMesh.VtxCount, 0u, CurMesh.PrimitiveCount);
				}
			}
			Fx->EndPass();
		}
	}
	Fx->End();
}; 
void Engine::Mesh::RenderReady(Engine::Renderer* const _Renderer)&
{
	PrevWorld = OwnerWorld;
	PrevView = _Renderer->GetPrevRenderInformation().View;
	OwnerWorld = Owner->GetComponent<Engine::Transform>()->UpdateWorld();

}
void Engine::Mesh::RenderVelocity(Engine::Renderer* const _Renderer)&
{
	if (nullptr == Device)
		return;

	Device->SetVertexDeclaration(VtxDecl);
	auto Fx = VelocityFx.GetHandle();
	const auto& RenderInfo = _Renderer->GetCurrentRenderInformation();
	const auto& PrevRenderInfo = _Renderer->GetPrevRenderInformation();

	const Matrix WorldView = OwnerWorld* RenderInfo.View;
	const Matrix PrevWorldView = PrevWorld* PrevRenderInfo.View;
	
	Fx->SetMatrix("WorldView", &WorldView);
	Fx->SetMatrix("PrevWorldView", &PrevWorldView);
	Fx->SetMatrix("Projection", &RenderInfo.Projection);

	uint32 Pass = 0u;
	Fx->Begin(&Pass, NULL);
	{
		for (uint32 i = 0; i < Pass; ++i)
		{
			Fx->BeginPass(i);
			{
				for (auto& CurMesh : MeshContainer)
				{
					Device->SetStreamSource(0, CurMesh.VertexBuffer, 0, CurMesh.Stride);
					Device->SetIndices(CurMesh.IndexBuffer);
					Fx->CommitChanges();
					Device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0u, 0u, CurMesh.VtxCount, 0u, CurMesh.PrimitiveCount);
				}
			}
			Fx->EndPass();
		}
	}
	Fx->End();
}
;