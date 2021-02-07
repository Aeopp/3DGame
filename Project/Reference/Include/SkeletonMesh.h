#pragma once
#include "Mesh.h"
#include "Vertexs.hpp"
#include <vector>
#include <unordered_map>
#include <string>
#include "FMath.hpp"
#include <filesystem>
#include "DllHelper.h"
#include "Bone.h"
#include "UtilityGlobal.h"
#include "ResourceSystem.h"
#include "ExportUtility.hpp"
#include <optional>
#include <set>


namespace Engine
{
	struct DLL_DECL SkinningMeshElement : public MeshElement
	{
		std::vector<std::vector<float>>     Weights{};
		std::vector<std::vector<Matrix*>>   Finals{};
	};

	class DLL_DECL SkeletonMesh : public Mesh
	{
	public:
		using Super = Mesh;
	public:
		SkeletonMesh() = default;
		SkeletonMesh(const SkeletonMesh&) = default;
		SkeletonMesh(SkeletonMesh&&)noexcept = default;
		SkeletonMesh& operator=(const SkeletonMesh&) = default;
		SkeletonMesh& operator=(SkeletonMesh&&)noexcept = default;
	public:
		template<typename VertexType>
		void  Load(IDirect3DDevice9* const Device,
			const std::filesystem::path FilePath,
			const std::filesystem::path FileName,
			const std::wstring ResourceName)&;
		void  Initialize(const std::wstring& ResourceName)&;
		void  Event(class Object* Owner) & override;
		void  Render() & override;
		void  Update(Object* const Owner, const float DeltaTime)&;
		Bone* MakeHierarchy(Bone* BoneParent, const aiNode* const AiNode);
		void  PlayAnimation(const uint32 AnimIdx, const double Acceleration)&;
	public:
		static const inline Property          TypeProperty = Property::Render;
		std::shared_ptr<std::vector<Vector3>> LocalVertexLocations;
		uint32    AnimIdx{ 0u };
		double T{ 0.0f };
		uint32 NumMaxRefBone{ 0u };
		uint32 MaxAnimIdx{ 0u };
	private:
		double Acceleration = 1.f;
		std::vector       <std::unordered_map<std::string, aiNodeAnim*>>  AnimTable{};
		std::shared_ptr<AnimationTrack>                            _AnimationTrack{};
		std::unordered_map<std::string, Bone>                             BoneTable{};
		Bone* RootBone{ nullptr };
		std::vector       <SkinningMeshElement>                          MeshContainer{};
	};
}

static std::optional<aiNode* const>  FindRootBone (
	const std::set<std::string>& BoneNameSet,
	aiNode* const TargetNode)
{
	auto  TargetName = TargetNode->mName.C_Str();

	if (TargetNode->mParent)
	{
		const std::string ParentName = TargetNode->mParent->mName.C_Str();
		if (BoneNameSet.contains(TargetName) && !BoneNameSet.contains(ParentName))
		{
			return  { TargetNode };
		}
		else
		{
			for (uint32 i = 0; i < TargetNode->mNumChildren; ++i)
			{
				auto Result= FindRootBone(BoneNameSet, TargetNode->mChildren[i]);
				if (Result.has_value())
				{
					return Result;
				}
			}
		}
	}
	else
	{
		for (uint32 i = 0; i < TargetNode->mNumChildren; ++i)
		{
			auto Result = FindRootBone(BoneNameSet, TargetNode->mChildren[i]);
			if (Result.has_value())
			{
				return Result;
			}
		}
	}

	return std::nullopt;
};


