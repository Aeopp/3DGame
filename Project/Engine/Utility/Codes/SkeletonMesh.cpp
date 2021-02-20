#include "SkeletonMesh.h"
#include "UtilityGlobal.h"
#include "FMath.hpp"
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
	Super::Initialize(Device);

	auto& ResourceSys = Engine::ResourceSystem::Instance;

	auto ProtoSkeletonMesh = 
		(*ResourceSys->GetAny<std::shared_ptr<Engine::SkeletonMesh>>(ResourceName));

	uint32 IDBackUp = ID;
	this->operator=(*ProtoSkeletonMesh);
	ID = IDBackUp;

	BoneTable.clear();
	BoneTableIdxFromName.clear();
	// Bone Info 
	auto _Bone = std::make_shared<Bone>(); 
	BoneTable.push_back(_Bone);
	const uint64 CurBoneIdx = BoneTable.size() - 1u; 
	Bone* RootBone = _Bone.get();
	BoneTableIdxFromName.insert({AiScene->mRootNode->mName.C_Str() , CurBoneIdx });
	RootBone->Name = AiScene->mRootNode->mName.C_Str();
	RootBone->OriginTransform = RootBone->Transform = 
		FromAssimp(AiScene->mRootNode->mTransformation);
	RootBone->Parent = nullptr;
	RootBone->ToRoot = RootBone->OriginTransform;  *FMath::Identity();

	for (uint32 i = 0; i < AiScene->mRootNode->mNumChildren; ++i)
	{
		RootBone->Childrens.push_back(MakeHierarchy(RootBone, AiScene->mRootNode->mChildren[i]));
	}

	for (uint32 MeshIdx = 0u; MeshIdx < AiScene->mNumMeshes; ++MeshIdx)
	{
		aiMesh* _AiMesh = AiScene->mMeshes[MeshIdx];

		if (_AiMesh->HasBones())
		{
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

	if (Engine::Global::bDebugMode)
	{
		if (ImGui::TreeNode(("SkeletonEdit_" + ToA(Owner->GetName())).c_str()))
		{
			ImGui::BulletText("%s", (std::to_string(ID) + "_" + FullPath.string()).c_str());

			if (ImGui::TreeNode("Animation"))
			{
				if (ImGui::Button("Save"))
				{
					AnimationSave();
				}

				for (uint32 AnimIdx = 0u; AnimIdx < AnimTable.size(); ++AnimIdx)
				{
					std::string AnimName = (AiScene->mAnimations[AnimIdx]->mName.C_Str());
			
					std::string Message = "Index : " + std::to_string(AnimIdx) +
						" Name : " + AnimName;

					if (ImGui::CollapsingHeader(Message.c_str()))
					{
						ImGui::Text("TickPerSecond : %f", AiScene->mAnimations[AnimIdx]->mTicksPerSecond);

						std::string AccelerationMsg = "Acceleration_" + AnimName;

						float FAcceleration = AnimInfoTable[AnimIdx].Acceleration;
						ImGui::SliderFloat(AccelerationMsg.c_str(), &FAcceleration, 0.01f, 100.f);
						AnimInfoTable[AnimIdx].Acceleration = static_cast<double>(FAcceleration);

						std::string TransitionTimeMsg = "TransitionTime_" + AnimName;

						float FTransitionTime = AnimInfoTable[AnimIdx].TransitionTime;
						ImGui::SliderFloat(TransitionTimeMsg.c_str(), &FTransitionTime, 0.01f, 3.f);
						AnimInfoTable[AnimIdx].TransitionTime = static_cast<double>(FTransitionTime);

						std::string PlayMsg = "Play_" + AnimName;

						if (ImGui::Button(PlayMsg.c_str()))
						{
							PlayAnimation(AnimIdx, AnimInfoTable[AnimIdx].Acceleration,
								AnimInfoTable[AnimIdx].TransitionTime);
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

void Engine::SkeletonMesh::Render(	const Matrix& World,
									const Matrix& View,
									const Matrix& Projection,
									const Vector4& CameraLocation4D)&
{
	if (Engine::Global::bDebugMode)
	{
		ImGui::TextColored(ImVec4{ 1.f,114.f/255.f, 198.f/255.f , 1.0f},"Draw : %s", ToA(ResourceName).c_str());
	}

	Super::Render(World,View,Projection,CameraLocation4D );

	std::vector<Matrix> RenderBoneMatricies(BoneTable.size());

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
	std::memcpy(LockRect.pBits, RenderBoneMatricies.data(), RenderBoneMatricies.size() * sizeof(Matrix) );
	BoneAnimMatrixInfo->UnlockRect(0u);

	auto Fx = _ShaderFx.GetHandle();
	auto& Renderer = *Engine::Renderer::Instance;

	Fx->SetMatrix("World", &World);
	Fx->SetMatrix("View", &View);
	Fx->SetMatrix("Projection", &Projection);
	Fx->SetVector("LightDirection", &Renderer.LightDirection);
	Fx->SetVector("LightColor", &Renderer.LightColor);
	Fx->SetVector("CameraLocation", &CameraLocation4D);
	Fx->SetTexture("VTF", BoneAnimMatrixInfo);
	Fx->SetInt("VTFPitch", VTFPitch);
	uint32 PassNum = 0u;
	Fx->Begin(&PassNum, 0);
	
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

	if (bBoneDebug)
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

			Bone* RootBone = BoneTable.front().get();
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
			Bone* RootBone = BoneTable.front().get();

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

void Engine::SkeletonMesh::PlayAnimation(const uint32 AnimIdx)&
{
	this->PlayAnimation
	(AnimIdx, AnimInfoTable[AnimIdx].Acceleration, AnimInfoTable[AnimIdx].TransitionTime);
}

void Engine::SkeletonMesh::PlayAnimation(const std::string& AnimName)&
{
	const uint32 AnimIdx = AnimIdxFromName.find(AnimName)->second;
	this->PlayAnimation(AnimIdx);
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

	static uint64 BoneAnimMatrixInfoTextureResourceID = 0u;

	ResourceSystem::Instance->Insert<IDirect3DTexture9>
		(
			L"VTF_" + std::to_wstring(BoneAnimMatrixInfoTextureResourceID++), BoneAnimMatrixInfo);
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
				Writer.String(AiScene->mAnimations[AnimIdx]->mName.C_Str());

				Writer.Key("Acceleration");
				Writer.Double(AnimInfoTable[AnimIdx].Acceleration);

				Writer.Key("TransitionTime");
				Writer.Double(AnimInfoTable[AnimIdx].TransitionTime);
			}
			Writer.EndObject();
		}
	}
	Writer.EndArray();

	Writer.EndObject();
	std::filesystem::path TargetPath = FullPath; 
	TargetPath.replace_extension("Animation");
	std::ofstream Of{ TargetPath };
	Of << StrBuf.GetString();

};

void Engine::SkeletonMesh::AnimationLoad()&
{
	std::filesystem::path TargetPath = FullPath;
	TargetPath.replace_extension("Animation");
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

