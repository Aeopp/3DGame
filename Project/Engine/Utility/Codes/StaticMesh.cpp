#include <filesystem>
#include "StaticMesh.h"
#include "ResourceSystem.h"
#include <future>
#include <set>
#include <optional>
#include "UtilityGlobal.h"
#include "Renderer.h"

void Engine::StaticMesh::Initialize(
	const std::wstring& ResourceName)&
{
	Super::Initialize(Device);

	auto& ResourceSys = Engine::ResourceSystem::Instance;

	auto ProtoStaticMesh =
		(ResourceSys->GetAny<std::shared_ptr<Engine::StaticMesh>>(ResourceName));

	this->operator=(*ProtoStaticMesh);

	_ShaderFx.Initialize(L"DefaultFx");
}

void Engine::StaticMesh::Event(Object* Owner)&
{
	Super::Event(Owner);
}

void Engine::StaticMesh::Render(const Matrix& World,
								const Matrix& View,
                                const Matrix& Projection,
	                            const Vector4& CameraLocation4D)&
{
	Super::Render(World , View , Projection , CameraLocation4D);

	auto& Renderer = *Engine::Renderer::Instance;

	auto Fx = _ShaderFx.GetHandle();
	uint32 PassNum = 0u;
	Fx->Begin(&PassNum, 0);
	Fx->SetMatrix("World", &World);
	Fx->SetMatrix("View", &View);
	Fx->SetMatrix("Projection", &Projection);
	Fx->SetVector("LightDirection", &Renderer.LightDirection);
	Fx->SetVector("LightColor", &Renderer.LightColor);
	Fx->SetVector("CameraLocation", &CameraLocation4D);
	for (uint32 i = 0; i < PassNum; ++i)
	{
		Fx->BeginPass(i);
		for (auto& CurMesh : MeshContainer)
		{
			Fx->SetVector("RimAmtColor", &CurMesh.MaterialInfo.RimAmtColor);
			Fx->SetFloat("RimOuterWidth", CurMesh.MaterialInfo.RimOuterWidth);
			Fx->SetFloat("RimInnerWidth", CurMesh.MaterialInfo.RimInnerWidth);
			Fx->SetVector("AmbientColor", &CurMesh.MaterialInfo.AmbientColor);
			Fx->SetFloat("Power", CurMesh.MaterialInfo.Power);
			Fx->SetFloat("SpecularIntencity", CurMesh.MaterialInfo.SpecularIntencity);
			Fx->SetFloat("Contract", CurMesh.MaterialInfo.Contract);
			Fx->SetFloat("DetailDiffuseIntensity", CurMesh.MaterialInfo.DetailDiffuseIntensity);
			Fx->SetFloat("DetailNormalIntensity", CurMesh.MaterialInfo.DetailNormalIntensity);
			Fx->SetFloat("CavityCoefficient", CurMesh.MaterialInfo.CavityCoefficient);

			Fx->SetFloat("DetailScale", CurMesh.MaterialInfo.DetailScale);
			Device->SetVertexDeclaration(VtxDecl);
			Device->SetStreamSource(0, CurMesh.VertexBuffer, 0, CurMesh.Stride);
			Device->SetIndices(CurMesh.IndexBuffer);

			Fx->SetTexture("DiffuseMap", CurMesh.MaterialInfo.GetTexture(L"Diffuse"));
			Fx->SetTexture("NormalMap", CurMesh.MaterialInfo.GetTexture(L"Normal"));
			Fx->SetTexture("CavityMap", CurMesh.MaterialInfo.GetTexture(L"Cavity"));
			Fx->SetTexture("EmissiveMap", CurMesh.MaterialInfo.GetTexture(L"Emissive"));
			Fx->SetTexture("DetailDiffuseMap", CurMesh.MaterialInfo.GetTexture(L"DetailDiffuse"));
			Fx->SetTexture("DetailNormalMap", CurMesh.MaterialInfo.GetTexture(L"DetailNormal"));

			Fx->CommitChanges();
			Device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0u, 0u, CurMesh.VtxCount,
				0u, CurMesh.PrimitiveCount);
		}
		Fx->EndPass();
	}
	Fx->End();
}