template<typename VertexType>
void Engine::SkeletonMesh::Load(IDirect3DDevice9* const Device,
	const std::filesystem::path FilePath,
	const std::filesystem::path FileName,
	const std::wstring ResourceName)&
{
	this->Device = Device;
	// 모델 생성 플래그 , 같은 플래그를 두번, 혹은 호환이 안되는
	// 플래그가 겹칠 경우 런타임 에러이며 에러 핸들링이
	// 어려우므로 매우 유의 할 것.
	AiScene = Engine::Global::AssimpImporter.ReadFile(
		(FilePath / FileName).string(),
		aiProcess_MakeLeftHanded |
		aiProcess_FlipUVs |
		aiProcess_FlipWindingOrder |
		aiProcess_Triangulate |
		aiProcess_CalcTangentSpace |
		aiProcess_ValidateDataStructure |
		aiProcess_ImproveCacheLocality |
		aiProcess_RemoveRedundantMaterials |
		aiProcess_GenUVCoords |
		aiProcess_TransformUVCoords |
		aiProcess_FindInstances |
		aiProcess_GenSmoothNormals |
		aiProcess_SortByPType |
		aiProcess_OptimizeMeshes |
		aiProcess_OptimizeGraph |
		aiProcess_SplitLargeMeshes
	);
	/// <summary>
/// 1. 메시가 참조하는 본이름의 집합을 구성 = A
/// 2. 루트의 조건은 A에 포함 안되있고 자식이 본이어야함 = R
/// 3. R 의 부모의 정보는 필요없음 R부터 로직 시작.
/// </summary>
	std::set<std::string> BoneNameSet;
	for (uint32 i = 0; i < AiScene->mNumMeshes; ++i)
	{
		BoneNameSet.insert(AiScene->mMeshes[i]->mName.C_Str());
	}

	auto Result = FindRootBone(BoneNameSet, AiScene->mRootNode);
	aiNode* ResultRoot = *Result;




	static uint32 SkeletonResourceID = 0u;
	auto& ResourceSys = RefResourceSys();
	MaxAnimIdx = AiScene->mNumAnimations;
	// Bone Info 
	Bone* Root = &(BoneTable[AiScene->mRootNode->mName.C_Str()] = Bone{});
	Root->Name = AiScene->mRootNode->mName.C_Str();
	Root->OriginTransform = Root->Transform = FromAssimp(AiScene->mRootNode->mTransformation);
	Root->Parent = nullptr;
	Root->ToRoot = Root->OriginTransform;  *FMath::Identity();
	for (uint32 i = 0; i < AiScene->mRootNode->mNumChildren; ++i)
	{
		Root->Childrens.push_back(MakeHierarchy(Root, AiScene->mRootNode->mChildren[i]));
	}
	RootBone = Root;
	LocalVertexLocations = std::make_shared<std::vector<Vector3>>();
	for (uint32 MeshIdx = 0u; MeshIdx < AiScene->mNumMeshes; ++MeshIdx)
	{
		const std::wstring CurrentResourceName =
			ResourceName + L"_" + std::to_wstring(SkeletonResourceID++);

		SkinningMeshElement CreateMesh{};
		aiMesh* _AiMesh = AiScene->mMeshes[MeshIdx];
		// 버텍스 버퍼.
		std::shared_ptr<std::vector<VertexType>> Verticies = std::make_shared<std::vector<VertexType>>();

		for (uint32 VerticesIdx = 0u; VerticesIdx < _AiMesh->mNumVertices; ++VerticesIdx)
		{
			Verticies->push_back(VertexType::MakeFromAssimpMesh(_AiMesh, VerticesIdx));
			LocalVertexLocations->push_back(FromAssimp(_AiMesh->mVertices[VerticesIdx]));
		}

		const std::wstring MeshVtxBufResourceName =
			L"SkeletonMesh_VertexBuffer_" + CurrentResourceName;
		IDirect3DVertexBuffer9* _VertexBuffer{ nullptr };
		CreateMesh.VtxBufSize = sizeof(VertexType) * Verticies->size();
		Device->CreateVertexBuffer(CreateMesh.VtxBufSize, D3DUSAGE_DYNAMIC, VertexType::FVF,
			D3DPOOL_DEFAULT, &_VertexBuffer, nullptr);
		CreateMesh.VertexBuffer = ResourceSys.Insert<IDirect3DVertexBuffer9>(MeshVtxBufResourceName, _VertexBuffer);
		CreateMesh.PrimitiveCount = CreateMesh.FaceCount = _AiMesh->mNumFaces;
		VertexType* VertexBufferPtr{ nullptr };
		CreateMesh.VertexBuffer->Lock(0, 0, reinterpret_cast<void**>(&VertexBufferPtr), NULL);
		std::memcpy(VertexBufferPtr, Verticies->data(), CreateMesh.VtxBufSize);
		CreateMesh.VertexBuffer->Unlock();
		CreateMesh.VtxCount = Verticies->size();
		CreateMesh.Stride = sizeof(VertexType);
		CreateMesh.FVF = VertexType::FVF;
		// 인덱스 버퍼.
		std::vector<uint32> Indicies{};
		for (uint32 FaceIdx = 0u; FaceIdx < _AiMesh->mNumFaces; ++FaceIdx)
		{
			const aiFace CurrentFace = _AiMesh->mFaces[FaceIdx];
			for (uint32 Idx = 0u; Idx < CurrentFace.mNumIndices; ++Idx)
			{
				Indicies.push_back(CurrentFace.mIndices[Idx]);
			}
		}
		const std::wstring MeshIdxBufResourceName =
			L"SkeletonMesh_IndexBuffer_" + CurrentResourceName;
		IDirect3DIndexBuffer9* _IndexBuffer{ nullptr };
		const uint32 IdxBufSize = sizeof(uint32) * Indicies.size();
		Device->CreateIndexBuffer(IdxBufSize, D3DUSAGE_WRITEONLY, D3DFMT_INDEX32,
			D3DPOOL_MANAGED, &_IndexBuffer, nullptr);
		CreateMesh.IndexBuffer = ResourceSys.Insert<IDirect3DIndexBuffer9>(MeshIdxBufResourceName,
			_IndexBuffer);
		uint32* IndexBufferPtr{ nullptr };
		CreateMesh.IndexBuffer->Lock(0, 0, reinterpret_cast<void**>(&IndexBufferPtr), NULL);
		std::memcpy(IndexBufferPtr, Indicies.data(), IdxBufSize);
		CreateMesh.IndexBuffer->Unlock();
		CreateMesh.Verticies = Verticies;
		CreateMesh.VerticiesPtr = Verticies->data();
		// 머테리얼.
		aiMaterial* AiMaterial = AiScene->mMaterials[_AiMesh->mMaterialIndex];
		if (AiMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0)
		{
			aiString AiFileName;

			const aiReturn AiReturn = AiMaterial->
				GetTexture(aiTextureType_DIFFUSE, 0, &AiFileName, NULL, NULL, NULL, NULL, NULL);

			if (AiReturn == AI_SUCCESS)
			{
				const std::filesystem::path TexFileFullPath = FilePath / AiFileName.C_Str();
				const std::wstring TexResourceName = L"SkeletonMesh_DiffuseTexture_" + CurrentResourceName;

				ResourceSys.Emplace<IDirect3DTexture9>
					(TexResourceName,
						D3DXCreateTextureFromFile, Device, TexFileFullPath.c_str(), &CreateMesh.DiffuseTexture);
			}
		}
		if (AiMaterial->GetTextureCount(aiTextureType::aiTextureType_NORMALS) > 0)
		{
			aiString AiFileName;

			const aiReturn AiReturn = AiMaterial->
				GetTexture(aiTextureType::aiTextureType_NORMALS, 0, &AiFileName, NULL, NULL, NULL, NULL, NULL);

			if (AiReturn == AI_SUCCESS)
			{
				const std::filesystem::path TexFileFullPath = FilePath / AiFileName.C_Str();
				const std::wstring TexResourceName = L"SkeletonMesh_NormalTexture_" + CurrentResourceName;

				ResourceSys.Emplace<IDirect3DTexture9>
					(TexResourceName,
						D3DXCreateTextureFromFile, Device, TexFileFullPath.c_str(), &CreateMesh.NormalTexture);
			}
		}
		if (AiMaterial->GetTextureCount(aiTextureType::aiTextureType_SPECULAR) > 0)
		{
			aiString AiFileName;

			const aiReturn AiReturn = AiMaterial->
				GetTexture(aiTextureType::aiTextureType_SPECULAR, 0, &AiFileName, NULL, NULL, NULL, NULL, NULL);

			if (AiReturn == AI_SUCCESS)
			{
				const std::filesystem::path TexFileFullPath = FilePath / AiFileName.C_Str();
				const std::wstring TexResourceName = L"SkeletonMesh_SpecularTexture_" + CurrentResourceName;

				ResourceSys.Emplace<IDirect3DTexture9>
					(TexResourceName,
						D3DXCreateTextureFromFile, Device, TexFileFullPath.c_str(), &CreateMesh.SpecularTexture);
			}
		}

		// Vtx Bone 정보,
		if (_AiMesh->HasBones())
		{
			CreateMesh.Weights.resize(CreateMesh.VtxCount);
			CreateMesh.Finals.resize(CreateMesh.VtxCount);
			NumMaxRefBone = (std::max)(_AiMesh->mNumBones , NumMaxRefBone);
			for (uint32 BoneIdx = 0u; BoneIdx < _AiMesh->mNumBones; ++BoneIdx)
			{
				const aiBone* const CurVtxBone = _AiMesh->mBones[BoneIdx];
				auto iter = BoneTable.find(CurVtxBone->mName.C_Str());
				if (iter != std::end(BoneTable))
				{
					for (uint32 WeightIdx = 0u; WeightIdx < CurVtxBone->mNumWeights; ++WeightIdx)
					{
						const aiVertexWeight  _AiVtxWit = CurVtxBone->mWeights[WeightIdx];
						const uint32 VtxIdx = _AiVtxWit.mVertexId;
						const float _Wit = _AiVtxWit.mWeight;
						const Matrix OffsetMatrix = FromAssimp(CurVtxBone->mOffsetMatrix);
						iter->second.Offset = OffsetMatrix;
						CreateMesh.Weights[VtxIdx].push_back(_Wit);
						CreateMesh.Finals[VtxIdx].push_back(&(iter->second.Final));
					}
				}
			}
		}





		MeshContainer.push_back(CreateMesh);
	}

	// 애니메이션 데이터 파싱.
	if (AiScene->HasAnimations())
	{
		AnimTable.resize(AiScene->mNumAnimations);
		_AnimationTrack = std::make_shared<AnimationTrack>();
		_AnimationTrack->ScaleTimeLine.resize(AiScene->mNumAnimations);
		_AnimationTrack->QuatTimeLine.resize(AiScene->mNumAnimations);
		_AnimationTrack->PosTimeLine.resize(AiScene->mNumAnimations);

		for (uint32 AnimIdx = 0u; AnimIdx < AiScene->mNumAnimations; ++AnimIdx)
		{
			aiAnimation* _Animation = AiScene->mAnimations[AnimIdx];
			for (uint32 ChannelIdx = 0u; ChannelIdx < _Animation->mNumChannels; ++ChannelIdx)
			{
				const std::string ChannelName = _Animation->mChannels[ChannelIdx]->mNodeName.C_Str();

				AnimTable[AnimIdx].insert(
					{ ChannelName,
					_Animation->mChannels[ChannelIdx] });

				for (uint32 ScaleKeyIdx = 0u;
					ScaleKeyIdx < _Animation->mChannels[ChannelIdx]->mNumScalingKeys;
					++ScaleKeyIdx)
				{
					_AnimationTrack->ScaleTimeLine[AnimIdx][ChannelName]
						[_Animation->mChannels[ChannelIdx]->mScalingKeys[ScaleKeyIdx].mTime] = FromAssimp(_Animation->mChannels[ChannelIdx]->mScalingKeys[ScaleKeyIdx].mValue);
				}

				for (uint32 RotationKeyIdx = 0u;
					RotationKeyIdx < _Animation->mChannels[ChannelIdx]->mNumRotationKeys;
					++RotationKeyIdx)
				{
					Quaternion _Quat =
						FromAssimp(_Animation->mChannels[ChannelIdx]->mRotationKeys[RotationKeyIdx].mValue);
					D3DXQuaternionNormalize(&_Quat, &_Quat);
					_AnimationTrack->QuatTimeLine[AnimIdx][ChannelName]
						[_Animation->mChannels[ChannelIdx]->mRotationKeys[RotationKeyIdx].mTime] = _Quat;
				}

				for (uint32 PosKeyIdx = 0u;
					PosKeyIdx < _Animation->mChannels[ChannelIdx]->mNumPositionKeys;
					++PosKeyIdx)
				{
					_AnimationTrack->PosTimeLine[AnimIdx][ChannelName]
						[_Animation->mChannels[ChannelIdx]->mPositionKeys[PosKeyIdx].mTime] = FromAssimp(_Animation->mChannels[ChannelIdx]->mPositionKeys[PosKeyIdx].mValue);
				}
			}
		}
	}
}
