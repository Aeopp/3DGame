#include "SkeletonMesh.h"
#include "UtilityGlobal.h"
#include "FMath.hpp"
#include <future>
#include <set>
#include <optional>
#include "imgui.h"
#include "Renderer.h"


void Engine::SkeletonMesh::Initialize(const std::wstring& ResourceName)&
{
	Super::Initialize(Device);

	auto& ResourceSys = Engine::ResourceSystem::Instance;

	auto ProtoSkeletonMesh = 
		(ResourceSys->GetAny<std::shared_ptr<Engine::SkeletonMesh>>(ResourceName));

	this->operator=(*ProtoSkeletonMesh);
	BoneTableIdxFromName.clear();
	BoneTable.clear();
	// Bone Info 
	auto _Bone = std::make_shared<Bone>(); 
	BoneTable.push_back(_Bone);
	const uint64 CurBoneIdx = BoneTable.size() - 1u; 
	RootBone = _Bone.get();
	BoneTableIdxFromName.insert({AiScene->mRootNode->mName.C_Str() , CurBoneIdx });
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
			D3DUSAGE_DYNAMIC, NULL,D3DPOOL_DEFAULT,
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
				
				if (auto iter = BoneTableIdxFromName.find(CurVtxBone->mName.C_Str());
					iter != std::end(BoneTableIdxFromName))
				{
					const uint64 TargetBoneIdx = iter->second;
					if (TargetBoneIdx < BoneTable.size())
					{
						auto& TargetBone = BoneTable[TargetBoneIdx];
						
						for (uint32 WeightIdx = 0u; WeightIdx < CurVtxBone->mNumWeights; ++WeightIdx)
						{
								const aiVertexWeight  _AiVtxWit = CurVtxBone->mWeights[WeightIdx];
								const uint32 VtxIdx = _AiVtxWit.mVertexId;
								TargetBone->Offset = FromAssimp(CurVtxBone->mOffsetMatrix);

								MeshContainer[MeshIdx].Finals[VtxIdx].push_back(
									&TargetBone->Final);

								MeshContainer[MeshIdx].Weights[VtxIdx].push_back(_AiVtxWit.mWeight);
						}
					}
				}
			}
		}
	};

	_ShaderFx.Initialize(L"SkeletonSkinningDefaultFx");

	InitTextureForVertexTextureFetch();
}

void Engine::SkeletonMesh::Event(Object* Owner)&
{
	Super::Event(Owner);
}

