#include "SkeletonMesh.h"
#include "UtilityGlobal.h"
#include "FMath.hpp"
#include <future>
#include <set>
#include <optional>
#include "imgui.h"


void Engine::SkeletonMesh::Initialize(const std::wstring& ResourceName)&
{
	Super::Initialize(Device);

	auto& ResourceSys = Engine::ResourceSystem::Instance;

	auto ProtoSkeletonMesh = 
		(ResourceSys->GetAny<std::shared_ptr<Engine::SkeletonMesh>>(ResourceName));

	this->operator=(*ProtoSkeletonMesh);

	BoneTable.clear();
	// Bone Info 
	RootBone = &(BoneTable[AiScene->mRootNode->mName.C_Str()] = Bone{});
	RootBone->Name = AiScene->mRootNode->mName.C_Str();
	RootBone->OriginTransform = RootBone->Transform = FromAssimp(AiScene->mRootNode->mTransformation);
	RootBone->Parent = nullptr;
	RootBone->ToRoot = RootBone->OriginTransform;  *FMath::Identity();

	for (uint32 i = 0; i < AiScene->mRootNode->mNumChildren; ++i)
	{
		RootBone->Childrens.push_back(MakeHierarchy(RootBone, AiScene->mRootNode->mChildren[i]));
	}

	for (uint32 MeshIdx = 0u; MeshIdx < AiScene->mNumMeshes; ++MeshIdx)
	{
		MeshContainer[MeshIdx].Finals.clear();
		MeshContainer[MeshIdx].VertexBuffer = nullptr;

		Device->CreateVertexBuffer(MeshContainer[MeshIdx].VtxBufSize,
			D3DUSAGE_DYNAMIC, MeshContainer[MeshIdx].FVF,D3DPOOL_DEFAULT,
			&MeshContainer[MeshIdx].VertexBuffer, nullptr);

		static uint32 VertexBufferCloneID = 0u;
		void* VtxBufPtr{ nullptr };
		MeshContainer[MeshIdx].VertexBuffer->Lock(0, 0, reinterpret_cast<void**>(&VtxBufPtr), NULL);
		std::memcpy(VtxBufPtr, MeshContainer[MeshIdx].VerticiesPtr, MeshContainer[MeshIdx].VtxBufSize);
		MeshContainer[MeshIdx].VertexBuffer->Unlock();

		ResourceSys->Insert<IDirect3DVertexBuffer9>(
			L"VertexBuffer_SkeletonMesh_Clone_" + std::to_wstring(VertexBufferCloneID++), MeshContainer[MeshIdx].VertexBuffer);

		aiMesh* _AiMesh = AiScene->mMeshes[MeshIdx];

		if (_AiMesh->HasBones())
		{
			MeshContainer[MeshIdx].Finals.clear();
			MeshContainer[MeshIdx].Weights.clear();

			MeshContainer[MeshIdx].Finals.resize(MeshContainer[MeshIdx].VtxCount);
		    MeshContainer[MeshIdx].Weights.resize(MeshContainer[MeshIdx].VtxCount);

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
						iter->second.Offset = FromAssimp(CurVtxBone->mOffsetMatrix);
						MeshContainer[MeshIdx].Finals[VtxIdx].push_back(&(iter->second.Final));
					    MeshContainer[MeshIdx].Weights[VtxIdx].push_back(_AiVtxWit.mWeight);
					}
				}
			}
		}
	};
}

void Engine::SkeletonMesh::Event(Object* Owner)&
{
	Super::Event(Owner);
}

