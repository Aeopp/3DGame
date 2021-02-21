#pragma once
#include "DxHelper.h"
#include "TypeAlias.h"
#include "DllHelper.H"
#include "FMath.hpp"
#include "AssimpHelper.h"
#include <unordered_map>
#include <string>
#include <map>
#include <optional>
#include "Geometric.h"

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
		std::unique_ptr<Geometric> CollisionGeometric;

		bool bEditSelect = false;
		bool bEditObserver = false;
		std::string Name{};

		struct AnimationBlendInfo
		{
			uint32 PrevAnimationIdx{ 0u };

			const double PrevAnimationWeight = 0.0;
			const double AnimationTime{ 0.0 };

			const std::unordered_map<std::string,
			std::map<double, Vector3>>& ScaleTrack;

			const std::unordered_map<std::string,
			std::map<double, Quaternion>>&QuatTrack;

			const std::unordered_map<std::string, std::map<double, Vector3>>& PosTrack; 
		};

		void BoneEdit();

		void DebugRender( const Matrix& World , IDirect3DDevice9* Device, 
					ID3DXMesh*const DebugMesh)&;

		void BoneMatrixUpdate(
			const Matrix& ParentToRoot,
			const double T,

			const std::unordered_map<std::string,
			std::map<double, Vector3>>&ScaleTrack,

			const std::unordered_map<std::string,
			std::map<double, Quaternion>>&QuatTrack,

			const std::unordered_map<std::string,
			std::map<double, Vector3>>&PosTrack,

			const std::optional<AnimationBlendInfo>& IsAnimationBlend)&;
	};
};



