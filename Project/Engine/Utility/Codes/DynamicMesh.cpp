#include <filesystem>
#include "DynamicMesh.h"
#include "ResourceSystem.h"

void Engine::DynamicMesh::PrototypeInitialize(
	IDirect3DDevice9* const Device ,
	const std::filesystem::path& Path,
	const std::filesystem::path& Name)&
{
	Super::PrototypeInitialize(Device);

	auto& ResourceSys = ResourceSystem::Instance;
	static const std::wstring StaticMeshNaming = L"StaticMesh_";
	Loader  = 
	//_Mesh=ResourceSys->Get<ID3DXMesh>(StaticMeshNaming + L"Mesh_" + MeshResourceName);
	//Adjacency=ResourceSys->Get<ID3DXBuffer>(StaticMeshNaming + L"Adjacency_" + MeshResourceName);
	//SubSet=ResourceSys->Get<ID3DXBuffer>(StaticMeshNaming + L"SubSet_" + MeshResourceName);
	//SubSetCount=ResourceSys->GetAny<DWORD>(StaticMeshNaming + L"SubSetCount_" + MeshResourceName);
	//Textures=ResourceSys->GetAny<std::vector<IDirect3DTexture9*>>(StaticMeshNaming + L"Textures_" + MeshResourceName);
	//Materials = static_cast<D3DXMATERIAL*>(SubSet->GetBufferPointer());
	//Stride = ResourceSys->GetAny<uint32>(StaticMeshNaming + L"Stride_" + MeshResourceName);
	//VertexCount = ResourceSys->GetAny<uint32>(StaticMeshNaming + L"VertexCount_" + MeshResourceName);
	//VertexLocations = ResourceSys->GetAny<decltype(VertexLocations)>
	//	(StaticMeshNaming + L"VertexLocations_" + MeshResourceName);
}

void Engine::DynamicMesh::Initialize()&
{
	Super::Initialize();
}

void Engine::DynamicMesh::Clone()&
{
	Super::Clone();
	_Animation.Clone();
}

void Engine::DynamicMesh::Event(Object* Owner)&
{
	Super::Event(Owner);
}

void Engine::DynamicMesh::Render()&
{
	Super::Render();
}

const Engine::Bone* Engine::DynamicMesh::GetBone(const std::string BoneName)&
{
	return static_cast<Bone*>(D3DXFrameFind(RootFrame, BoneName.c_str()));
}

bool Engine::DynamicMesh::IsAnimationEnd()&
{
	return _Animation.IsEnd();
}

void Engine::DynamicMesh::SetAnimationIdx(const uint8 AnimationIndex)&
{
	_Animation.Set(AnimationIndex);
}

void Engine::DynamicMesh::PlayAnimation(const float DeltaTime)&
{
	_Animation.Play(DeltaTime);
	Matrix RotationY{};
	UpdateBone(RootFrame, D3DXMatrixRotationY(&RotationY,
		3.14f));
}

void Engine::DynamicMesh::UpdateBone()&
{
}

void Engine::DynamicMesh::InitBoneRefMatrix()&
{
}
