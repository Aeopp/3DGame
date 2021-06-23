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



void Engine::SkeletonMesh::Update(Object* const Owner,const float DeltaTime)&
{
	Super::Update(Owner, DeltaTime);

	static const Matrix Identity = FMath::Identity();

	CurrentAnimMotionTime  += (DeltaTime * Acceleration);
	PrevAnimMotionTime     += (DeltaTime * PrevAnimAcceleration);
	TransitionRemainTime   -= DeltaTime;

	AnimationNotify();


	if (TransitionRemainTime > 0.0)
	{
		std::optional<Bone::AnimationBlendInfo> IsAnimationBlend = std::nullopt;

		if (PrevAnimMotionTime > AnimInfoTable[PrevAnimIndex].Duration)
		{
			PrevAnimMotionTime = AnimInfoTable[PrevAnimIndex].Duration;
		}

		if (AnimIdx < AnimInfoTable.size() )
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
				_AnimationTrack->ScaleTimeLine[AnimIdx]    ,
				_AnimationTrack->QuatTimeLine[AnimIdx]   ,
				_AnimationTrack->PosTimeLine[AnimIdx]   ,     std::nullopt);
		}
	}
}

Engine::Bone* Engine::SkeletonMesh::MakeHierarchy(
	    Bone* BoneParent,const aiNode* const AiNode)
{
	auto TargetBone = std::make_shared<Bone>(); 
	BoneTable.push_back(TargetBone);// 이후 빠른 검색을 위해 테이블에 삽입
	const uint64 CurBoneIdx = BoneTable.size() - 1;
	// 초기 행렬 설정과 트리 부모관계 구축
	BoneTableIdxFromName.insert({ AiNode->mName.C_Str(),CurBoneIdx });
	TargetBone->Name = AiNode->mName.C_Str();
	TargetBone->OriginTransform = TargetBone->Transform = FromAssimp(AiNode->mTransformation);
	TargetBone->Parent = BoneParent;
	TargetBone->ToRoot = TargetBone->OriginTransform * BoneParent->ToRoot;
	// 자식 뼈대 테이블에 푸시함과 동시에 뼈대 트리 구축을 위해 자식들도 재귀 호출. 
	for (uint32 i = 0; i < AiNode->mNumChildren; ++i)
	{
		TargetBone->Childrens.push_back(MakeHierarchy(TargetBone.get(), AiNode->mChildren[i]));
	}
	auto * const AssignParent = TargetBone.get();
	return AssignParent;
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
	// 2^9 * 2^9 / 4 = 4096 최대 4096개의 본 개수 지원 . 
	// 현재 캐릭터 본 개수 바탕으로 텍스쳐 사이즈를 알아내기 위해 가장 가까운 2의 승수 찾기.
	for (uint8 PowerOf2 = 1u; PowerOf2 < PowerOfMax; ++PowerOf2)
	{
		VTFPitch = std::powl(2, PowerOf2);
		if (VTFPitch >= TexPitchPrecision)
			break;
	}
	// 텍스쳐 포맷 부동소수점 타입 4개로 만들기 여기에 스키닝 행렬정보 밀어넣기. 
	Device->CreateTexture
	(VTFPitch, VTFPitch,1,0, D3DFMT_A32B32G32R32F,D3DPOOL_MANAGED,
		&BoneAnimMatrixInfo, nullptr);

	static uint64 BoneAnimMatrixInfoTextureResourceID = 0u;

	ResourceSystem::Instance->Insert<IDirect3DTexture9>
		(L"VTF_" + std::to_wstring(BoneAnimMatrixInfoTextureResourceID), BoneAnimMatrixInfo);

	Device->CreateTexture
	(VTFPitch, VTFPitch, 1, 0, D3DFMT_A32B32G32R32F, D3DPOOL_MANAGED,
		&PrevBoneAnimMatrixInfo, nullptr);

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

