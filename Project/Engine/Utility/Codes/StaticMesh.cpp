#include <filesystem>
#include "StaticMesh.h"
#include "ResourceSystem.h"

void Engine::StaticMesh::Initialize(
	IDirect3DDevice9* const Device, 
	const RenderInterface::Group _Group,
	const std::wstring& MeshResourceName)&
{
	Super::Initialize(Device, _Group);

	auto& ResourceSys = ResourceSystem::Instance;
	static const std::wstring StaticMeshNaming = L"StaticMesh_";

	_Mesh=ResourceSys->Get<ID3DXMesh>(StaticMeshNaming + L"Mesh_" + MeshResourceName);
	Adjacency=ResourceSys->Get<ID3DXBuffer>(StaticMeshNaming + L"Adjacency_" + MeshResourceName);
	SubSet=ResourceSys->Get<ID3DXBuffer>(StaticMeshNaming + L"SubSet_" + MeshResourceName);
	SubSetCount=ResourceSys->GetAny<DWORD>(StaticMeshNaming + L"SubSetCount_" + MeshResourceName);
	Textures=ResourceSys->GetAny<std::vector<IDirect3DTexture9*>>(StaticMeshNaming + L"Textures_" + MeshResourceName);
	Materials = static_cast<D3DXMATERIAL*>(SubSet->GetBufferPointer());
}

void Engine::StaticMesh::Event(Object* Owner)&
{
	Super::Event(Owner);
}

void Engine::StaticMesh::Render()&
{
	Device->SetRenderState(D3DRS_LIGHTING, FALSE);
	Device->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);

	for (uint32 Idx = 0u; Idx < SubSetCount; ++Idx)
	{
		Device->SetTexture(0, Textures[Idx]);
		_Mesh->DrawSubset(Idx); 
	}
}
