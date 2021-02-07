#include <filesystem>
#include "StaticMesh.h"
#include "ResourceSystem.h"
#include <future>
#include <set>
#include <optional>
#include "UtilityGlobal.h"

void Engine::StaticMesh::Initialize(
	const std::wstring& ResourceName)&
{
	Super::Initialize(Device);

	auto& ResourceSys = Engine::ResourceSystem::Instance;

	auto ProtoStaticMesh =
		(ResourceSys->GetAny<std::shared_ptr<Engine::StaticMesh>>(ResourceName));

	this->operator=(*ProtoStaticMesh);
}

void Engine::StaticMesh::Event(Object* Owner)&
{
	Super::Event(Owner);
}

void Engine::StaticMesh::Render()&
{
	Super::Render();

	Device->SetVertexShader(nullptr);
	Device->SetPixelShader(nullptr);

	for (auto& CurrentRenderMesh : MeshContainer)
	{
		Device->SetFVF(CurrentRenderMesh.FVF);
		Device->SetTexture(0, CurrentRenderMesh.DiffuseTexture);
		Device->SetStreamSource(0, CurrentRenderMesh.VertexBuffer, 0, CurrentRenderMesh.Stride);
		Device->SetIndices(CurrentRenderMesh.IndexBuffer);

		Device->DrawIndexedPrimitive(
			D3DPT_TRIANGLELIST, 0u, 0u, CurrentRenderMesh.VtxCount, 0u, CurrentRenderMesh.PrimitiveCount);
	}
}