void Engine::SkeletonMesh::Render()&
{
	Super::Render();

	for (auto& CurrentRenderMesh : MeshContainer)
	{
		byte* VertexBufferPtr{ nullptr };

		CurrentRenderMesh.VertexBuffer->Lock(0, 0, reinterpret_cast<void**>(&VertexBufferPtr), NULL);

		std::memcpy(VertexBufferPtr, CurrentRenderMesh.VerticiesPtr,
			CurrentRenderMesh.VtxBufSize);

		for (uint64 i = 0; i < CurrentRenderMesh.VtxCount; ++i)
		{
			// 버텍스의 첫번째 메모리 주소가 반드시 Vector3 이라고 가정하고 있음. 유의해야함.
			void* CurrentMemory = (VertexBufferPtr + (i * CurrentRenderMesh.Stride));
			const Vector3*const CurrentLocationPtr = reinterpret_cast<const Vector3*const >(CurrentMemory);
			const uint32 FinalRenderMatrixSize = CurrentRenderMesh.Finals[i].size();
			const auto& CurMeshFinalRow = CurrentRenderMesh.Finals[i];
			const auto& CurMeshWeightsRow = CurrentRenderMesh.Weights[i]; 
			Vector3 AnimLocation{ 0,0,0 };
			for (uint32 j = 0; j < FinalRenderMatrixSize; ++j)
			{
				Vector3 _Location{ 0,0,0 };
				D3DXVec3TransformCoord(&_Location, CurrentLocationPtr, CurMeshFinalRow[j]);
				AnimLocation += (_Location *= CurMeshWeightsRow[j]);
			}
			static constexpr uint32 _float3Size = sizeof(Vector3);
			std::memcpy(CurrentMemory, &AnimLocation, _float3Size);
		}
		CurrentRenderMesh.VertexBuffer->Unlock();
	
	}

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

void Engine::SkeletonMesh::Update(Object* const Owner,const float DeltaTime)&
{
	Super::Update(Owner, DeltaTime);
	CurrentTransitionTime += (DeltaTime * CurrentTransitionAcceleration);

	std::optional<float> IsTransition = std::nullopt;

	if (CurrentTransitionTime < 1.0)
	{
		IsTransition = CurrentTransitionTime;
	}
	else
	{
		T += (DeltaTime * Acceleration);
	}

	bool bAnimation = AnimIdx < AiScene->mNumAnimations;
	aiAnimation* CurAnimation = nullptr;
	std::unordered_map<std::string, aiNodeAnim*>* CurAnimTable = nullptr;
	uint32 TimeLineIdx = 0u;
	float Duration = 0.0f;

	if (bAnimation)
	{
		CurAnimation = AiScene->mAnimations[AnimIdx];
		CurAnimTable = &AnimTable[AnimIdx];
		TimeLineIdx = AnimIdx;
		Duration = AiScene->mAnimations[AnimIdx]->mDuration;
		T=std::fmod(T,CurAnimation->mDuration);
	}

	static const Matrix Identity = FMath::Identity();


	RootBone->BoneMatrixUpdate(Identity,
		T, CurAnimation, CurAnimTable ,
		IsTransition,
		_AnimationTrack->ScaleTimeLine[TimeLineIdx],
		_AnimationTrack->QuatTimeLine[TimeLineIdx],
		_AnimationTrack->PosTimeLine[TimeLineIdx]);
}

Engine::Bone* Engine::SkeletonMesh::MakeHierarchy(
	Bone* BoneParent,const aiNode* const AiNode)
{
	Bone* TargetBone =     &(BoneTable[AiNode->mName.C_Str()] = Bone{});
	TargetBone->Name = AiNode->mName.C_Str();
	TargetBone->OriginTransform = TargetBone->Transform = FromAssimp(AiNode->mTransformation);
	TargetBone->Parent = BoneParent;
	TargetBone->ToRoot = TargetBone->OriginTransform * BoneParent->ToRoot;
	std::cout << TargetBone->Name << std::endl;

	for (uint32 i = 0; i < AiNode->mNumChildren; ++i)
	{
		TargetBone->Childrens.push_back(MakeHierarchy(TargetBone, AiNode->mChildren[i]));
	}

	return TargetBone;
}

void Engine::SkeletonMesh::PlayAnimation(const uint32 AnimIdx ,
	                                     const double Acceleration ,
										// 애니메이션 전이 가속 시간 1 = 전이시간 1
									     const std::optional<double> TransitionAcceleration)&
{
	this->T = 0.0;
	this->AnimIdx = AnimIdx;
	this->Acceleration = Acceleration;

	if (TransitionAcceleration.has_value())
	{
		this->CurrentTransitionAcceleration = TransitionAcceleration.value();
		this->CurrentTransitionTime = 0.0;
	}
	else
	{
		static constexpr double NonTransitionTime = 1.0 + 0.1f;
		this->CurrentTransitionTime = NonTransitionTime;
	}
}


