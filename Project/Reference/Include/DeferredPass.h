#pragma once
#include "DxHelper.h"
#include "TypeAlias.h"
#include "DllHelper.H"
#include "FMath.hpp"
#include <unordered_map>
#include <string>
#include <map>
#include <optional>
#include "RenderTarget.h"

namespace Engine
{
	class DLL_DECL DeferredPass
	{
    public:
		DeferredPass() = default;
		void Initialize(IDirect3DDevice9* const Device)&;
		void DefaultInitialize()&;
	public:
		RenderTarget Albedo3_Contract1{};
		RenderTarget Normal3_Power1{};
		RenderTarget Velocity2_None1_Depth1{};
		RenderTarget CavityRGB1_RimRGB1_RimInnerWidth1_RimOuterWidth1{};
		
		RenderTarget ShadowDepth{};
		RenderTarget DeferredTarget{};
		RenderTarget VelocityMap{};
		RenderTarget MotionBlur{};

		RenderTarget EmissiveTarget{};
		RenderTarget EmissiveBlurX{};
		RenderTarget EmissiveBlurY{};
	private:
		IDirect3DDevice9* Device{ nullptr };
	};
};



