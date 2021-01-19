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

	ResourceSys->Get<ID3DXMesh>(StaticMeshNaming + L"Mesh_" + MeshResourceName);
	ResourceSys->Get<ID3DXBuffer>(StaticMeshNaming + L"Adjacency_" + MeshResourceName);
	ResourceSys->Get<ID3DXBuffer>(StaticMeshNaming + L"SubSet_" + MeshResourceName);
	ResourceSys->GetAny<DWORD>(StaticMeshNaming + L"SubSetCount_" + MeshResourceName);
	ResourceSys->GetAny<std::vector<IDirect3DTexture9*>>(StaticMeshNaming + L"Textures_" + MeshResourceName);
}

void Engine::StaticMesh::Initialize(IDirect3DDevice9* const Device, const RenderInterface::Group _Group, const std::wstring& MeshResourceName)&
{
}

void Engine::StaticMesh::Event(Object* Owner)&
{
	Super::Event(Owner);
}
