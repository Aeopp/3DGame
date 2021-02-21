#include "Bone.h"
#include "imgui.h"
#include "ResourceSystem.h"
#include "ExportUtility.hpp"

static std::tuple<Vector3, Quaternion, Vector3> CurrentAnimationTransform(

	const double CurrentAnimTimeTrackTime,
	const std::string& Name,

	const std::unordered_map<std::string,
	std::map<double, Vector3>>&ScaleTrack,

	const std::unordered_map<std::string,
	std::map<double, Quaternion>>&QuatTrack,

	const std::unordered_map<std::string,
	std::map<double, Vector3>>&PosTrack
)
{
	const auto& CurNameScaleTrack = ScaleTrack.find(Name)->second;
	const auto ScaleEnd = CurNameScaleTrack.upper_bound(CurrentAnimTimeTrackTime);
	auto ScaleBegin = ScaleEnd;
	std::advance(ScaleBegin, -1);

	const bool bScaleNextFrame = ScaleEnd != std::end(CurNameScaleTrack);

	Vector3 CurAnimScale = ScaleBegin->second;

	if (bScaleNextFrame)
	{
		const double ScaleInterval = ScaleEnd->first - ScaleBegin->first;

		CurAnimScale = FMath::Lerp(
			ScaleBegin->second,
			ScaleEnd->second,
			(CurrentAnimTimeTrackTime - ScaleBegin->first) / ScaleInterval);
	}
	const auto& CurNameQuatTrack = QuatTrack.find(Name)->second;

	const auto QuatEnd = CurNameQuatTrack.upper_bound(CurrentAnimTimeTrackTime);
	auto QuatBegin = QuatEnd;
	std::advance(QuatBegin, -1);

	const bool bQuatNextFrame = QuatEnd != std::end(CurNameQuatTrack);
	Quaternion CurAnimRotation = QuatBegin->second;

	if (bQuatNextFrame)
	{
		const double QuatInterval = QuatEnd->first - QuatBegin->first;

		CurAnimRotation = FMath::SLerp(
			QuatBegin->second,
			QuatEnd->second,
			(CurrentAnimTimeTrackTime - QuatBegin->first) / QuatInterval);
		D3DXQuaternionNormalize(&CurAnimRotation, &CurAnimRotation);
	}

	const auto& CurNamePosTrack = PosTrack.find(Name)->second;
	const auto PosEnd = CurNamePosTrack.upper_bound(CurrentAnimTimeTrackTime);
	auto PosBegin = PosEnd;
	std::advance(PosBegin, -1);

	const bool bPosNextFrame = PosEnd != std::end(CurNamePosTrack);
	Vector3 CurAnimLocation = PosBegin->second;

	if (bPosNextFrame)
	{
		const double PosInterval = PosEnd->first - PosBegin->first;

		CurAnimLocation = FMath::Lerp(PosBegin->second,
			PosEnd->second, (CurrentAnimTimeTrackTime - PosBegin->first) / PosInterval);
	}

	return { CurAnimScale,CurAnimRotation,CurAnimLocation };
};

void Engine::Bone::BoneEdit()
{
	if (ImGui::TreeNode(Name.c_str()))
	{
		bEditObserver = true;
		if (ImGui::CollapsingHeader("Edit"))
		{
			bEditSelect = true; 
			ImGui::Button("Attach");
			ImGui::Button("Detach");
		}
		else
		{
			bEditSelect = false;
		}

		for (const auto& _Children : Childrens)
		{
			_Children->BoneEdit();
		}
		ImGui::TreePop();
	}
	else
	{
		bEditObserver = false;
	}
};

void Engine::Bone::DebugRender(
	const Matrix& World,
	IDirect3DDevice9* Device, 
	ID3DXMesh* const DebugMesh)&
{
	IDirect3DTexture9* CurColorTex{ nullptr };
	auto& ResourceSys = ResourceSystem::Instance;

	if (bEditSelect)
	{
		CurColorTex = ResourceSys->Get<IDirect3DTexture9>(L"Texture_Red");
	}
	else if (bEditObserver)
	{
		CurColorTex = ResourceSys->Get<IDirect3DTexture9>(L"Texture_Blue");
	}
	else
	{
		CurColorTex = ResourceSys->Get<IDirect3DTexture9>(L"Texture_Green");
	}
	
	const Matrix Final = FMath::Scale({ 10,10,10 }) * ToRoot * World ;
	Device->SetTransform(D3DTS_WORLD,&Final);
	Device->SetTexture(0, CurColorTex);
	DebugMesh->DrawSubset(0);

	if (CollisionGeometric)
	{
		CollisionGeometric->Render(Device, true);
	}
};

void Engine::Bone::BoneMatrixUpdate(
	const Matrix& ParentToRoot,
	const double CurrentAnimationTime,

	const std::unordered_map<std::string,
	std::map<double, Vector3>>&ScaleTrack,

	const std::unordered_map<std::string,
	std::map<double, Quaternion>>&QuatTrack,

	const std::unordered_map<std::string,
	std::map<double, Vector3>>&PosTrack,

	const std::optional<AnimationBlendInfo>& IsAnimationBlend)&
{
	// 여기서 이전 프레임과 다음 프레임을 보간 한다.
	Matrix AnimationTransform = OriginTransform;

	const bool bCurBoneApplyAnimation = PosTrack.contains(Name);

	if (bCurBoneApplyAnimation)
	{
		const auto& [CurAnimScale, CurAnimRotation, CurAnimLocation] =
			CurrentAnimationTransform(CurrentAnimationTime, Name, ScaleTrack, QuatTrack, PosTrack);

		if (IsAnimationBlend.has_value())
		{
			 const bool bCurBoneApplyPrevAnimation = IsAnimationBlend->PosTrack.contains(Name);
			
			if (bCurBoneApplyPrevAnimation)
			{
				const auto& [BlendAnimScale, BlendAnimRotation, BlendAnimLocation] =
					CurrentAnimationTransform(
						IsAnimationBlend->AnimationTime, Name,
						IsAnimationBlend->ScaleTrack,
						IsAnimationBlend->QuatTrack,
						IsAnimationBlend->PosTrack);

				const Vector3 LerpAnimScale =
					FMath::Lerp(BlendAnimScale, CurAnimScale,
						1.0 - IsAnimationBlend->PrevAnimationWeight);

				const Quaternion LerpAnimRotation =
					FMath::SLerp(BlendAnimRotation, CurAnimRotation,
						1.0 - IsAnimationBlend->PrevAnimationWeight);

				const Vector3 LerpAnimLocation =
					FMath::Lerp(BlendAnimLocation, CurAnimLocation,
						1.0 - IsAnimationBlend->PrevAnimationWeight);

				AnimationTransform = (FMath::Scale(LerpAnimScale) *
							FMath::Rotation(LerpAnimRotation) *
							FMath::Translation(LerpAnimLocation));
			}
		}
		else
		{
			AnimationTransform = (FMath::Scale(CurAnimScale) *
				FMath::Rotation(CurAnimRotation) *
				FMath::Translation(CurAnimLocation));
		}
	}

    Transform = AnimationTransform;
	ToRoot =    Transform * ParentToRoot;
	Final =     Offset * ToRoot;

	/*if (CollisionGeometric)
	{
		CollisionGeometric->Update()
	}*/

	for (auto& ChildrenTarget : Childrens)
	{
		ChildrenTarget->BoneMatrixUpdate(
			ToRoot, CurrentAnimationTime,
			ScaleTrack,
			QuatTrack,
			PosTrack , IsAnimationBlend);
	}
}
