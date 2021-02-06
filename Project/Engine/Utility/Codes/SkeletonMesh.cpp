#include "SkeletonMesh.h"
#include "UtilityGlobal.h"
#include "FMath.hpp"
#include <future>

#include "imgui.h"



void Engine::SkeletonMesh::Initialize(const std::wstring& ResourceName)&
{
	auto& ResourceSys = Engine::ResourceSystem::Instance;

	auto ProtoSkeletonMesh = 
		(ResourceSys->GetAny<std::shared_ptr<Engine::SkeletonMesh>>(ResourceName));

	this->operator=(*ProtoSkeletonMesh);

	BoneTable.clear();

	/*Bone* Root = &(BoneTable[AiScene->mRootNode->mName.C_Str()] = Bone{});
	Root->Name = AiScene->mRootNode->mName.C_Str();
	Root->OriginTransform = Root->Transform = FromAssimp(AiScene->mRootNode->mTransformation);
	Root->Parent = nullptr;
	Root->ToRoot = Root->OriginTransform; *FMath::Identity();

	for (uint32 i = 0; i < AiScene->mRootNode->mNumChildren; ++i)
	{
		Root->Childrens.push_back(MakeHierarchy(Root, AiScene->mRootNode->mChildren[i]));
	}

	this->RootBone = Root;*/

	if (AiScene->mRootNode->mNumMeshes > 0u)
	{
		// Bone Info 
		Bone* Root = &(BoneTable[AiScene->mRootNode->mName.C_Str()] = Bone{});
		Root->Name = AiScene->mRootNode->mName.C_Str();
		Root->OriginTransform = Root->Transform = FromAssimp(AiScene->mRootNode->mTransformation);
		Root->Parent = nullptr;
		Root->ToRoot = Root->OriginTransform; *FMath::Identity();

		for (uint32 i = 0; i < AiScene->mRootNode->mNumChildren; ++i)
		{
			Root->Childrens.push_back(MakeHierarchy(Root, AiScene->mRootNode->mChildren[i]));
		}

		this->RootBone = Root;
	}
	else
	{
		for (uint32 i = 0; i < AiScene->mRootNode->mNumChildren; ++i)
		{
			if (AiScene->mRootNode->mChildren[i]->mNumMeshes > 0u)
			{
				// Bone Info 
				Bone* Root = &(BoneTable[AiScene->mRootNode->mChildren[i]->mName.C_Str()] = Bone{});
				Root->Name = AiScene->mRootNode->mChildren[i]->mName.C_Str();
				Root->OriginTransform = Root->Transform = FromAssimp(AiScene->mRootNode->mChildren[i]->mTransformation);
				Root->Parent = nullptr;
				Root->ToRoot = Root->OriginTransform; *FMath::Identity();

				for (uint32 j = 0; j < AiScene->mRootNode->mChildren[i]->mNumChildren; ++j)
				{
					Root->Childrens.push_back(MakeHierarchy(Root, AiScene->mRootNode->mChildren[i]->mChildren[j]));
				}

				this->RootBone = Root;
			}
			break;
		}
	}

	for (uint32 MeshIdx = 0u; MeshIdx < AiScene->mNumMeshes; ++MeshIdx)
	{
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
	Device->SetVertexShader(nullptr);
	Device->SetPixelShader(nullptr);

	std::vector<std::future<void>> ParallelSkinnings{};

	for (auto& CurrentRenderMesh : MeshContainer)
	{
		//ParallelSkinnings.push_back(std::async(std::launch::async, [this, CurrentRenderMesh]()
		//	{
		//		byte* VertexBufferPtr{ nullptr };

		//		CurrentRenderMesh.VertexBuffer->Lock(0, 0, reinterpret_cast<void**>(&VertexBufferPtr), NULL);

		//		std::memcpy(VertexBufferPtr, CurrentRenderMesh.VerticiesPtr,
		//			CurrentRenderMesh.VtxBufSize);

		//		for (uint32 i = 0; i < CurrentRenderMesh.VtxCount; ++i)
		//		{
		//			Vector3       AnimLocation{ 0,0,0 };
		//			// 버텍스의 첫번째 메모리 주소가 반드시 Vector3 이라고 가정하고 있음. 유의해야함.
		//			void* CurrentMemory = (VertexBufferPtr + (i * CurrentRenderMesh.Stride));
		//			Vector3* CurrentLocationPtr = reinterpret_cast<Vector3*>(CurrentMemory);
		//			const Vector3 OriginLocation = *CurrentLocationPtr;
		//			for (uint32 j = 0; j < CurrentRenderMesh.Finals[i].size(); ++j)
		//			{
		//				Vector3 _Location{ 0,0,0 };
		//				D3DXVec3TransformCoord(&_Location, &OriginLocation, CurrentRenderMesh.Finals[i][j]);
		//				AnimLocation += (_Location *= CurrentRenderMesh.Weights[i][j]);
		//			}
		//			static constexpr uint32 _float3Size = sizeof(Vector3);
		//			std::memcpy(CurrentMemory, &AnimLocation, _float3Size);
		//		}
		//		CurrentRenderMesh.VertexBuffer->Unlock();
		//	}));
				byte* VertexBufferPtr{ nullptr };

				CurrentRenderMesh.VertexBuffer->Lock(0, 0, reinterpret_cast<void**>(&VertexBufferPtr), NULL);

				std::memcpy(VertexBufferPtr, CurrentRenderMesh.VerticiesPtr,
					CurrentRenderMesh.VtxBufSize);

				for (uint32 i = 0; i < CurrentRenderMesh.VtxCount; ++i)
				{
					Vector3       AnimLocation{ 0,0,0 };
					// 버텍스의 첫번째 메모리 주소가 반드시 Vector3 이라고 가정하고 있음. 유의해야함.
					void* CurrentMemory = (VertexBufferPtr + (i * CurrentRenderMesh.Stride));
					Vector3* CurrentLocationPtr = reinterpret_cast<Vector3*>(CurrentMemory);
					const Vector3 OriginLocation = *CurrentLocationPtr;
					for (uint32 j = 0; j < CurrentRenderMesh.Finals[i].size(); ++j)
					{
						Vector3 _Location{ 0,0,0 };
						D3DXVec3TransformCoord(&_Location, &OriginLocation, CurrentRenderMesh.Finals[i][j]);
						AnimLocation += (_Location *= CurrentRenderMesh.Weights[i][j]);
					}
					static constexpr uint32 _float3Size = sizeof(Vector3);
					std::memcpy(CurrentMemory, &AnimLocation, _float3Size);
				}
				CurrentRenderMesh.VertexBuffer->Unlock();
	}

	for (auto& Joinable : ParallelSkinnings)
	{
		Joinable.get();
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
	T +=  (DeltaTime * Acceleration);

	bool bAnimation = AnimIdx < AiScene->mNumAnimations;
	aiAnimation* CurAnimation = nullptr;
	std::unordered_map<std::string, aiNodeAnim*>* CurAnimTable = nullptr;
	uint32 TimeLineIdx = 0u;

	if (bAnimation)
	{
		CurAnimation = AiScene->mAnimations[AnimIdx];
		CurAnimTable = &AnimTable[AnimIdx];
		TimeLineIdx = AnimIdx;
		if (Engine::Global::bDebugMode)
		{
			ImGui::Text("TickPerSecond : %f", CurAnimation->mTicksPerSecond);
			ImGui::Text("Duration      : %f", CurAnimation->mDuration);
			ImGui::Text("T : %f", T);
		}
		T=std::fmod(T, CurAnimation->mDuration);
	}
	

	RootBone->BoneMatrixUpdate(FMath::Identity(), 
		T, CurAnimation, CurAnimTable ,
		_AnimationTrack->ScaleTimeLine[TimeLineIdx],
		_AnimationTrack->QuatTimeLine[TimeLineIdx],
		_AnimationTrack->PosTimeLine[TimeLineIdx]);
}

Engine::Bone* Engine::SkeletonMesh::MakeHierarchy(Bone* BoneParent,const aiNode* const AiNode)
{
	Bone* TargetBone =     &(BoneTable[AiNode->mName.C_Str()] = Bone{});
	TargetBone->Name = AiNode->mName.C_Str();
	TargetBone->OriginTransform = TargetBone->Transform = FromAssimp(AiNode->mTransformation);
	TargetBone->Parent = BoneParent;
	TargetBone->ToRoot = TargetBone->OriginTransform * BoneParent->ToRoot;

	for (uint32 i = 0; i < AiNode->mNumChildren; ++i)
	{
		TargetBone->Childrens.push_back(MakeHierarchy(TargetBone, AiNode->mChildren[i]));
	}

	return TargetBone;
}

void Engine::SkeletonMesh::PlayAnimation(const uint32 AnimIdx ,
	                                     const double Acceleration)&
{
	this->T = 0.0f;
	this->AnimIdx = AnimIdx;
	this->Acceleration = Acceleration;
}


