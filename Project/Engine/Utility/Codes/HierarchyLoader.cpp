#include "HierarchyLoader.h"
#include "FMath.hpp"
#include <algorithm>
#include "ResourceSystem.h"

void Engine::HierarchyLoader::Initialize(IDirect3DDevice9* const Device, const std::filesystem::path& Path)&
{
	this->Device = Device;
	this->Path = Path;
}

STDMETHODIMP_(HRESULT __stdcall) Engine::HierarchyLoader::CreateFrame(
	LPCSTR Name, LPD3DXFRAME* NewFrame)
{
	Bone* _FrameSub = new Bone;
	ZeroMemory(_FrameSub, sizeof(Bone));
	AllocateName(&_FrameSub->Name, Name);
	_FrameSub->ToRootSpace = FMath::Identity(_FrameSub->TransformationMatrix);
	*NewFrame = _FrameSub;
	return S_OK;
}

STDMETHODIMP_(HRESULT __stdcall) Engine::HierarchyLoader::
CreateMeshContainer(
	LPCSTR Name, 
	const D3DXMESHDATA* MeshData, 
	const D3DXMATERIAL* Materials, 
	const D3DXEFFECTINSTANCE* EffectInstances, 
	DWORD NumMaterials, 
	const DWORD* Adjacency, 
	LPD3DXSKININFO SkinInfo, 
	LPD3DXMESHCONTAINER* NewMeshContainer)
{
	MeshContainerSub* _MeshContainerSub = new MeshContainerSub;
	ZeroMemory(_MeshContainerSub, sizeof(MeshContainerSub));
	AllocateName(&_MeshContainerSub->Name, Name);
	_MeshContainerSub->MeshData.Type = D3DXMESHTYPE_MESH;
	ID3DXMesh* Mesh = MeshData->pMesh;
	uint32 NumFaces = Mesh->GetNumFaces();
	_MeshContainerSub->pAdjacency = new DWORD[NumFaces *3u];

	std::memcpy(_MeshContainerSub->pAdjacency, Adjacency, sizeof(uint32) *
		NumFaces * 3u);
	uint32 FVF = Mesh->GetFVF();

	if (!(FVF & D3DFVF_NORMAL))
	{
		Mesh->CloneMeshFVF(Mesh->GetOptions(), FVF | D3DFVF_NORMAL
			, Device, &_MeshContainerSub->MeshData.pMesh);
		D3DXComputeNormals(_MeshContainerSub->MeshData.pMesh,
			_MeshContainerSub->pAdjacency);
	}
	else
	{
		Mesh->CloneMeshFVF(Mesh->GetOptions(),
			FVF, Device, &_MeshContainerSub->MeshData.pMesh);
	}

	_MeshContainerSub->NumMaterials = (NumMaterials == 0 ? 1 : NumMaterials);

	_MeshContainerSub->pMaterials = 
		new D3DXMATERIAL[_MeshContainerSub->NumMaterials];

	ZeroMemory(_MeshContainerSub->pMaterials, sizeof(D3DXMATERIAL) *
		_MeshContainerSub->NumMaterials);

	_MeshContainerSub->Textures.resize(_MeshContainerSub->NumMaterials);
	std::fill(std::begin(_MeshContainerSub->Textures),
		std::end(_MeshContainerSub->Textures), nullptr);

	if (0 != NumMaterials)
	{
		std::memcpy(_MeshContainerSub->pMaterials, Materials,
			sizeof(D3DXMATERIAL) *
			_MeshContainerSub->NumMaterials);

		for (uint32 i = 0; i < _MeshContainerSub->NumMaterials; ++i)
		{
			const std::filesystem::path 
				TexturePath = Path / _MeshContainerSub->pMaterials[i].pTextureFilename;

			auto & ResourceSys = ResourceSystem::Instance;
			_MeshContainerSub->Textures[i]  =ResourceSys->Get<IDirect3DTexture9>(TexturePath);
			if (_MeshContainerSub->Textures[i] == nullptr)
			{
				ResourceSys->Emplace<IDirect3DTexture9>(
					TexturePath, D3DXCreateTextureFromFile,
					Device, TexturePath.c_str(), &_MeshContainerSub->Textures[i]);
			};
		}
	}
	else
	{
		_MeshContainerSub->pMaterials[0].MatD3D.Diffuse =  D3DXCOLOR(0.f, 0.f, 0.f, 0.f);
		_MeshContainerSub->pMaterials[0].MatD3D.Specular = D3DXCOLOR(0.f, 0.f, 0.f, 0.f);
		_MeshContainerSub->pMaterials[0].MatD3D.Ambient =  D3DXCOLOR(0.f, 0.f, 0.f, 0.f);
		_MeshContainerSub->pMaterials[0].MatD3D.Emissive = D3DXCOLOR(0.f, 0.f, 0.f, 0.f);
		_MeshContainerSub->pMaterials[0].MatD3D.Power = 0.f;
		_MeshContainerSub->Textures[0] = nullptr;
	}

	if (nullptr == SkinInfo)
	{
		return S_OK; 
	}

	_MeshContainerSub->pSkinInfo = SkinInfo;
	_MeshContainerSub->pSkinInfo->AddRef();

	_MeshContainerSub->MeshData.pMesh->
		CloneMeshFVF(_MeshContainerSub->MeshData.pMesh->GetOptions(),
			_MeshContainerSub->MeshData.pMesh->GetFVF(), 
			Device, &_MeshContainerSub->OriginMesh);

	// 메쉬가 지닌 뼈의 개수를 반환하는 함수
	_MeshContainerSub->NumBones= _MeshContainerSub->pSkinInfo->GetNumBones();
	_MeshContainerSub->FrameOffsetMatrix.resize(_MeshContainerSub->NumBones);
	
	std::fill(std::begin(
		_MeshContainerSub->FrameOffsetMatrix), std::end(
			_MeshContainerSub->FrameOffsetMatrix), FMath::Identity());

	_MeshContainerSub->FrameCombinedMatrix.resize(_MeshContainerSub->NumBones);

	std::fill(
		std::begin(_MeshContainerSub->FrameCombinedMatrix),
		std::end(_MeshContainerSub->FrameCombinedMatrix),nullptr);

	_MeshContainerSub->RenderingMatrix.resize(_MeshContainerSub->NumBones);

	std::fill(
		std::begin (_MeshContainerSub->RenderingMatrix) , 
		std::end (_MeshContainerSub->RenderingMatrix) , FMath::Identity());

	for (uint32 i = 0; i < _MeshContainerSub->NumBones; ++i)
	{
		_MeshContainerSub->FrameCombinedMatrix[i] = 
			_MeshContainerSub->pSkinInfo->GetBoneOffsetMatrix(i);
	}

	*NewMeshContainer = _MeshContainerSub;

	return S_OK;
}

