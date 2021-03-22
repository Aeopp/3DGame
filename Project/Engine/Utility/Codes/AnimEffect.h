#pragma once
#include "Mesh.h"
#include "Vertexs.hpp"
#include <vector>
#include <unordered_map>
#include <string>
#include "FMath.hpp"
#include <filesystem>
#include "DllHelper.h"
#include "UtilityGlobal.h"
#include "ResourceSystem.h"
#include "ExportUtility.hpp"
#include <optional>
#include <set>
#include "ShaderFx.h"
#include "StringHelper.h"
#include "Bone.h"
#include "AnimationInformation.h"


// 월드 지정 해줘야함.
// 그리는 순서 포워드랑 같이하면 될듯 ?

namespace Engine
{
	class DLL_DECL AnimEffect
	{
	public :
		struct DLL_DECL AnimEffectMesh
		{
			IDirect3DVertexBuffer9* VertexBuffer{ nullptr };
			IDirect3DIndexBuffer9* IndexBuffer{ nullptr };
			DWORD  Stride{ 0u };
			uint32 FaceCount{ 0u };
			uint32 VtxCount{ 0u };
			uint32 PrimitiveCount{ 0u };
			uint32 VtxBufSize{ 0u };
			
			IDirect3DTexture9* Diffuse{ nullptr };
		};

		void  Load(IDirect3DDevice9* const Device,
			  const std::filesystem::path FileFullPath)&;

		void Initialize()&;

		void RenderReady(Engine::Renderer* const _Renderer)&;
		void Render(Engine::Renderer* const _Renderer)&;
		void Update(const float DeltaTime)&;
	public:
		struct DLL_DECL AnimNotify
		{
			std::string Name{};
			bool bAnimationEnd{ false };
			bool bLoop{ false };
			std::map<double, std::function<void(Engine::AnimEffect* const)>, std::greater<float>> AnimTimeEventCallMapping{};
			std::set<double> HavebeenCalledEvent{};
		};

		void  PlayAnimation(
			const uint32 AnimIdx,
			const double Acceleration,
			const double TransitionDuration, const AnimNotify& _AnimNotify)&;

		Engine::AnimEffect::AnimNotify GetCurrentAnimNotify()const&;
		float GetCurrentNormalizeAnimTime()const&;

		void AnimationEnd()&;
		void AnimationNotify()&;

		uint32 PrevAnimIndex = 0u;
		uint32 AnimIdx{ 0u };
		uint32 AnimEndAfterAnimIdx{ 0u };

		double CurrentAnimMotionTime{ 0.0 };
		double PrevAnimMotionTime{ 0.0 };
		double TransitionRemainTime = -1.0;
		double TransitionDuration = 0.0;

		
		uint32 MaxAnimIdx{ 0u };
	public:
		Vector3 Scale{ 1,1,1 };
		Vector3 Rotation{ 0, 0, 0 };
		Vector3 Location{ 0, 0, 0 };
		// AnimEffect Info 
		struct AnimEffectInfo
		{
			float Time = 0.0f;
			float AlphaFactor = 0.0f;
			float Brightness = 1.f;
			bool bRender = true;
			Vector2 GradientUVOffsetFactor{ 0.f,1.f };

			IDirect3DTexture9* DiffuseMap{ nullptr };
			IDirect3DTexture9* PatternMap{ nullptr };
			IDirect3DTexture9* AddColorMap{ nullptr };
			IDirect3DTexture9* UVDistorMap{ nullptr };
			IDirect3DTexture9* GradientMap{ nullptr };
			
		};
		std::function<void(AnimEffectInfo&,float)> _AnimEffectUpdateCall{};
		AnimEffectInfo _CurAnimEffectInfo{};
		//
	private:
		Matrix  World{ FMath::Identity() };

		Engine::Bone*
			MakeHierarchy(Bone* BoneParent, const aiNode* const AiNode);
		Engine::Bone*
			MakeHierarchyClone(Bone* BoneParent, const Bone* const PrototypeBone);

		void InitTextureForVertexTextureFetch()&;
	private:
		AnimNotify CurrentNotify{};
		double PrevAnimAcceleration = 1.f;
		double Acceleration = 1.f;

		std::vector<Matrix> RenderBoneMatricies{};
		Engine::ShaderFx AnimEffectFx{};
		IDirect3DDevice9* Device{ nullptr };
		std::vector<AnimationInformation>                          AnimInfoTable{ };
		std::unordered_map<std::string, uint32>                     AnimIdxFromName{};
		IDirect3DVertexDeclaration9* VtxDecl{ nullptr };
		std::shared_ptr<std::vector<Vector3>> LocalVertexLocations{};
		std::shared_ptr<AnimationTrack>                             _AnimationTrack{};
		std::unordered_map<std::string, uint64>						BoneTableIdxFromName{};
		std::vector<std::shared_ptr<Bone>>							BoneTable{};
		IDirect3DTexture9* BoneAnimMatrixInfo{ nullptr };
		std::vector<AnimEffectMesh> MeshContainer{};
		int32 VTFPitch{ 0 };
	};
}