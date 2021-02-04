#pragma once
#include "DxHelper.h"
#include "TypeAlias.h"
#include "DllHelper.H"
#include "FMath.hpp"
#include "AssimpHelper.h"
#include <unordered_map>
#include <string>
#include <map>

namespace Engine
{
	struct DLL_DECL AnimationTrack
	{
		std::vector       <std::unordered_map<std::string,
			std::map<double, Vector3>>>   ScaleTimeLine{};
		std::vector       <std::unordered_map<std::string,
			std::map<double, Quaternion>>>QuatTimeLine{};
		std::vector       <std::unordered_map<std::string,
			std::map<double, Vector3>>>   PosTimeLine{};
	};

	struct DLL_DECL Bone
	{
		Bone* Parent{};
		std::vector<Bone*>    Childrens{};
		Matrix Final          { FMath::Identity() };
		Matrix Transform      { FMath::Identity() };
		Matrix OriginTransform{ FMath::Identity() };
		Matrix ToRoot         { FMath::Identity() };
		Matrix Offset         { FMath::Identity() };
		std::string Name{};

		void BoneMatrixUpdate(
			const Matrix ParentToRoot,
			const double T,
			aiAnimation* CurAnimation,
			std::unordered_map<std::string, aiNodeAnim*>* TargetAnimTable,

			std::unordered_map<std::string,
			std::map<double, Vector3>>&ScaleTrack,
			std::unordered_map<std::string,
			std::map<double, Quaternion>>&QuatTrack,
			std::unordered_map<std::string,
			std::map<double, Vector3>>&  PosTrack)&;
	};
};