STDMETHODIMP_(HRESULT __stdcall) Engine::HierarchyLoader::DestroyFrame(LPD3DXFRAME BoneToFree)
{
	delete[] BoneToFree->Name;
	BoneToFree->Name = nullptr;

	if (nullptr != BoneToFree->pMeshContainer)
		DestroyMeshContainer(BoneToFree->pMeshContainer);

	if (nullptr != BoneToFree->pFrameSibling)
		DestroyFrame(BoneToFree->pFrameSibling);

	if (nullptr != BoneToFree->pFrameFirstChild)
		DestroyFrame(BoneToFree->pFrameFirstChild);

	delete BoneToFree;
	return S_OK;
}

STDMETHODIMP_(HRESULT __stdcall) Engine::HierarchyLoader::
DestroyMeshContainer(LPD3DXMESHCONTAINER MeshContainerToFree)
{
	MeshContainerSub* _MeshContainerSub =
		(MeshContainerSub*)MeshContainerToFree;

	(_MeshContainerSub->pSkinInfo)->Release();
	(_MeshContainerSub->MeshData.pMesh)->Release();
	(_MeshContainerSub->OriginMesh)->Release();

	if (_MeshContainerSub->pAdjacency)
		delete[] _MeshContainerSub->pAdjacency;
	if (_MeshContainerSub->pMaterials)
		delete[] _MeshContainerSub->pMaterials;
	if (_MeshContainerSub->Name)
		delete[] _MeshContainerSub->Name;

	delete _MeshContainerSub;

	return S_OK;
}

void Engine::HierarchyLoader::AllocateName(
	char** Name, const char* TargetName)&
{
	if (nullptr == TargetName)
		return;
	const uint32	Length = strlen(TargetName);
	*Name = new char[Length + 1u];
	strcpy_s(*Name, Length + 1u, TargetName);
}
