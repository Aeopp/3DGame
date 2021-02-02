#include <filesystem>
#include "DynamicMesh.h"
#include "ResourceSystem.h"

void Engine::DynamicMesh::Initialize(IDirect3DDevice9* const Device, const std::wstring& MeshResourceName)&
{
	Super::Initialize(Device);
	auto& ResourceSys = ResourceSystem::Instance;
	static const std::wstring StaticDynamicMeshNaming = L"DynamicMesh_";

	wchar_t		szFullPath[MAX_PATH] = L"";
	const wchar_t* pFilePath =
		L"..\\..\\..\\Resource\\Mesh\\DynamicMesh\\PlayerXFile\\";

	const wchar_t* pFileName = 
		L"Player.x";

	lstrcpy(szFullPath, pFilePath);
	lstrcat(szFullPath, pFileName);
	HierarchyLoader m_pLoader;
	m_pLoader.Initialize(Device, pFilePath);

	LPD3DXANIMATIONCONTROLLER	 pAniCtrl = nullptr;

	D3DXLoadMeshHierarchyFromX(szFullPath,
		D3DXMESH_MANAGED,
		Device,
		&m_pLoader, // HierarchyLoader
		NULL,
		reinterpret_cast<D3DXFRAME**>(&RootBone),
		&pAniCtrl);
	Engine::Aniamtion* _Animation;
	_Animation->Initialize(pAniCtrl);
	_Animation->Clone();

	Matrix matTemp;
	D3DXMatrixRotationY(&matTemp, D3DXToRadian(180.f));
	UpdateBone((Bone*)RootBone,matTemp);
	InitBoneToRootSpace(RootBone);

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

void Engine::DynamicMesh::Event(Object* Owner)&
{
	Super::Event(Owner);
}

void Engine::DynamicMesh::Render()&
{
	Super::Render();

	for (auto& iter : MeshContainer)
	{
		MeshContainerSub* _MeshContainer = iter;

		for (uint32 Idx = 0; Idx < _MeshContainer->NumBones; ++Idx)
		{
			_MeshContainer->RenderingMatrix[Idx] = _MeshContainer->FrameOffsetMatrix[Idx] *
				*_MeshContainer->FrameCombinedMatrix[Idx];
		}

		void* SourceVertex = nullptr;
		void* DestVertex = nullptr;

		_MeshContainer->OriginMesh->LockVertexBuffer(0, &SourceVertex);
		_MeshContainer->MeshData.pMesh->LockVertexBuffer(0, &DestVertex);

		// 소프트 웨어 스키닝을 수행하는 함수(스키닝 뿐 아니라 애니메이션 변경 시, 뼈대들과 정점 정보들의 변경을 동시에 수행)
		_MeshContainer->pSkinInfo->UpdateSkinnedMesh(
			_MeshContainer->RenderingMatrix.data(), NULL, SourceVertex, DestVertex);

		// 1인자 : 뼈의 최종적인 변환 상태
		// 2인자 : 원래대로 돌려놓기 위한 상태 행렬(본래는 1인자의 역행렬을 구해서 넣어줘야 하지만 안 넣어줘도 상관없음)
		// 3인자 : 변하지 않는 원본 메쉬의 정점 정보
		// 4인자 : 변환된 메쉬의 정점 정보

		for (uint32 Idx = 0; Idx < _MeshContainer->NumMaterials; ++Idx)
		{
			Device->SetTexture(0, _MeshContainer->Textures[Idx]);
			_MeshContainer->MeshData.pMesh->DrawSubset(Idx);
		}

		_MeshContainer->OriginMesh->UnlockVertexBuffer();
		_MeshContainer->MeshData.pMesh->UnlockVertexBuffer();
	}
}

const Engine::Bone* Engine::DynamicMesh::GetBone(const std::string BoneName)&
{
	return static_cast<Bone*>(D3DXFrameFind(RootBone, BoneName.c_str()));
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
	D3DXMatrixRotationY(&RotationY, 3.14f);
	UpdateBone(RootBone, RotationY);
}

void Engine::DynamicMesh::UpdateBone(
						Bone*const TargetBone,
						const Matrix& ToRootSpace)&
{
	if (nullptr == TargetBone)
		return;

	TargetBone->ToRootSpace = TargetBone->TransformationMatrix * (*ToRootSpace);

	if (nullptr != TargetBone->pFrameSibling)
		UpdateBone((Bone*const)TargetBone->pFrameSibling, ToRootSpace);

	if (nullptr != TargetBone->pFrameFirstChild)
		UpdateBone((Bone* const)TargetBone->pFrameFirstChild, TargetBone->ToRootSpace);
}

void Engine::DynamicMesh::InitBoneToRootSpace(Bone*const TargetBone)&
{
	if (nullptr != TargetBone->pMeshContainer)
	{
		MeshContainerSub*const  _TargetBoneMeshContainerSub =
			static_cast<MeshContainerSub* const>(TargetBone->pMeshContainer);

		for (uint32 Idx = 0u; Idx < _TargetBoneMeshContainerSub->NumBones; ++Idx)
		{
			const char* BoneName = _TargetBoneMeshContainerSub->pSkinInfo->GetBoneName(Idx);
			Bone* _Bone = static_cast<Bone* const>(D3DXFrameFind(
			RootBone,BoneName)); 
			
			_TargetBoneMeshContainerSub->FrameCombinedMatrix[Idx] = &_Bone->ToRootSpace;
		}

		MeshContainer.push_back(_TargetBoneMeshContainerSub);
	}

	if (nullptr != TargetBone->pFrameSibling)
		InitBoneToRootSpace(static_cast<Bone* const>(TargetBone->pFrameSibling));

	if (nullptr != TargetBone->pFrameFirstChild)
		InitBoneToRootSpace(static_cast<Bone* const>(TargetBone->pFrameFirstChild));
}
