#include "Bone.h"
#include "imgui.h"

void Engine::Bone::BoneMatrixUpdate(
	const Matrix ParentToRoot,
	const double T,
	aiAnimation* CurAnimation,
	std::unordered_map<std::string,aiNodeAnim*>* TargetAnimTable,

	std::unordered_map<std::string,
	std::map<double, Vector3>>&ScaleTrack,
	std::unordered_map<std::string,
	std::map<double, Quaternion>>&QuatTrack,
	std::unordered_map<std::string,
	std::map<double, Vector3>>&PosTrack)&
{
	Matrix AnimationTransform = OriginTransform;

	if (CurAnimation)
	{
		if (auto iter = TargetAnimTable->find(Name);
			iter != std::end(*TargetAnimTable))
		{
			aiNodeAnim* CurAnimation = iter->second;

				auto ScaleEnd = ScaleTrack[Name].upper_bound(T);
				auto ScaleBegin = ScaleEnd;
				std::advance(ScaleBegin, -1);

				const double ScaleInterval = ScaleEnd->first - ScaleBegin->first;
				const Vector3 ScaleLerp = FMath::Lerp(ScaleBegin->second, ScaleEnd->second,
					(T -ScaleBegin->first) / ScaleInterval);
			
				auto QuatEnd = QuatTrack[Name].upper_bound(T);
				auto QuatBegin = QuatEnd;
				std::advance(QuatBegin, -1);

				const double QuatInterval = QuatEnd->first - QuatBegin->first;
				 Quaternion QuatLerp = FMath::SLerp(QuatBegin->second, QuatEnd->second,
					(T-QuatBegin->first) / QuatInterval);
				//D3DXQuaternionNormalize(&QuatLerp, &QuatLerp);

				auto PosEnd = PosTrack[Name].upper_bound(T);
				auto PosBegin = PosEnd;
				std::advance(PosBegin, -1);

				const double PosInterval = PosEnd->first - PosBegin->first;
				const Vector3 PosLerp = FMath::Lerp(PosBegin->second, PosEnd->second,
					(T-PosBegin->first) / PosInterval);
			
				AnimationTransform=
			   (FMath::Scale(ScaleLerp)* FMath::Rotation(QuatLerp)* FMath::Translation(PosLerp));

				ImGui::Text("Scale : %f,%f,%f", ScaleLerp.x, ScaleLerp.y , ScaleLerp.z);
				ImGui::Text("Rotation : %f,%f,%f", QuatLerp.x, QuatLerp.y, QuatLerp.z);
				ImGui::Text("Pos : %f,%f,%f", PosLerp.x, PosLerp.y, PosLerp.z);

		}
	}

	Transform = AnimationTransform;
	ToRoot =    Transform * ParentToRoot;
	Final =     Offset * ToRoot;

	for (auto& ChildrenTarget : Childrens)
	{
		ChildrenTarget->BoneMatrixUpdate(
			ToRoot, T, CurAnimation, TargetAnimTable,
			ScaleTrack,
			QuatTrack,
			PosTrack);
	}
}
