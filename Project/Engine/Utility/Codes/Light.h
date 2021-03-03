#pragma once
#include <array>
#include "TypeAlias.h"
#include "MathStruct.h"
#include "DllHelper.H"
#include "ShaderFx.h"
#include <filesystem>

namespace Engine
{
	class DLL_DECL Light
	{
	public:
		Light();
		Light(Light&&)noexcept = default; 
		Light(const Light&) = default;
		Light&operator=(const Light&) = default;
		Light& operator=(Light&&)noexcept = default;
		~Light()noexcept = default;

		enum class LightOption : uint8
		{
			Directional =0u,
			Point,
			Spot,
		};
		struct LightInformation
		{
			LightOption _LightOpt = { LightOption::Directional };
			Vector4 Location{ 0,0,0 ,1}; 
			Vector4 Direction{ 0,-1,0,0 };
			Vector4 LightColor{ 1,1,1 ,1}; 
			float ShadowDepthBias = { 0.0f };
			float ShadowFar{10000.f};
			float ShadowDepthMapSize = 2048.f;
			float OrthoProjectionSize = ShadowDepthMapSize * 1.5f;
		};
		void Initialize(IDirect3DDevice9* const Device,
			const LightInformation& SetLightInformation)&;
		
		void Render(IDirect3DDevice9* const Device   ,
					const Vector3& CameraLocation, 
					const Matrix& View, const Matrix& Projection,
					IDirect3DTexture9* Albedo3_Contract1,
					IDirect3DTexture9* Normal3_Power1,
					IDirect3DTexture9* WorldPos3_Depth1,
					IDirect3DTexture9* CavityRGB1_CavityAlpha1,
					IDirect3DTexture9* RimRGB1_InnerWidth1_OuterWidth1 ,
					IDirect3DTexture9* ShadowDepth ,
			const Vector3& FogColor,
			const float FogDistance)&;

		Matrix CalcLightViewProjection()const&;

		LightInformation _LightInfo{};
	private:		
		Engine::ShaderFx _DeferredLight{};
		uint32 ID = 0u;
		IDirect3DVertexBuffer9* VtxBuf{ nullptr };
		IDirect3DIndexBuffer9* IdxBuf{ nullptr };
		IDirect3DVertexDeclaration9* VtxDecl{ nullptr };
		
	};
};
