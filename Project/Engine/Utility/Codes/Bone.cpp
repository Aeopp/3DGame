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
				
				Vector3 ScaleLerp;  
				if (ScaleTrack[Name].size() > 1u)
				{
					auto ScaleLast = ScaleTrack[Name].end();
					std::advance(ScaleLast, -1);
					const float ScaleLastT = ScaleLast->first;
					const float ScaleT = T >= ScaleLastT ? (ScaleLastT - (std::numeric_limits<double>::min)()) : T;

					auto ScaleEnd = ScaleTrack[Name].upper_bound(ScaleT);
					auto ScaleBegin = ScaleEnd;
					std::advance(ScaleBegin, -1);

					const double ScaleInterval = (ScaleEnd->first - ScaleBegin->first);
					ScaleLerp = FMath::Lerp(ScaleBegin->second, ScaleEnd->second,
						(T - ScaleBegin->first) / ScaleInterval);
				}
				else
				{
					auto ScaleLast = ScaleTrack[Name].end();
					std::advance(ScaleLast, -1);
					ScaleLerp = ScaleLast->second;
				}

				Quaternion QuatLerp; 
				if (QuatTrack[Name].size() > 1u)
				{
					auto RotationLast = QuatTrack[Name].end();
					std::advance(RotationLast, -1);
					const float RotationLastT = RotationLast->first;
					const float RotationT = T >= RotationLastT ? (RotationLastT - (std::numeric_limits<double>::min)()) : T;

					auto QuatEnd = QuatTrack[Name].upper_bound(RotationT);
					auto QuatBegin = QuatEnd;
					std::advance(QuatBegin, -1);

					const double QuatInterval = QuatEnd->first - QuatBegin->first;

					QuatLerp = FMath::SLerp(QuatBegin->second, QuatEnd->second,
						(T - QuatBegin->first) / QuatInterval);
				}
				else
				{
					auto RotationLast = QuatTrack[Name].end();
					std::advance(RotationLast, -1);
					QuatLerp = RotationLast->second;
				}
				Vector3 PosLerp;  
				if (PosTrack[Name].size() > 1u)
				{
					auto PosLast = PosTrack[Name].end();
					std::advance(PosLast, -1);
					const float PosLastT = PosLast->first;
					const float PosT = T >= PosLastT ? (PosLastT - (std::numeric_limits<double>::min)()) : T;

					auto PosEnd = PosTrack[Name].upper_bound(PosT);
					auto PosBegin = PosEnd;
					std::advance(PosBegin, -1);

					const double PosInterval = PosEnd->first - PosBegin->first;
					PosLerp  = FMath::Lerp(PosBegin->second, PosEnd->second,
						(T - PosBegin->first) / PosInterval);
				}
				else
				{
					auto PosLast = PosTrack[Name].end();
					std::advance(PosLast, -1);
					PosLerp = PosLast->second;
				}
			
				AnimationTransform=
			   (FMath::Scale(ScaleLerp)* FMath::Rotation(QuatLerp)* FMath::Translation(PosLerp));
		}
	}

    Transform = AnimationTransform;
	ToRoot =    Transform * ParentToRoot;
	Final =     Offset * ToRoot;

	if (++Bone::CallCount>= Bone::BoneCount)
	{
		bool bDebug = !!!!!!!!!!!!!!!!!!!!!!!!!!!!true;
	}
	
	/// <summary>
	/// TOOD :: TEST CODE 
	std::cout << Name << std::endl;

	if (Name.find("Bip001-R-Toe0") != std::string::npos)
	{ 
		uint32 Debug = Childrens.size();   // ?? 
		for (auto& ChildrenTarget : Childrens)
		{
			const std::string TargetName = ChildrenTarget->Name;
			std::cout << ChildrenTarget->Name << std::endl;
		}
	}
	/// <summary>

	for (auto& ChildrenTarget : Childrens)
	{
		ChildrenTarget->BoneMatrixUpdate(
			ToRoot, T, CurAnimation, TargetAnimTable,
			ScaleTrack,
			QuatTrack,
			PosTrack);
	}
}
