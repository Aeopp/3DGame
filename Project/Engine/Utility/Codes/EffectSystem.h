#pragma once
#include "DllHelper.h"
#include "AnimEffect.h"


// 월드 지정 해줘야함.
// 그리는 순서 포워드랑 같이하면 될듯 ?

namespace Engine
{
	class DLL_DECL EffectSystem
	{
	public :
		enum class EffectType :uint8
		{
			AnimEffect ,
		};
		void Initialize(IDirect3DDevice9* const Device)&;
		void LoadEffect(IDirect3DDevice9* const Device,
			const std::filesystem::path FileFullPath,
			const std::wstring& ResourceName,
			const EffectType _EffectType)&;
		Engine::AnimEffect* MakeEffect(const std::wstring& ResourceName ,
										const EffectType _EffectType)&;
		void Update(const float DeltaTime)&;
		void RenderReady(class Renderer* const _Renderer)&;
		void Render(class Renderer* const _Renderer)&;
		void RenderEmissive(class Renderer* const _Renderer)&;
		
		std::unordered_map<std::wstring, IDirect3DTexture9*> EffectTextures{};

		float SoftParticleDepthScale = 30000.f;
	private:
		std::unordered_map<std::wstring, std::shared_ptr<AnimEffect>> ProtoAnimEffects{};
		std::vector<std::shared_ptr<AnimEffect>> AnimEffects{};
	};
}