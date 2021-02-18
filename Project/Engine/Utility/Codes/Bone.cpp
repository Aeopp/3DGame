#include "Bone.h"
#include "imgui.h"



void Engine::Bone::BoneMatrixUpdate(
	const Matrix& ParentToRoot,
	const double T,
	const aiAnimation*const  CurAnimation,
	const std::unordered_map<std::string,aiNodeAnim*>* TargetAnimTable,

	const std::unordered_map<std::string,
	std::map<double, Vector3>>&ScaleTrack,

	const std::unordered_map<std::string,
	std::map<double, Quaternion>>&QuatTrack,

	const std::unordered_map<std::string,
	std::map<double, Vector3>>&PosTrack)&
{
	// 여기서 이전 프레임과 다음 프레임을 보간 한다.

	Matrix AnimationTransform = OriginTransform;

	if (CurAnimation)
	{
		if (auto iter = TargetAnimTable->find(Name);
			iter != std::end(*TargetAnimTable))
		{
			    const aiNodeAnim* const CurAnimation = iter->second;
				
				const auto& CurNameScaleTrack = ScaleTrack.find(Name)->second;
				const auto ScaleEnd = CurNameScaleTrack.upper_bound(T);
				auto ScaleBegin = ScaleEnd;
				std::advance(ScaleBegin, -1);

				const bool bScaleNextFrame = ScaleEnd != std::end(CurNameScaleTrack);
				Vector3 ScaleLerp = ScaleBegin->second;

				if (bScaleNextFrame)
				{
					const double ScaleInterval = ScaleEnd->first - ScaleBegin->first;
					
					ScaleLerp= FMath::Lerp(
						ScaleBegin->second, 
						ScaleEnd->second,
						(T - ScaleBegin->first) / ScaleInterval);
				}
				const auto& CurNameQuatTrack = QuatTrack.find(Name)->second;

				const auto QuatEnd = CurNameQuatTrack.upper_bound(T);
				auto QuatBegin = QuatEnd;
				std::advance(QuatBegin, -1);

				const bool bQuatNextFrame = QuatEnd != std::end(CurNameQuatTrack);
				Quaternion  QuatLerp = QuatBegin->second;

				if (bQuatNextFrame)
				{
					const double QuatInterval = QuatEnd->first - QuatBegin->first;
					
					QuatLerp = FMath::Lerp(
						QuatBegin->second,
						QuatEnd->second, 
						(T - QuatBegin->first) / QuatInterval);
					D3DXQuaternionNormalize(&QuatLerp, &QuatLerp);
				}

				const auto& CurNamePosTrack = PosTrack.find(Name)->second;
				const auto PosEnd = CurNamePosTrack.upper_bound(T);
				auto PosBegin = PosEnd;
				std::advance(PosBegin, -1);

				const bool bPosNextFrame = PosEnd != std::end(CurNamePosTrack);
				Vector3 PosLerp = PosBegin->second;

				if (bPosNextFrame)
				{
					const double PosInterval = PosEnd->first - PosBegin->first;
				
					PosLerp = FMath::Lerp(PosBegin->second,
						PosEnd->second,	(T - PosBegin->first) / PosInterval);
				}
			
				AnimationTransform=(FMath::Scale(ScaleLerp)* FMath::Rotation(QuatLerp)* FMath::Translation(PosLerp));
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
