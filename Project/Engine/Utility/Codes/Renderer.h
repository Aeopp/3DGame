#pragma once
#include "Light.h"
#include "DllHelper.h"
#include "TypeAlias.h"
#include <filesystem>
#include "SingletonInterface.h"
#include "RenderInterface.h"
#include <map>
#include <vector>
#include <d3d9.h>
#include "DxHelper.h"
#include "Frustum.h"
#include "Landscape.h"
#include "Sky.h"
#include "DeferredPass.h"
#include "EffectSystem.h"
#include "UI.h"

namespace Engine
{
	class DLL_DECL Renderer : public SingletonInterface<Renderer>
	{
	public:
		struct RenderInformation
		{
			Matrix  InverseViewProjection{};
			Matrix  View{};
			Matrix  Projection{};
			Matrix  ViewProjection{};
			Vector3 CameraLocation{};
			Vector4 CameraLocation4D{};  
			Matrix  LightViewProjection{};
			Matrix  OrthoProjection{};
			float Far{};
		};

		void Initialize(
			const DX::SharedPtr<IDirect3DDevice9>& Device)&;
		void Update(const float DeltaTime)&; 
		void Render()&; 
		void Regist(RenderInterface* const Target);
		
		Landscape& RefLandscape()&;
		
		void SkyInitialize(const std::filesystem::path& FullPath)&;

		inline DeferredPass& RefDeferredPass()& { return _DeferredPass; };

		void CreateStaticLightResource()&; 

		const RenderInformation& GetCurrentRenderInformation() const& {return CurrentRenderInformation; };
		const RenderInformation& GetPrevRenderInformation() const& { return PrevRenderInformation; };
		Engine::Light _DirectionalLight{};

		IDirect3DTexture9* GetBurnTexture()const& { return BurnTexture;  };
		IDirect3DTexture9* GetDissolveTexture()const& { return DissolveTexture;  };
		IDirect3DTexture9* GetBlueBurnTexture() const& { return BlueBurnTexture;  };

		EffectSystem& RefEffectSystem()& { return EffectSys; };
	public:
		std::weak_ptr<UI> MakeUI(
			const Vector2 Scale,
			const Vector2 Position,
			const std::filesystem::path& TexFullPath,
			const float UIDepthZ )&;
	private:
		void FrustumInCheck()&;
		void RenderReady()&;
		void SetUpRenderInfo()&;
		void SetUpPrevRenderInfo()&;
		void BackUpCurBackBuffer()&;
		void RestoreBackBuffer()&;
		void ClearAllRenderTarget()&;
		void BindDeferredPass()&;
		void RenderDeferred()&;
		void BindVelocity()&;
		void RenderVelocity()&;
		void BindShadowDepthPass()&;
		void RenderShadowDepth()&;

		void BindDeferredTarget()&;
		void RenderDeferredTarget()&;

		void BindMotionBlur()&;
		void RenderMotionBlur()&;
		
		void RenderDeferredDebugBuffer()&;
		void RenderSky()&;

		void RenderEffect()&;

		void RenderDebugCollision()&;
		void RenderAlphaBlend()&;
		void RenderAlphaTest()&;
		void RenderUI()&;
	public:
		Vector3 FogColor{ 0.2901f,0.2901f,0.2901f };
		float FogDistance = 1024.465f;
		float MotionBlurVelocityScale = 0.030f;
		float MotionBlurDepthBias = 0.0001f;
		float MotionBlurLengthMin = 0.000f;
		RenderInformation CurrentRenderInformation{};
	private:
		std::multimap<float,std::shared_ptr<UI>> _UIs{};
		EffectSystem EffectSys{};
		IDirect3DTexture9* BurnTexture{ nullptr };
		IDirect3DTexture9* BlueBurnTexture{ nullptr };
		IDirect3DTexture9* DissolveTexture{ nullptr };
		IDirect3DSurface9* CurBackBufSurface{ nullptr };
		IDirect3DSurface9* CurBackDepthStencil{ nullptr };
		D3DVIEWPORT9 CurViewPort{};
		  
		RenderInformation PrevRenderInformation{};
		DeferredPass _DeferredPass{};
		Sky _Sky{};
		Landscape CurrentLandscape{};
		std::map<RenderInterface::Group, std::vector<std::reference_wrapper<RenderInterface>>>RenderObjects;
		DX::SharedPtr<IDirect3DDevice9> Device{ nullptr };
		Frustum _Frustum;
	};
};



