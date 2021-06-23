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
#include "ShaderFx.h"
#include "StringHelper.h"
#include "AnimationInformation.h"

namespace Engine
{
	// ���� ���� ������ Ŭ�е鳢�� ����� �������� ������ 
	// ( ������Ÿ�Կ����� ī�� ���� ������ ���������� ���ο� �ν��Ͻ��� �������� �����ϼ���.)
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
			const std::wstring ResourceName ,
			const Engine::RenderInterface::Group RenderGroup) &;
		void  Initialize(const std::wstring& ResourceName)&;
		void  Event(class Object* Owner) & override;
		// �⺻ ���̴� . (������)
		virtual void Render(Engine::Renderer* const _Renderer) & override;
		// ���� �н��� �ʿ��� �븻 �˺����� �Ӽ��� ����Ÿ�ٿ� ������. 
		virtual void RenderDeferredAlbedoNormalVelocityDepthSpecularRim(Engine::Renderer* const _Renderer) & override;
		// ������ �ʿ� �ʿ��� ������ ���������� ���������� ������. 
		virtual void RenderShadowDepth(Engine::Renderer* const _Renderer) & override;
		virtual void RenderReady(Engine::Renderer* const _Renderer) & override;
		virtual void RenderVelocity(Engine::Renderer* const _Renderer) & override;

		void  Update(Object* const Owner, const float DeltaTime)&;
		Engine::Bone*
			MakeHierarchy(Bone* BoneParent, const aiNode* const AiNode);

		Engine::Bone*
			MakeHierarchyClone(Bone* BoneParent, const Bone* const PrototypeBone);

		struct AnimNotify
		{
			std::string Name{}; 
			bool bAnimationEnd { false };
			bool bLoop { false };
			std::map<double, std::function<void(Engine::SkeletonMesh*const)>,std::greater<float>> AnimTimeEventCallMapping{};
			std::set<double> HavebeenCalledEvent{};
		};

		Engine::SkeletonMesh::AnimNotify GetCurrentAnimNotify()const&;
		float GetCurrentNormalizeAnimTime()const&;

		void  PlayAnimation(
			const uint32 AnimIdx, 
			const double Acceleration,
			const double TransitionDuration, const AnimNotify& _AnimNotify)&;
		void  PlayAnimation(const AnimNotify& _AnimNotify)&;

	

		inline std::shared_ptr<Engine::Bone> GetBone(const std::string& BoneName) const&;
		inline std::shared_ptr<Engine::Bone> GetRootBone() const&;
	private:
		void AnimationEnd()&;
		void AnimationNotify()&;

		void InitTextureForVertexTextureFetch()&; 
		void AnimationSave()&;
		void AnimationLoad()&;
	public:
		static const inline Property          TypeProperty = Property::Render;
		uint32 PrevAnimIndex = 0u;
		uint32 AnimIdx{ 0u };
		uint32 AnimEndAfterAnimIdx{ 0u };

		double CurrentAnimMotionTime{ 0.0 };
		double PrevAnimMotionTime{ 0.0 }; 
		double TransitionRemainTime = -1.0;
		double TransitionDuration = 0.0;

		uint32 MaxAnimIdx{ 0u };
		bool bBoneDebug = false; 
	private:
		AnimNotify CurrentNotify{};
	private:
		std::vector<Matrix> RenderBoneMatricies{};
		std::string         RootBoneName{}; 
		std::shared_ptr<std::set<std::string>> BoneNameSet{}; 
		double PrevAnimAcceleration = 1.f;
		double Acceleration = 1.f;

		std::vector<AnimationInformation>                          AnimInfoTable{}; 
		std::unordered_map<std::string,uint32>                     AnimIdxFromName{}; 

		std::shared_ptr<AnimationTrack>                             _AnimationTrack{};
		std::unordered_map<std::string,uint64>						BoneTableIdxFromName{};
		std::vector<std::shared_ptr<Bone>>							BoneTable{}; 
		// VTF ����� ���ؽ� ���̴����� �ִϸ��̼� ��Ű���� ����.
		IDirect3DTexture9* BoneAnimMatrixInfo{nullptr}; 
		IDirect3DTexture9* PrevBoneAnimMatrixInfo{ nullptr };
		
		int32 VTFPitch{ 0 };

		std::filesystem::path FilePath{};
		std::filesystem::path FilePureName{};
	};
}

