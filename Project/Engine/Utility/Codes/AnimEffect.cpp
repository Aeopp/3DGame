#include "AnimEffect.h"
#include "ResourceSystem.h"
#include "Renderer.h"
#include "imgui.h"

void Engine::AnimEffect::Load(
	IDirect3DDevice9* const Device, 
	const std::filesystem::path FileFullPath)&
{
	const std::filesystem::path ParentPath = FileFullPath.parent_path();

	this->Device = Device;
	using VertexType = Vertex::AnimMeshEffect;

	const aiScene* const AiScene = Engine::Global::AssimpImporter.ReadFile (
		FileFullPath.string(),
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

	auto& ResourceSys = ResourceSystem::Instance;


	BoneTable.clear();
	BoneTableIdxFromName.clear();
	// Bone Info 
	auto _Bone = std::make_shared<Bone>();
	Bone* RootBone = _Bone.get();
	BoneTable.push_back(_Bone);
	BoneTableIdxFromName.insert({ AiScene->mRootNode->mName.C_Str()  , BoneTable.size() - 1u });
	RootBone->Name = AiScene->mRootNode->mName.C_Str();
	RootBone->OriginTransform = RootBone->Transform = FromAssimp(AiScene->mRootNode->mTransformation);
	RootBone->Parent = nullptr;
	RootBone->ToRoot = RootBone->OriginTransform;  *FMath::Identity();

	for (uint32 i = 0; i < AiScene->mRootNode->mNumChildren; ++i)
	{
		RootBone->Childrens.push_back(MakeHierarchy(RootBone, AiScene->mRootNode->mChildren[i]));
	}
	LocalVertexLocations = std::make_shared<std::vector<Vector3>>();

	static const std::wstring EffectMeshResourceName = L"EffectMesh_";
	static uint32 UniqueResourceID = 0u;

	for (uint32 MeshIdx = 0u; MeshIdx < AiScene->mNumMeshes; ++MeshIdx)
	{
		AnimEffectMesh CreateMesh{};

		aiMesh* _AiMesh = AiScene->mMeshes[MeshIdx];
		std::vector<VertexType> Verticies;

		for (uint32 VerticesIdx = 0u; VerticesIdx < _AiMesh->mNumVertices; ++VerticesIdx)
		{
			Verticies.push_back(VertexType::MakeFromAssimpMesh(_AiMesh, VerticesIdx));
			LocalVertexLocations->push_back(FromAssimp(_AiMesh->mVertices[VerticesIdx]));
		}
		

		IDirect3DVertexBuffer9* _VtxBuf{ nullptr };
		CreateMesh.VtxBufSize = sizeof(VertexType) * Verticies.size();
		Device->CreateVertexBuffer(CreateMesh.VtxBufSize, D3DUSAGE_DYNAMIC, NULL,
			D3DPOOL_DEFAULT, &_VtxBuf, nullptr);
		CreateMesh.VertexBuffer = ResourceSys->Insert<IDirect3DVertexBuffer9>
			(EffectMeshResourceName + L"_VertexBuffer_"
			+ std::to_wstring(UniqueResourceID++), _VtxBuf);
		CreateMesh.PrimitiveCount = CreateMesh.FaceCount = _AiMesh->mNumFaces;
		CreateMesh.VtxCount = Verticies.size();
		CreateMesh.Stride = sizeof(VertexType);

		std::vector<uint32> Indicies{};
		for (uint32 FaceIdx = 0u; FaceIdx < _AiMesh->mNumFaces; ++FaceIdx)
		{
			const aiFace CurrentFace = _AiMesh->mFaces[FaceIdx];
			for (uint32 Idx = 0u; Idx < CurrentFace.mNumIndices; ++Idx)
			{
				Indicies.push_back(CurrentFace.mIndices[Idx]);
			}
		}
		IDirect3DIndexBuffer9* _IndexBuffer{ nullptr };
		const uint32 IdxBufSize = sizeof(uint32) * Indicies.size();
		Device->CreateIndexBuffer(IdxBufSize, D3DUSAGE_WRITEONLY, D3DFMT_INDEX32,
			D3DPOOL_MANAGED, &_IndexBuffer, nullptr);
		CreateMesh.IndexBuffer = ResourceSys->Insert<IDirect3DIndexBuffer9>
			(EffectMeshResourceName + L"_IndexBuffer_"
				+ std::to_wstring(UniqueResourceID++),
			_IndexBuffer);
		uint32* IndexBufferPtr{ nullptr };
		CreateMesh.IndexBuffer->Lock(0, 0, reinterpret_cast<void**>(&IndexBufferPtr), NULL);
		std::memcpy(IndexBufferPtr, Indicies.data(), IdxBufSize);
		CreateMesh.IndexBuffer->Unlock();

		aiMaterial* AiMaterial = AiScene->mMaterials[_AiMesh->mMaterialIndex];

		const std::wstring
			MatName = ToW(AiMaterial->GetName().C_Str());

		if (AiScene->HasMaterials())
		{
			if (AiMaterial->GetTextureCount(aiTextureType::aiTextureType_DIFFUSE) > 0)
			{
				aiString TexPath{};
				if (AiMaterial->GetTexture
				(aiTextureType::aiTextureType_DIFFUSE, 0, &TexPath, NULL, NULL,
					NULL, NULL, NULL) == AI_SUCCESS)
				{
					const std::wstring LoadFilePathW = ToW(TexPath.C_Str());
					using ResourceType = std::remove_pointer_t<decltype(CreateMesh.Diffuse)>;
					CreateMesh.Diffuse = ResourceSys->Get<ResourceType>(LoadFilePathW);
					if (nullptr == CreateMesh.Diffuse)
					{
						D3DXCreateTextureFromFile(Device,
							(ParentPath / LoadFilePathW).c_str(), &CreateMesh.Diffuse);

						if (CreateMesh.Diffuse)
						{
							ResourceSys->Insert<ResourceType>(LoadFilePathW,CreateMesh.Diffuse);
						}
					}
				}
			}
		}


		if (_AiMesh->HasBones())
		{
			uint64 WeightMatrixCount = 0u;
			for (uint32 BoneIdx = 0u; BoneIdx < _AiMesh->mNumBones; ++BoneIdx)
			{
				const aiBone* const CurVtxBone = _AiMesh->mBones[BoneIdx];
				auto iter = BoneTableIdxFromName.find(CurVtxBone->mName.C_Str());
				if (iter != std::end(BoneTableIdxFromName))
				{
					const uint64 TargetBoneIdx = iter->second;
					if (TargetBoneIdx < BoneTable.size())
					{
						auto& TargetBone = BoneTable[TargetBoneIdx];
						for (uint32 WeightIdx = 0u; WeightIdx < CurVtxBone->mNumWeights; ++WeightIdx)
						{
							const aiVertexWeight  _AiVtxWit = CurVtxBone->mWeights[WeightIdx];
							const uint32 VtxIdx = _AiVtxWit.mVertexId;
							const float _Wit = _AiVtxWit.mWeight;
							const Matrix OffsetMatrix = FromAssimp(CurVtxBone->mOffsetMatrix);
							TargetBone->Offset = OffsetMatrix;

							// 현재 버텍스에서 Vector4 의 float 슬롯중 비어있는 슬롯을 찾아냄.
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
								FindVtxCurrentBoneSlot((Verticies)[VtxIdx].Weights);

							(Verticies)[VtxIdx].Weights[CurSlot] = _Wit;
							(Verticies)[VtxIdx].BoneIds[CurSlot] = static_cast<int16>(TargetBoneIdx);

							++WeightMatrixCount;
						}
					}
				}
			}
		}


		VertexType* VertexBufferPtr{ nullptr };
		CreateMesh.VertexBuffer->Lock(0, 0, reinterpret_cast<void**>(&VertexBufferPtr), NULL);
		std::memcpy(VertexBufferPtr, Verticies.data(), CreateMesh.VtxBufSize);
		CreateMesh.VertexBuffer->Unlock();

		MeshContainer.push_back(CreateMesh);
	}


	const std::wstring VtxTypeNameW = ToW(typeid(VertexType).name());
	VtxDecl = ResourceSys->Get<IDirect3DVertexDeclaration9>(VtxTypeNameW);
	if (!VtxDecl)
	{
		VtxDecl = ResourceSys->Insert<IDirect3DVertexDeclaration9>(VtxTypeNameW, VertexType::GetVertexDecl(Device));
	}





	// 애니메이션 데이터 파싱.
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

			AnimIdxFromName.insert({ _Animation->mName.C_Str() , AnimIdx });
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
}

void Engine::AnimEffect::Initialize()&
{
	auto ProtoRootBone = this->BoneTable.front();
	
	BoneTable.clear();
	BoneTableIdxFromName.clear();
	// Bone Info 
	auto _Bone = std::make_shared<Bone>();
	BoneTable.push_back(_Bone);
	const uint64 CurBoneIdx = BoneTable.size() - 1u;
	Bone* RootBone = _Bone.get();
	BoneTableIdxFromName.insert({ ProtoRootBone->Name, CurBoneIdx });
	RootBone->Name = ProtoRootBone->Name;
	RootBone->OriginTransform = RootBone->Transform = ProtoRootBone->Transform;
	RootBone->Parent = nullptr;
	RootBone->ToRoot = RootBone->OriginTransform;  *FMath::Identity();

	for (const auto& ProtoBoneChildren : ProtoRootBone->Childrens)
	{
		RootBone->Childrens.push_back(MakeHierarchyClone(RootBone, ProtoBoneChildren));
	}

	InitTextureForVertexTextureFetch();

	AnimEffectFx.Initialize(L"AnimEffectFx");
	RenderBoneMatricies.resize(BoneTable.size());
}

void Engine::AnimEffect::RenderReady(Engine::Renderer* const _Renderer)&
{
	if (!bRender)return;

	std::transform(std::begin(BoneTable), std::end(BoneTable),
		std::begin(RenderBoneMatricies), []
		(const std::shared_ptr<Bone>& CopyBoneFinalMatrix)
		{
			return CopyBoneFinalMatrix->Final;
		});

	D3DLOCKED_RECT LockRect{ 0u, };
	if (FAILED(BoneAnimMatrixInfo->LockRect(0u, &LockRect, nullptr, 0)))
	{

	}

	std::memcpy(LockRect.pBits, RenderBoneMatricies.data(), RenderBoneMatricies.size() * sizeof(Matrix));
	BoneAnimMatrixInfo->UnlockRect(0u);

	World = FMath::WorldMatrix(Scale, Rotation, Location);
}

void Engine::AnimEffect::Render(Engine::Renderer* const _Renderer)&
{
	if (!bRender)return;

	//matrix World;
	//matrix View;
	//matrix Projection;
	//texture DiffuseMap;

	auto Fx = AnimEffectFx.GetHandle();
	Fx->SetTexture("VTF", BoneAnimMatrixInfo);
	Fx->SetInt("VTFPitch", VTFPitch);	
	if (nullptr == Device)
		return;


	// 테스팅.. 
	//static Matrix World;
	//static Vector3 Scale{ 1,1,1 };
	//static Vector3 Rotation{ 0,0,0 };
	//static Vector3 Location{ 0,0,0 };
	//{
	//	if (Engine::Global::bDebugMode)
	//	{
	//		ImGui::SliderFloat3("TestScale", Scale, 0.1f, 10.f);
	//		ImGui::SliderFloat3("TestRotation", Rotation, -FMath::PI, FMath::PI);
	//		ImGui::SliderFloat3("TestLocation", Location, -1000.f, +1000.f);
	//	}
	//	World = FMath::WorldMatrix(Scale, Rotation, Location);
	//}
	//.

	const auto& RenderInfo = _Renderer->GetCurrentRenderInformation();
	const auto& _RefDeferredPass = _Renderer->RefDeferredPass();
	Fx->SetMatrix("World", &World);
	Fx->SetMatrix("View", &RenderInfo.View);
	Fx->SetMatrix("Projection", &RenderInfo.Projection);
	Fx->SetFloat("AlphaFactor", _CurAnimEffectInfo.AlphaFactor);
	Fx->SetFloat("Brightness", _CurAnimEffectInfo.Brightness);
	Device->SetVertexDeclaration(VtxDecl);
	uint32 PassNum = 0u;
	Fx->Begin(&PassNum, 0);
	for (auto& CurrentRenderMesh : MeshContainer)
	{
		Device->SetStreamSource(0, CurrentRenderMesh.VertexBuffer, 0, CurrentRenderMesh.Stride);
		Device->SetIndices(CurrentRenderMesh.IndexBuffer);
		Fx->SetTexture("DiffuseMap", CurrentRenderMesh.Diffuse);
		Fx->SetTexture("PatternMap", _CurAnimEffectInfo.PatternMap);
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

	if (Engine::Global::bDebugMode)
	{
		Device->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
		Device->SetRenderState(D3DRS_ZENABLE, FALSE);
		auto& ResourceSys = ResourceSystem::Instance;
		ID3DXMesh* const _DebugMesh = ResourceSys->Get<ID3DXMesh>(L"SphereMesh");
		for (auto& _Bone : BoneTable)
		{
			_Bone->DebugRender(World, Device, _DebugMesh);
		}
		Device->SetRenderState(D3DRS_ZENABLE, TRUE);
		Device->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
	}
}

void Engine::AnimEffect::Update(const float DeltaTime)&
{
	static const Matrix Identity = FMath::Identity();

	CurrentAnimMotionTime += (DeltaTime * Acceleration);
	PrevAnimMotionTime += (DeltaTime * PrevAnimAcceleration);
	TransitionRemainTime -= DeltaTime;

	AnimationNotify();

	if (TransitionRemainTime > 0.0)
	{
		std::optional<Bone::AnimationBlendInfo> IsAnimationBlend = std::nullopt;

		if (PrevAnimMotionTime > AnimInfoTable[PrevAnimIndex].Duration)
		{
			PrevAnimMotionTime = AnimInfoTable[PrevAnimIndex].Duration;
		}

		if (AnimIdx < AnimInfoTable.size())
		{
			const double PrevAnimationWeight = TransitionRemainTime / TransitionDuration;

			IsAnimationBlend.emplace(Bone::AnimationBlendInfo
				{
					PrevAnimIndex,
					PrevAnimationWeight , PrevAnimMotionTime ,
					_AnimationTrack->ScaleTimeLine[PrevAnimIndex] ,
					_AnimationTrack->QuatTimeLine[PrevAnimIndex],
					_AnimationTrack->PosTimeLine[PrevAnimIndex]
				});

			Bone* RootBone = BoneTable.front().get();
			RootBone->BoneMatrixUpdate(Identity,
				CurrentAnimMotionTime,
				_AnimationTrack->ScaleTimeLine[AnimIdx],
				_AnimationTrack->QuatTimeLine[AnimIdx],
				_AnimationTrack->PosTimeLine[AnimIdx], IsAnimationBlend);

			if (CurrentAnimMotionTime > AnimInfoTable[AnimIdx].Duration)
			{
				AnimationEnd();
			}
		}
	}
	else
	{
		if (AnimIdx < AnimInfoTable.size())
		{
			if (CurrentAnimMotionTime > AnimInfoTable[AnimIdx].Duration)
			{
				AnimationEnd();
			}
			Bone* RootBone = BoneTable.front().get();

			RootBone->BoneMatrixUpdate(Identity,
				CurrentAnimMotionTime,
				_AnimationTrack->ScaleTimeLine[AnimIdx],
				_AnimationTrack->QuatTimeLine[AnimIdx],
				_AnimationTrack->PosTimeLine[AnimIdx], std::nullopt);
		}
	}

	if (_AnimEffectUpdateCall)
		_AnimEffectUpdateCall(_CurAnimEffectInfo ,DeltaTime);

	/*Time += DeltaTime * TimeAcceleration;
	AlphaFactor = std::fabsf(std::sinf(Time));*/
}

void Engine::AnimEffect::PlayAnimation(
	const uint32 AnimIdx,
	const double Acceleration, 
	const double TransitionDuration, 
	const AnimNotify& _AnimNotify)&
{
	PrevAnimMotionTime = CurrentAnimMotionTime;
	CurrentAnimMotionTime = 0.0;
	PrevAnimIndex = this->AnimIdx;

	this->AnimIdx = AnimIdx;
	this->TransitionDuration = TransitionRemainTime = TransitionDuration;
	this->PrevAnimAcceleration = this->Acceleration;
	this->Acceleration = Acceleration;

	CurrentNotify = _AnimNotify;
}

Engine::AnimEffect::AnimNotify Engine::AnimEffect::GetCurrentAnimNotify() const&
{
	return CurrentNotify;
}

float Engine::AnimEffect::GetCurrentNormalizeAnimTime() const&
{
	return CurrentAnimMotionTime / AnimInfoTable[AnimIdx].Duration;
}

void Engine::AnimEffect::AnimationEnd()&
{
	if (CurrentNotify.bLoop)
	{
		PlayAnimation(AnimIdx, Acceleration, TransitionDuration, CurrentNotify);
	}
	else
	{
		CurrentAnimMotionTime = AnimInfoTable[AnimIdx].Duration;
		CurrentNotify.bAnimationEnd = true;
	}
}

void Engine::AnimEffect::AnimationNotify()&
{
	const float AnimDurationNormalize = CurrentAnimMotionTime / AnimInfoTable[AnimIdx].Duration;
	auto EventIter = CurrentNotify.AnimTimeEventCallMapping.lower_bound(AnimDurationNormalize);

	bool bTiming = (std::end(CurrentNotify.AnimTimeEventCallMapping) != EventIter) &&
		AnimDurationNormalize >= EventIter->first;

	if (bTiming)
	{
		// 과거에 호출 한 적이 없으며 함수가 유효한가요 ? 
		if (!CurrentNotify.HavebeenCalledEvent.contains(EventIter->first) && EventIter->second)
		{
			CurrentNotify.HavebeenCalledEvent.insert(EventIter->first);
			EventIter->second(this);
		}
	}
}

Engine::Bone*
Engine::AnimEffect::MakeHierarchy(
	Bone* BoneParent, const aiNode* const AiNode)
{
	auto TargetBone = std::make_shared<Bone>();
	BoneTable.push_back(TargetBone);
	const uint64 CurBoneIdx = BoneTable.size() - 1;

	BoneTableIdxFromName.insert({ AiNode->mName.C_Str()  ,CurBoneIdx });
	TargetBone->Name = AiNode->mName.C_Str();
	TargetBone->OriginTransform = TargetBone->Transform = FromAssimp(AiNode->mTransformation);
	// TargetBone->Parent = TargetBone.get(); 
	TargetBone->Parent = BoneParent;
	TargetBone->ToRoot = TargetBone->OriginTransform * BoneParent->ToRoot;

	for (uint32 i = 0; i < AiNode->mNumChildren; ++i)
	{
		TargetBone->Childrens.push_back(MakeHierarchy(TargetBone.get(), AiNode->mChildren[i]));
	}

	return TargetBone.get();
};

Engine::Bone* Engine::AnimEffect::MakeHierarchyClone(Bone* BoneParent, const Bone* const PrototypeBone)
{
	auto TargetBone = std::make_shared<Bone>();
	BoneTable.push_back(TargetBone);
	const uint64 CurBoneIdx = BoneTable.size() - 1;

	BoneTableIdxFromName.insert({ PrototypeBone->Name  ,CurBoneIdx });
	TargetBone->Name = PrototypeBone->Name;
	TargetBone->OriginTransform = TargetBone->Transform = PrototypeBone->Transform;
	// TargetBone->Parent = TargetBone.get();
	TargetBone->Parent = BoneParent;
	TargetBone->ToRoot = TargetBone->OriginTransform * BoneParent->ToRoot;
	TargetBone->Offset = PrototypeBone->Offset;

	for (const auto& PrototypeChildren : PrototypeBone->Childrens)
	{
		TargetBone->Childrens.push_back(MakeHierarchyClone(TargetBone.get(), PrototypeChildren));
	}

	return TargetBone.get();
}
;

void Engine::AnimEffect::InitTextureForVertexTextureFetch()&
{
	// 본 테이블 개수만큼 매트릭스가 필요하며 텍셀당 벡터4D 하나씩 저장 가능.
	const float TexPitchPrecision = std::sqrtf(BoneTable.size() * sizeof(Matrix) / 4u);

	const uint8 PowerOfMax = 9u;
	// 2^9 * 2^9 / 4 = 4096개의 행렬을 저장 가능하며 4096개의 본을 가진 캐릭터가 존재하는 게임을 나는 아직 못만듬.
	for (uint8 PowerOf2 = 1u; PowerOf2 < PowerOfMax; ++PowerOf2)
	{
		VTFPitch = std::powl(2, PowerOf2);
		if (VTFPitch >= TexPitchPrecision)
			break;
	}

	Device->CreateTexture
	(VTFPitch, VTFPitch, 1, 0, D3DFMT_A32B32G32R32F, D3DPOOL_MANAGED,
		&BoneAnimMatrixInfo, nullptr);

	static uint64 BoneAnimMatrixInfoTextureResourceID = 0u;

	ResourceSystem::Instance->Insert<IDirect3DTexture9>
		(
			L"AnimEffect_VTF_" + std::to_wstring(BoneAnimMatrixInfoTextureResourceID), BoneAnimMatrixInfo);
};