void Engine::SkeletonMesh::Render(const Matrix& World,
	const Matrix& View,
	const Matrix& Projection,
	const Vector4& CameraLocation4D)&
{
	Super::Render(World ,View ,Projection, CameraLocation4D );

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
			/*const uint32 FinalRenderMatrixSize = CurrentRenderMesh.Finals[i].size();
			const auto& CurMeshFinalRow = CurrentRenderMesh.Finals[i];
			const auto& CurMeshWeightsRow = CurrentRenderMesh.Weights[i]; */
			Vector3 AnimLocation{ 0,0,0 };
			for (uint32 j = 0; j < 4; ++j)
			{
				Vector3 _Location{ 0,0,0 };

				const Matrix& RenderMatrix = BoneTable[reinterpret_cast<Vertex::LocationTangentUV2DSkinning*>(VertexBufferPtr)
					[i].BoneIds[j]  ]->Final ;
				
				const float _Weight = reinterpret_cast<Vertex::LocationTangentUV2DSkinning*>(VertexBufferPtr)
					[i].Weights[j];

					D3DXVec3TransformCoord(&_Location, CurrentLocationPtr, 
					&RenderMatrix);

					AnimLocation += (_Location *= _Weight);					
			}
			static constexpr uint32 _float3Size = sizeof(Vector3);
			std::memcpy(CurrentMemory, &AnimLocation, _float3Size);
		}
		CurrentRenderMesh.VertexBuffer->Unlock();

		D3DLOCKED_RECT LockRect{ 0u, };
		if (FAILED(BoneAnimMatrixInfo->LockRect(0u, &LockRect, nullptr, 0)))
		{
			assert(NULL);
		}

		//std::memcpy(LockRect.pBits, );
		BoneAnimMatrixInfo->UnlockRect(0u);

	}

	auto Fx = _ShaderFx.GetHandle();
	auto& Renderer = *Engine::Renderer::Instance;

	uint32 PassNum = 0u;
	Fx->Begin(&PassNum, 0);

	Fx->SetMatrix("World", &World);
	Fx->SetMatrix("View", &View);
	Fx->SetMatrix("Projection", &Projection);
	Fx->SetVector("LightDirection", &Renderer.LightDirection);
	Fx->SetVector("LightColor", &Renderer.LightColor);
	Fx->SetVector("CameraLocation", &CameraLocation4D);
	for (auto& CurrentRenderMesh : MeshContainer)
	{
		Fx->SetVector("RimAmtColor", &CurrentRenderMesh.MaterialInfo.RimAmtColor);
		Fx->SetFloat("RimOuterWidth", CurrentRenderMesh.MaterialInfo.RimOuterWidth);
		Fx->SetFloat("RimInnerWidth", CurrentRenderMesh.MaterialInfo.RimInnerWidth);
		Fx->SetVector("AmbientColor", &CurrentRenderMesh.MaterialInfo.AmbientColor);
		Fx->SetFloat("Power", CurrentRenderMesh.MaterialInfo.Power);
		Fx->SetFloat("SpecularIntencity", CurrentRenderMesh.MaterialInfo.SpecularIntencity);
		Fx->SetFloat("Contract", CurrentRenderMesh.MaterialInfo.Contract);
		Fx->SetFloat("DetailDiffuseIntensity", CurrentRenderMesh.MaterialInfo.DetailDiffuseIntensity);
		Fx->SetFloat("DetailNormalIntensity", CurrentRenderMesh.MaterialInfo.DetailNormalIntensity);
		Fx->SetFloat("CavityCoefficient", CurrentRenderMesh.MaterialInfo.CavityCoefficient);

		Fx->SetFloat("DetailScale", CurrentRenderMesh.MaterialInfo.DetailScale);
		Device->SetVertexDeclaration(VtxDecl);
		Device->SetStreamSource(0, CurrentRenderMesh.VertexBuffer, 0, CurrentRenderMesh.Stride);
		Device->SetIndices(CurrentRenderMesh.IndexBuffer);

		Fx->SetTexture("DiffuseMap", CurrentRenderMesh.MaterialInfo.GetTexture(L"Diffuse"));
		Fx->SetTexture("NormalMap", CurrentRenderMesh.MaterialInfo.GetTexture(L"Normal"));
		Fx->SetTexture("CavityMap", CurrentRenderMesh.MaterialInfo.GetTexture(L"Cavity"));
		Fx->SetTexture("EmissiveMap", CurrentRenderMesh.MaterialInfo.GetTexture(L"Emissive"));
		Fx->SetTexture("DetailDiffuseMap", CurrentRenderMesh.MaterialInfo.GetTexture(L"DetailDiffuse"));
		Fx->SetTexture("DetailNormalMap", CurrentRenderMesh.MaterialInfo.GetTexture(L"DetailNormal"));

		Fx->CommitChanges();

		for (uint32 i = 0; i < PassNum; ++i)
		{
			Fx->BeginPass(i);

			Device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0u, 0u, 
				CurrentRenderMesh.VtxCount, 0u, CurrentRenderMesh.PrimitiveCount);

			Fx->EndPass();
		}
	}
	Fx->End();
}

void Engine::SkeletonMesh::Update(Object* const Owner,const float DeltaTime)&
{
	Super::Update(Owner, DeltaTime);

	static const Matrix Identity = FMath::Identity();

	CurrentAnimMotionTime  += (DeltaTime * Acceleration);
	PrevAnimMotionTime     += (DeltaTime * PrevAnimAcceleration);
	TransitionRemainTime   -= DeltaTime;

	if (TransitionRemainTime > 0.0)
	{
		const aiAnimation* const PrevAnimation = AiScene->mAnimations[PrevAnimIndex];
		std::optional<Bone::AnimationBlendInfo> IsAnimationBlend = std::nullopt;

		if (PrevAnimMotionTime > PrevAnimation->mDuration)
		{
			PrevAnimMotionTime = PrevAnimation->mDuration;
		}

		if (AnimIdx < AiScene->mNumAnimations)
		{
			const double PrevAnimationWeight = TransitionRemainTime / TransitionDuration;
			std::unordered_map<std::string, aiNodeAnim*>* PrevAnimTable = &AnimTable[PrevAnimIndex];

			IsAnimationBlend.emplace(Bone::AnimationBlendInfo
				{
					PrevAnimationWeight , PrevAnimMotionTime ,
					PrevAnimation , PrevAnimTable  ,
						_AnimationTrack->ScaleTimeLine[PrevAnimIndex] ,
						_AnimationTrack->QuatTimeLine[PrevAnimIndex],
						_AnimationTrack->PosTimeLine[PrevAnimIndex]
				});

			aiAnimation* CurAnimation = AiScene->mAnimations[AnimIdx];
			std::unordered_map<std::string, aiNodeAnim*>* CurAnimTable = &AnimTable[AnimIdx];

			if (CurrentAnimMotionTime > CurAnimation->mDuration)
			{
				CurrentAnimMotionTime = CurAnimation->mDuration;
			}

			RootBone->BoneMatrixUpdate(Identity,
				CurrentAnimMotionTime, CurAnimation, CurAnimTable,
				_AnimationTrack->ScaleTimeLine[AnimIdx],
				_AnimationTrack->QuatTimeLine[AnimIdx],
				_AnimationTrack->PosTimeLine[AnimIdx], IsAnimationBlend);
		}
	}
	else
	{
		if (AnimIdx < AiScene->mNumAnimations)
		{
			aiAnimation*  CurAnimation = AiScene->mAnimations[AnimIdx];
			std::unordered_map<std::string, aiNodeAnim*>*  CurAnimTable = &AnimTable[AnimIdx];

			if (CurrentAnimMotionTime > CurAnimation->mDuration)
			{
				CurrentAnimMotionTime = CurAnimation->mDuration; 
			}

			RootBone->BoneMatrixUpdate(Identity,
				CurrentAnimMotionTime, CurAnimation, CurAnimTable,
				_AnimationTrack->ScaleTimeLine[AnimIdx]    ,
				_AnimationTrack->QuatTimeLine[AnimIdx]   ,
				_AnimationTrack->PosTimeLine[AnimIdx]   ,     std::nullopt);
		}
	}
}

