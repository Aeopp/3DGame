#include <filesystem>
#include "StaticMesh.h"
#include "ResourceSystem.h"
#include <future>
#include <set>
#include <optional>
#include "UtilityGlobal.h"
#include "Renderer.h"
#include "imgui.h"

void Engine::StaticMesh::Initialize(
	const std::wstring& ResourceName)&
{
	Super::Initialize(Device);

	auto& ResourceSys = Engine::ResourceSystem::Instance;

	auto ProtoStaticMesh =
		(*ResourceSys->GetAny<std::shared_ptr<Engine::StaticMesh>>(ResourceName));
	uint32 BackupID = ID; 
	this->operator=(*ProtoStaticMesh);
	ID = BackupID; 
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
	if (Engine::Global::bDebugMode)
	{
		ImGui::TextColored(ImVec4{ 1.f,114.f / 255.f, 198.f / 255.f , 1.0f }, "Draw : %s", ToA(ResourceName).c_str());
	}

	Super::Render(World , View , Projection , CameraLocation4D);

	auto& Renderer = *Engine::Renderer::Instance;


	Device->SetVertexDeclaration(VtxDecl);
	auto Fx = _ShaderFx.GetHandle();
	Fx->SetMatrix("World", &World);
	Fx->SetMatrix("View", &View);
	Fx->SetMatrix("Projection", &Projection);
	Fx->SetVector("LightDirection", &Renderer.LightDirection);
	Fx->SetVector("LightColor", &Renderer.LightColor);
	Fx->SetVector("CameraLocation", &CameraLocation4D);

	uint32 PassNum = 0u;
	Fx->Begin(&PassNum, 0);

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

		Device->SetStreamSource(0, CurMesh.VertexBuffer, 0, CurMesh.Stride);
		Device->SetIndices(CurMesh.IndexBuffer);

		Fx->SetTexture("DiffuseMap", CurMesh.MaterialInfo.GetTexture(L"Diffuse"));
		Fx->SetTexture("NormalMap", CurMesh.MaterialInfo.GetTexture(L"Normal"));
		Fx->SetTexture("CavityMap", CurMesh.MaterialInfo.GetTexture(L"Cavity"));
		Fx->SetTexture("EmissiveMap", CurMesh.MaterialInfo.GetTexture(L"Emissive"));
		Fx->SetTexture("DetailDiffuseMap", CurMesh.MaterialInfo.GetTexture(L"DetailDiffuse"));
		Fx->SetTexture("DetailNormalMap", CurMesh.MaterialInfo.GetTexture(L"DetailNormal"));

		Fx->CommitChanges();

		for (uint32 i = 0; i < PassNum; ++i)
		{
			Fx->BeginPass(i);
			Device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0u, 0u, CurMesh.VtxCount,
				0u, CurMesh.PrimitiveCount);
			Fx->EndPass();
		}
	}
	Fx->End();
}
