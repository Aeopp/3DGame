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


namespace Engine
{
	class DLL_DECL Renderer : public SingletonInterface<Renderer>
	{
	public:
		struct RenderInformation
		{
			Matrix  View{};
			Matrix  Projection{};
			Vector3 CameraLocation{};
			Vector4 CameraLocation4D{};  
			Matrix  LightViewProjection{};
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

		Engine::Light _DirectionalLight{};
	private:
		void FrustumInCheck()&;
		void RenderReady()&;
		void SetUpRenderInfo()&;
		void BackUpCurBackBuffer()&;
		void RestoreBackBuffer()&;
		void ClearAllRenderTarget()&;
		void BindDeferredPass()&;
		void RenderDeferred()&;
		void BindShadowDepthPass()&;
		void RenderShadowDepth()&;
		void RenderDeferredLight()&;
		void RenderDeferredDebugBuffer()&;

		void RenderDebugCollision()&;
		void RenderNoAlpha()&;
		void RenderUI()&;
	public:
		Vector3 FogColor{ 0.1f,0.1f,0.1f };
		float FogDistance = 10000.f;
	private:
		IDirect3DSurface9* CurBackBufSurface{ nullptr };
		IDirect3DSurface9* CurBackDepthStencil{ nullptr };
		D3DVIEWPORT9 CurViewPort{};
		RenderInformation CurrentRenderInformation{};  
		DeferredPass _DeferredPass{};
		Sky _Sky{};
		Landscape CurrentLandscape{};
		std::map<RenderInterface::Group, std::vector<std::reference_wrapper<RenderInterface>>>RenderObjects;
		DX::SharedPtr<IDirect3DDevice9> Device{ nullptr };
		Frustum _Frustum;
	};
};