Engine::Bone*
Engine::SkeletonMesh::MakeHierarchy(
	Bone* BoneParent,const aiNode* const AiNode)
{
	auto TargetBone = std::make_shared<Bone>(); 
	BoneTable.push_back(TargetBone);
	const uint64 CurBoneIdx = BoneTable.size() - 1;

	BoneTableIdxFromName.insert({ AiNode->mName.C_Str()  ,CurBoneIdx });
	TargetBone->Name = AiNode->mName.C_Str();
	TargetBone->OriginTransform = TargetBone->Transform = FromAssimp(AiNode->mTransformation);
	TargetBone->Parent = TargetBone.get(); 
	TargetBone->ToRoot = TargetBone->OriginTransform * BoneParent->ToRoot;

	for (uint32 i = 0; i < AiNode->mNumChildren; ++i)
	{
		TargetBone->Childrens.push_back(MakeHierarchy(TargetBone.get(), AiNode->mChildren[i]));
	}

	return TargetBone.get();
}

void Engine::SkeletonMesh::PlayAnimation(const uint32 AnimIdx ,
	                                     const double Acceleration ,
										 const double TransitionDuration)&
{
	PrevAnimMotionTime = CurrentAnimMotionTime;
	CurrentAnimMotionTime = 0.0;
	PrevAnimIndex = this->AnimIdx;

	this->AnimIdx = AnimIdx;
	this->TransitionDuration = TransitionRemainTime = TransitionDuration;
	this->PrevAnimAcceleration = this->Acceleration;
	this->Acceleration = Acceleration;
}

void Engine::SkeletonMesh::InitTextureForVertexTextureFetch()&
{
	/// <summary>
	/// 텍스쳐 채널 4개 채널당 부동소수 1개이니 텍셀당 4차원 벡터를 저장할수 있으며
	/// 4x4 행렬로 본 매트릭스를 넘기므로 텍셀 4개당 행렬 하나의 정보이다.
	/// 때문에 단일 서브 메쉬가 참조하는 본의 최대 개수의 상한선 만큼 (혹은 조금 어유를 둘 정도로 2의 배수로 텍스쳐의 사이즈를 맞춰준다.
	/// </summary>
	
	//const uint64 ExactTexelLength = 
	//	static_cast<uint64>(std::ceill(sqrt(NumMaxRefBone * 16u)));

	//uint64 PowerOf2 = 1u;
	//uint64 RealTexelLength = static_cast<uint64> (std::pow(2, PowerOf2));
	//// 2의 승수가 최대 6이므로 지원하는 최대 본은 1024 임을 나타냄.
	//for (PowerOf2 = 1u; PowerOf2 < 7u; ++PowerOf2)
	//{
	//	RealTexelLength = static_cast<uint64>(std::pow(2u, PowerOf2)); 

	//	if (ExactTexelLength <= RealTexelLength)
	//		break;;
	//};

	Device->CreateTexture
	(64u, 64u,1,0, D3DFMT_A32B32G32R32F,D3DPOOL_MANAGED,
		&BoneAnimMatrixInfo, nullptr);

	static uint64 BoneAnimMatrixInfoTextureResourceID = 0u;

	ResourceSystem::Instance->Insert<IDirect3DTexture9>
		(
			L"VTF_" + std::to_wstring(BoneAnimMatrixInfoTextureResourceID++), BoneAnimMatrixInfo);
}