template<typename VertexType>
void Engine::SkeletonMesh::Load(IDirect3DDevice9* const Device,
	const std::filesystem::path FilePath,
	const std::filesystem::path FileName,
	const std::wstring ResourceName ,
	const Engine::RenderInterface::Group RenderGroup)&
{
	this->_Group = RenderGroup;
	this->ResourceName = ResourceName;
	this->DebugName = ToA(ResourceName);
	this->Device = Device;
	// �� ���� �÷��� , ���� �÷��׸� �ι�, Ȥ�� ȣȯ�� �ȵǴ�
	// �÷��װ� ��ĥ ��� ��Ÿ�� �����̸� ���� �ڵ鸵��
	// �����Ƿ� �ſ� ���� �� ��.
	this->FilePath     = FilePath; 
	this->FilePureName = FileName.stem();

	const aiScene*const  AiScene = Engine::Global::AssimpImporter.ReadFile(
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
		aiProcess_SplitLargeMeshes
	);

	BoneNameSet = std::make_shared<std::set<std::string>>();

	static uint32 SkeletonResourceID = 0u;
	auto& ResourceSys = RefResourceSys();
	MaxAnimIdx = AiScene->mNumAnimations;

	BoneTable.clear();
	BoneTableIdxFromName.clear();
	// Bone Info 
	auto _Bone = std::make_shared<Bone>();
	Bone* RootBone = _Bone.get();
	BoneTable.push_back(_Bone);
	BoneTableIdxFromName.insert({ AiScene->mRootNode->mName.C_Str()  , BoneTable .size()-1u });
	RootBone->Name = AiScene->mRootNode->mName.C_Str();
	RootBone->OriginTransform = RootBone->Transform = FromAssimp(AiScene->mRootNode->mTransformation);
	RootBone->Parent = nullptr;
	RootBone->ToRoot = RootBone->OriginTransform;  *FMath::Identity();
	 
	for (uint32 i = 0; i < AiScene->mRootNode->mNumChildren; ++i)
	{
		RootBone->Childrens.push_back(MakeHierarchy(RootBone, AiScene->mRootNode->mChildren[i]));
	}
	
	LocalVertexLocations = std::make_shared<std::vector<Vector3>>();
	for (uint32 MeshIdx = 0u; MeshIdx < AiScene->mNumMeshes; ++MeshIdx)
	{
		const std::wstring CurrentResourceName =
			ResourceName + L"_" + std::to_wstring(SkeletonResourceID++);

		MeshElement CreateMesh{};
		aiMesh* _AiMesh = AiScene->mMeshes[MeshIdx];
		
		auto Verticies = std::make_shared<std::vector<VertexType>>();
		// ���̺귯�� Ÿ������ ���� ���� �޽� Ÿ������ ��ȯ. 
		for (uint32 VerticesIdx = 0u; VerticesIdx < _AiMesh->mNumVertices; ++VerticesIdx)
		{
			Verticies->push_back(VertexType::MakeFromAssimpMesh(_AiMesh, VerticesIdx));
			LocalVertexLocations->push_back(FromAssimp(_AiMesh->mVertices[VerticesIdx]));
		}
		// ���ҽ� �Ŵ����� �ѱ� ���� ���� ���ҽ� �̸�
		const std::wstring MeshVtxBufResourceName =L"SkeletonMesh_VertexBuffer_" + CurrentResourceName;
		IDirect3DVertexBuffer9* _VertexBuffer{ nullptr };
		// ���̺귯�� �����κ��� �غ� ������ ���� ���� ���� . 
		CreateMesh.VtxBufSize = sizeof(VertexType) * Verticies->size();
		Device->CreateVertexBuffer(CreateMesh.VtxBufSize, D3DUSAGE_DYNAMIC, NULL,D3DPOOL_DEFAULT, &_VertexBuffer, nullptr);

		CreateMesh.VertexBuffer = ResourceSys.Insert<IDirect3DVertexBuffer9>(MeshVtxBufResourceName, _VertexBuffer);
		CreateMesh.PrimitiveCount = CreateMesh.FaceCount = _AiMesh->mNumFaces;

		CreateMesh.VtxCount = Verticies->size();
		CreateMesh.Stride = sizeof(VertexType);
		// �ε��� ����.
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

		// ���׸��� �Ľ� . 
		aiMaterial* AiMaterial = AiScene->mMaterials[_AiMesh->mMaterialIndex];

		const std::wstring
			MatName = ToW(AiScene->mMaterials[_AiMesh->mMaterialIndex]->GetName().C_Str());

		CreateMesh.MaterialInfo.Load(Device,
			FilePath / L"Material",MatName+L".mat",L".tga");
		///// 

		// Vtx Bone ����,
		if (_AiMesh->HasBones())
		{
			uint64 WeightMatrixCount = 0u;
			for (uint32 BoneIdx = 0u; BoneIdx < _AiMesh->mNumBones; ++BoneIdx)
			{
				const aiBone* const CurVtxBone = _AiMesh->mBones[BoneIdx];
				auto iter = BoneTableIdxFromName.find(CurVtxBone->mName.C_Str());
				BoneNameSet->insert(CurVtxBone->mName.C_Str());
				if (iter != std::end(BoneTableIdxFromName))
				{
					const uint64 TargetBoneIdx = iter->second;
					if (TargetBoneIdx < BoneTable.size())
					{
						auto& TargetBone = BoneTable[TargetBoneIdx];
						for (uint32 WeightIdx = 0u; WeightIdx < CurVtxBone->mNumWeights; ++WeightIdx)
						{
							const aiVertexWeight  AiVertexWeight = CurVtxBone->mWeights[WeightIdx];
							const uint32 VtxIdx = AiVertexWeight.mVertexId;
							const float CurrentVertexWeight = AiVertexWeight.mWeight;
							const Matrix OffsetMatrix = FromAssimp(CurVtxBone->mOffsetMatrix);
							TargetBone->Offset = OffsetMatrix;
							
							// ���� ���ؽ����� Vector4 �� float ������ ����ִ�(0��) ������ ã�Ƴ�.
							static auto FindVtxCurrentBoneSlot = [](
								const Vector4& TargetWeights)->uint8
							{
								uint8 SlotIdx = 0u;
								while (((SlotIdx < 4u) && (TargetWeights[SlotIdx] != 0.0f)))
								{
									++SlotIdx;
								}
								return SlotIdx;
							};

							const uint8 CurSlot = 
								FindVtxCurrentBoneSlot((*Verticies)[VtxIdx].Weights);
							// ������ ��Ű�� �� �ε����� ����ġ ���� . 
							(*Verticies)[VtxIdx].Weights[CurSlot] = CurrentVertexWeight;
							(*Verticies)[VtxIdx].BoneIds[CurSlot] = static_cast<int16>(TargetBoneIdx);

							++WeightMatrixCount;
						}
					}
				}
			}
		}


		VertexType* VertexBufferPtr{ nullptr };
		CreateMesh.VertexBuffer->Lock(0, 0, reinterpret_cast<void**>(&VertexBufferPtr), NULL);
		std::memcpy(VertexBufferPtr, Verticies->data(), CreateMesh.VtxBufSize);
		CreateMesh.VertexBuffer->Unlock();

		MeshContainer.push_back(CreateMesh);
	}


	const std::wstring VtxTypeNameW = ToW(typeid(VertexType).name());
	VtxDecl = ResourceSys.Get<IDirect3DVertexDeclaration9>(VtxTypeNameW);
	if (!VtxDecl)
	{
		VtxDecl= ResourceSys.Insert<IDirect3DVertexDeclaration9>(VtxTypeNameW, VertexType::GetVertexDecl(Device));
	}


	// �ִϸ��̼� ������ �Ľ�.
	if (AiScene->HasAnimations())
	{
		AnimInfoTable.resize(AiScene->mNumAnimations); 
		AnimIdxFromName.reserve(AiScene->mNumAnimations);

		_AnimationTrack = std::make_shared<AnimationTrack>();
		_AnimationTrack->ScaleTimeLine.resize(AiScene->mNumAnimations);
		_AnimationTrack->QuatTimeLine.resize(AiScene->mNumAnimations);
		_AnimationTrack->PosTimeLine.resize(AiScene->mNumAnimations);

		for (uint32 AnimIdx = 0u; AnimIdx < AiScene->mNumAnimations; ++AnimIdx)
		{
			aiAnimation* _Animation = AiScene->mAnimations[AnimIdx];
			AnimInfoTable[AnimIdx].Acceleration = 1.0 * _Animation->mTicksPerSecond;
			AnimInfoTable[AnimIdx].TickPerSecond = _Animation->mTicksPerSecond;
			AnimInfoTable[AnimIdx].Name = (_Animation->mName.C_Str());
			AnimInfoTable[AnimIdx].Duration = _Animation->mDuration;

			AnimIdxFromName.insert({ _Animation ->mName.C_Str() , AnimIdx});
			for (uint32 ChannelIdx = 0u; ChannelIdx < _Animation->mNumChannels; ++ChannelIdx)
			{
				const std::string ChannelName = _Animation->mChannels[ChannelIdx]->mNodeName.C_Str();


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

	AnimationLoad();
}




inline  std::shared_ptr<Engine::Bone> Engine::SkeletonMesh::GetBone(const std::string& BoneName) const&
{
	if (auto iter = BoneTableIdxFromName.find(BoneName);
		iter != std::end(BoneTableIdxFromName))
	{
		return BoneTable[iter->second];
	}

	return {};
}

inline  std::shared_ptr<Engine::Bone> Engine::SkeletonMesh::GetRootBone() const&
{
	return GetBone(RootBoneName);
}

