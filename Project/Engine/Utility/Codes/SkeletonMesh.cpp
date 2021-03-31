#include "SkeletonMesh.h"
#include "UtilityGlobal.h"
#include "FMath.hpp"
#include "Transform.h"
#include <future>
#include <set>
#include <optional>
#include "imgui.h"
#include "Renderer.h"
#include <rapidjson/prettywriter.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/reader.h> 
#include <rapidjson/document.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/istreamwrapper.h>
#include "FileHelper.h"
#include <fstream>
#include <ostream>

void Engine::SkeletonMesh::ChangeModel(IDirect3DDevice9* const Device,
	const std::filesystem::path& FileFullPath)&
{
	using VertexType = Vertex::LocationTangentUV2DSkinning;
	const aiScene* const  AiScene =
		Engine::Global::AssimpImporter.ReadFile(
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

	LocalVertexLocations = std::make_shared<std::vector<Vector3>>();
	static uint32 SkeletonResourceID = 0u;
	MeshContainer.clear();
	for (uint32 MeshIdx = 0u; MeshIdx < AiScene->mNumMeshes; ++MeshIdx)
	{
		const std::wstring CurrentResourceName =
			L"ChangeModel_" + std::to_wstring(SkeletonResourceID++);

		MeshElement CreateMesh{};
		aiMesh* _AiMesh = AiScene->mMeshes[MeshIdx];
		// 버텍스 버퍼.
		std::shared_ptr<std::vector<VertexType>> 
			Verticies = std::make_shared<std::vector<VertexType>>();

		for (uint32 VerticesIdx = 0u; VerticesIdx < _AiMesh->mNumVertices; ++VerticesIdx)
		{
			Verticies->push_back(VertexType::MakeFromAssimpMesh(_AiMesh, VerticesIdx));
			LocalVertexLocations->push_back(FromAssimp(_AiMesh->mVertices[VerticesIdx]));
		}

		const std::wstring MeshVtxBufResourceName =
			L"ChangeModel_SkeletonMesh_VertexBuffer_" + CurrentResourceName;
		IDirect3DVertexBuffer9* _VertexBuffer{ nullptr };
		CreateMesh.VtxBufSize = sizeof(VertexType) * Verticies->size();
		Device->CreateVertexBuffer(CreateMesh.VtxBufSize, D3DUSAGE_DYNAMIC, NULL,
			D3DPOOL_DEFAULT, &_VertexBuffer, nullptr);
		CreateMesh.VertexBuffer = ResourceSys->Insert<IDirect3DVertexBuffer9>(MeshVtxBufResourceName, _VertexBuffer);
		CreateMesh.PrimitiveCount = CreateMesh.FaceCount = _AiMesh->mNumFaces;

		CreateMesh.VtxCount = Verticies->size();
		CreateMesh.Stride = sizeof(VertexType);
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
			L"ChangeModel_SkeletonMesh_IndexBuffer_" + CurrentResourceName;
		IDirect3DIndexBuffer9* _IndexBuffer{ nullptr };
		const uint32 IdxBufSize = sizeof(uint32) * Indicies.size();
		Device->CreateIndexBuffer(IdxBufSize, D3DUSAGE_WRITEONLY, D3DFMT_INDEX32,
			D3DPOOL_MANAGED, &_IndexBuffer, nullptr);
		CreateMesh.IndexBuffer = ResourceSys->Insert<IDirect3DIndexBuffer9>(MeshIdxBufResourceName,
			_IndexBuffer);
		uint32* IndexBufferPtr{ nullptr };
		CreateMesh.IndexBuffer->Lock(0, 0, reinterpret_cast<void**>(&IndexBufferPtr), NULL);
		std::memcpy(IndexBufferPtr, Indicies.data(), IdxBufSize);
		CreateMesh.IndexBuffer->Unlock();

		// 머테리얼 파싱 . 
		aiMaterial* AiMaterial = AiScene->mMaterials[_AiMesh->mMaterialIndex];

		const std::wstring
			MatName = ToW(AiScene->mMaterials[_AiMesh->mMaterialIndex]->GetName().C_Str());

		CreateMesh.MaterialInfo.Load(Device,
			FilePath / L"Material", MatName + L".mat", L".tga");
		///// 

		// Vtx Bone 정보,
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
								FindVtxCurrentBoneSlot((*Verticies)[VtxIdx].Weights);

							(*Verticies)[VtxIdx].Weights[CurSlot] = _Wit;
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
	VtxDecl = ResourceSys->Get<IDirect3DVertexDeclaration9>(VtxTypeNameW);
	if (!VtxDecl)
	{
		VtxDecl = ResourceSys->Insert<IDirect3DVertexDeclaration9>(VtxTypeNameW, VertexType::GetVertexDecl(Device));
	}

	// 애니메이션 데이터 파싱.
	/*if (AiScene->HasAnimations())
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
	}*/

	// AnimationLoad();

}

void Engine::SkeletonMesh::Initialize(const std::wstring& ResourceName)&
{
	auto& ResourceSys = Engine::ResourceSystem::Instance;

	auto ProtoSkeletonMesh = 
		(*ResourceSys->GetAny<std::shared_ptr<Engine::SkeletonMesh>>(ResourceName));

	uint32 IDBackUp = ID;
	this->operator=(*ProtoSkeletonMesh);
	ID = IDBackUp;

	Super::Initialize(Device, _Group);

	auto ProtoRootBone = ProtoSkeletonMesh->BoneTable.front();
	ProtoSkeletonMesh->BoneTableIdxFromName;

	BoneTable.clear();
	BoneTableIdxFromName.clear();
	// Bone Info 
	auto _Bone = std::make_shared<Bone>(); 
	BoneTable.push_back(_Bone);
	const uint64 CurBoneIdx = BoneTable.size() - 1u; 
	Bone* RootBone = _Bone.get();
	BoneTableIdxFromName.insert({ ProtoRootBone ->Name, CurBoneIdx });
	RootBone->Name = ProtoRootBone->Name;
	RootBone->OriginTransform = RootBone->Transform = ProtoRootBone->Transform;
	RootBone->Parent = nullptr;
	RootBone->ToRoot = RootBone->OriginTransform;  *FMath::Identity();

	for (const auto& ProtoBoneChildren : ProtoRootBone->Childrens)
	{
		RootBone->Childrens.push_back(MakeHierarchyClone(RootBone, ProtoBoneChildren));
	}

	InitTextureForVertexTextureFetch();

	ForwardShaderFx.Initialize(L"SkeletonSkinningDefaultFx");
	DepthShadowFx.Initialize(L"ShadowDepthSkeletonFx");
	DeferredDefaultFx.Initialize(L"DeferredAlbedoNormalVelocityDepthSpecularRimSkeletonFx");
	DeferredDissolveFx.Initialize(L"DeferredDissolveAlbedoNormalVelocityDepthSpecularRimSkeletonFx");
	VelocityFx.Initialize(L"VelocitySkinningFx");
	RenderBoneMatricies.resize(BoneTable.size());
}

void Engine::SkeletonMesh::Event(Object* Owner)&
{
	Super::Event(Owner);

	if (Engine::Global::bDebugMode)
	{
		if (ImGui::TreeNode(("SkeletonEdit_" + ToA(Owner->GetName())).c_str()))
		{
			ImGui::BulletText("%s", (std::to_string(ID) + "_" + (FilePath / FilePureName).string()).c_str());

			if (ImGui::TreeNode("Animation"))
			{
				if (ImGui::Button("Save"))
				{
					AnimationSave();
				}

				for (uint32 AnimIdx = 0u; AnimIdx < AnimInfoTable.size(); ++AnimIdx)
				{
					std::string AnimName = ((AnimInfoTable[AnimIdx].Name).c_str());

					std::string Message = "Index : " + std::to_string(AnimIdx) +
						" Name : " + AnimName;

					if (ImGui::CollapsingHeader(Message.c_str()))
					{
						ImGui::Text("TickPerSecond : %f", AnimInfoTable[AnimIdx].TickPerSecond);

						std::string AccelerationMsg = "Acceleration_" + AnimName;

						float FAcceleration = AnimInfoTable[AnimIdx].Acceleration;
						ImGui::SliderFloat(AccelerationMsg.c_str(), &FAcceleration, 0.01f, 100.f);
						AnimInfoTable[AnimIdx].Acceleration = static_cast<double>(FAcceleration);

						std::string TransitionTimeMsg = "TransitionTime_" + AnimName;

						float FTransitionTime = AnimInfoTable[AnimIdx].TransitionTime;
						ImGui::SliderFloat(TransitionTimeMsg.c_str(), &FTransitionTime, 0.01f, 3.f);
						AnimInfoTable[AnimIdx].TransitionTime = static_cast<double>(FTransitionTime);

						float FDurationTime = AnimInfoTable[AnimIdx].Duration;
						ImGui::Text("Duration : %.3f", FDurationTime);

						const float CurrentMotionNormalizeTime = GetCurrentNormalizeAnimTime(); 
						ImGui::Text("Current Motion Normalize Time : %.3f",CurrentMotionNormalizeTime);

						
						std::string PlayMsg = "Play_" + AnimName;

						if (ImGui::Button(PlayMsg.c_str()))
						{
							Engine::SkeletonMesh::AnimNotify _AnimNotify{};
							_AnimNotify.bLoop = true;
							_AnimNotify.Name = AnimName;
							PlayAnimation(AnimIdx, FAcceleration, FTransitionTime, _AnimNotify);
						}

						ImGui::Separator();
					}
				};
				ImGui::TreePop();
			};

			if (ImGui::TreeNode("Bone"))
			{
				bBoneDebug = true;
				BoneTable.front()->BoneEdit();
				ImGui::TreePop();
			}
			else
			{
				bBoneDebug = false;
			}

			ImGui::TreePop();
		}
	}
}
void Engine::SkeletonMesh::Render(Engine::Renderer* const _Renderer)&
{
	auto Fx = ForwardShaderFx.GetHandle();
	Fx->SetTexture("VTF", BoneAnimMatrixInfo);
	Fx->SetInt("VTFPitch", VTFPitch);
	Super::Render(_Renderer);

	Device->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
	Device->SetRenderState(D3DRS_ZENABLE, FALSE);
	auto& ResourceSys = ResourceSystem::Instance;
	ID3DXMesh* const _DebugMesh = ResourceSys->Get<ID3DXMesh>(L"SphereMesh");
	for (auto& _Bone : BoneTable)
	{
		_Bone->DebugRender(OwnerWorld, Device, _DebugMesh);
	}
	Device->SetRenderState(D3DRS_ZENABLE, TRUE);
	Device->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
}
void Engine::SkeletonMesh::RenderDeferredAlbedoNormalVelocityDepthSpecularRim(Engine::Renderer* const _Renderer)&
{
	ID3DXEffect* Fx = nullptr;

	if (_DissolveInfo.has_value())
	{
		Fx = DeferredDissolveFx.GetHandle();
	}
	else
	{
		Fx = DeferredDefaultFx.GetHandle();

	}

	Fx->SetTexture("VTF", BoneAnimMatrixInfo);
	Fx->SetInt("VTFPitch", VTFPitch);
	Super::RenderDeferredAlbedoNormalVelocityDepthSpecularRim(_Renderer);

	if (Engine::Global::bDebugMode)
	{
		Device->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
		Device->SetRenderState(D3DRS_ZENABLE, FALSE);
		auto& ResourceSys = ResourceSystem::Instance;
		ID3DXMesh* const _DebugMesh = ResourceSys->Get<ID3DXMesh>(L"SphereMesh");
		for (auto& _Bone : BoneTable)
		{
			_Bone->DebugRender(OwnerWorld, Device, _DebugMesh);
		}
		Device->SetRenderState(D3DRS_ZENABLE, TRUE);
		Device->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
	}
	
}
void Engine::SkeletonMesh::RenderShadowDepth(Engine::Renderer* const _Renderer)&
{
	auto Fx = DepthShadowFx.GetHandle();
	Fx->SetTexture("VTF", BoneAnimMatrixInfo);
	Fx->SetInt("VTFPitch", VTFPitch);

	Super::RenderShadowDepth(_Renderer);
}
void Engine::SkeletonMesh::RenderReady(Engine::Renderer* const _Renderer)&
{
	Super::RenderReady(_Renderer);

	std::swap(BoneAnimMatrixInfo, PrevBoneAnimMatrixInfo);

	std::transform(std::begin(BoneTable), std::end(BoneTable),
		std::begin(RenderBoneMatricies), []
		(const std::shared_ptr<Bone>& CopyBoneFinalMatrix)
		{
			return CopyBoneFinalMatrix->Final;
		});

	D3DLOCKED_RECT LockRect{ 0u, };
	if (FAILED(BoneAnimMatrixInfo->LockRect(0u, &LockRect, nullptr, 0)))
	{
		assert(NULL);
	}

	std::memcpy(LockRect.pBits, RenderBoneMatricies.data(), RenderBoneMatricies.size() * sizeof(Matrix));
	BoneAnimMatrixInfo->UnlockRect(0u);
};

void Engine::SkeletonMesh::RenderVelocity(Engine::Renderer* const _Renderer)&
{
	auto Fx = VelocityFx.GetHandle();
	Fx->SetTexture("VTF", BoneAnimMatrixInfo);
	Fx->SetTexture("PrevVTF", PrevBoneAnimMatrixInfo);
	Fx->SetInt("VTFPitch", VTFPitch);
	
	Super::RenderVelocity(_Renderer);
}



void Engine::SkeletonMesh::Update(Object* const Owner, const float DeltaTime)&
{
	Super::Update(Owner, DeltaTime);

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
}

Engine::Bone*
Engine::SkeletonMesh::MakeHierarchy(
	Bone* BoneParent,const aiNode* const AiNode)
{
	auto TargetBone = std::make_shared<Bone>(); 
	BoneTable.push_back(TargetBone);
	const uint64 CurBoneIdx = BoneTable.size() - 1;

	if (BoneTableIdxFromName.contains(AiNode->mName.C_Str()))
	{
		std::terminate();
	}

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
}

Engine::Bone* Engine::SkeletonMesh::MakeHierarchyClone(Bone* BoneParent, const Bone* const PrototypeBone)
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

Engine::SkeletonMesh::AnimNotify Engine::SkeletonMesh::GetCurrentAnimNotify() const&
{
	return CurrentNotify;
}

float Engine::SkeletonMesh::GetCurrentNormalizeAnimTime() const&
{
	return CurrentAnimMotionTime / AnimInfoTable[AnimIdx].Duration;
}

void Engine::SkeletonMesh::PlayAnimation(const uint32 AnimIdx ,
	                                     const double Acceleration ,
										 const double TransitionDuration,
										 const AnimNotify& _AnimNotify)&
{
	PrevAnimMotionTime    = CurrentAnimMotionTime;
	CurrentAnimMotionTime = 0.0;
	PrevAnimIndex = this->AnimIdx;

	this->AnimIdx = AnimIdx;
	this->TransitionDuration = TransitionRemainTime = TransitionDuration;
	this->PrevAnimAcceleration = this->Acceleration;
	this->Acceleration = Acceleration;

	CurrentNotify = _AnimNotify;
}

void Engine::SkeletonMesh::PlayAnimation(const Engine::SkeletonMesh::AnimNotify& _AnimNotify)&
{
	const uint32 AnimIdx = AnimIdxFromName.find(_AnimNotify.Name)->second;
	this->PlayAnimation(AnimIdx, AnimInfoTable[AnimIdx].Acceleration, AnimInfoTable[AnimIdx].TransitionTime, _AnimNotify);
}

void Engine::SkeletonMesh::AnimationEnd()&
{
	if (CurrentNotify.bLoop)
	{
		PlayAnimation(CurrentNotify);
	}
	else
	{
		CurrentAnimMotionTime = AnimInfoTable[AnimIdx].Duration;
		CurrentNotify.bAnimationEnd = true;
	}
}

void Engine::SkeletonMesh::AnimationNotify()&
{
	const float AnimDurationNormalize = CurrentAnimMotionTime / AnimInfoTable[AnimIdx].Duration;
	auto EventIter = CurrentNotify.AnimTimeEventCallMapping.lower_bound(AnimDurationNormalize);

	bool bTiming = (std::end(CurrentNotify.AnimTimeEventCallMapping) != EventIter) && 
		AnimDurationNormalize >= EventIter->first;

	if (bTiming)
	{
		// 과거에 호출 한 적이 없으며 함수가 유효한가요 ? 
		if (!CurrentNotify.HavebeenCalledEvent.contains(EventIter->first)&& EventIter->second)
		{
			CurrentNotify.HavebeenCalledEvent.insert(EventIter->first);
			EventIter->second(this);
		}
	}
}

void Engine::SkeletonMesh::InitTextureForVertexTextureFetch()&
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
	(VTFPitch, VTFPitch,1,0, D3DFMT_A32B32G32R32F,D3DPOOL_MANAGED,
		&BoneAnimMatrixInfo, nullptr);

	Device->CreateTexture
	(VTFPitch, VTFPitch, 1, 0, D3DFMT_A32B32G32R32F, D3DPOOL_MANAGED,
		&PrevBoneAnimMatrixInfo, nullptr);

	static uint64 BoneAnimMatrixInfoTextureResourceID = 0u;

	ResourceSystem::Instance->Insert<IDirect3DTexture9>
		(
			L"VTF_" + std::to_wstring(BoneAnimMatrixInfoTextureResourceID), BoneAnimMatrixInfo);

	ResourceSystem::Instance->Insert<IDirect3DTexture9>
		(
			L"PrevVTF_" + std::to_wstring(BoneAnimMatrixInfoTextureResourceID++), PrevBoneAnimMatrixInfo);
}

void Engine::SkeletonMesh::AnimationSave()&
{
	using namespace rapidjson;

	StringBuffer StrBuf;
	PrettyWriter<StringBuffer> Writer(StrBuf);
	// Cell Information Write...
	Writer.StartObject();
	Writer.Key("AnimationInfoTable");
	Writer.StartArray();
	{
		for (uint32 AnimIdx = 0u; AnimIdx < AnimInfoTable.size(); ++AnimIdx)
		{
			Writer.StartObject();
			{
				Writer.Key("Index");
				Writer.Uint(AnimIdx);

				Writer.Key("Name");
				Writer.String((AnimInfoTable[AnimIdx].Name).c_str());

				Writer.Key("Acceleration");
				Writer.Double(AnimInfoTable[AnimIdx].Acceleration);

				Writer.Key("TransitionTime");
				Writer.Double(AnimInfoTable[AnimIdx].TransitionTime);

				Writer.Key("Duration");
				Writer.Double(AnimInfoTable[AnimIdx].Duration);
			}
			Writer.EndObject();
		}
	}
	Writer.EndArray();

	Writer.EndObject();
	std::filesystem::path TargetPath = FilePath;
	TargetPath /= L"Animation";
	TargetPath /= FilePureName;
	TargetPath.replace_extension("json");
	std::ofstream Of{ TargetPath };
	Of << StrBuf.GetString();

};

void Engine::SkeletonMesh::AnimationLoad()&
{
	std::filesystem::path TargetPath = FilePath;
	TargetPath /= L"Animation";
	TargetPath /= FilePureName;
	TargetPath.replace_extension("json");
	std::ifstream Is{ TargetPath };
	using namespace rapidjson;
	if (!Is.is_open()) return;

	IStreamWrapper Isw(Is);
	Document _Document;
	_Document.ParseStream(Isw);

	if (_Document.HasParseError())
	{
		MessageBox(Engine::Global::Hwnd, L"Json Parse Error", L"Json Parse Error", MB_OK);
		return;
	}

	const Value& AnimationJsonTable = _Document["AnimationInfoTable"];

	const auto& AnimTableArray  =AnimationJsonTable.GetArray();
	for (auto iter = AnimTableArray.begin();
		iter != AnimTableArray.end(); ++iter)
	{
		const uint32 Idx = iter->FindMember("Index")->value.GetUint();
		AnimInfoTable[Idx].Acceleration = iter->FindMember("Acceleration")->value.GetDouble();
		AnimInfoTable[Idx].TransitionTime = iter->FindMember("TransitionTime")->value.GetDouble();
	}
}

