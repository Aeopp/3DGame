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
		RenderTarget WorldLocation3_Depth1{};
		RenderTarget CavityRGB1_CavityAlpha1_NULL_NULL1{};

		RenderTarget RimRGB1_InnerWidth1_OuterWidth1_NULL1{};
		RenderTarget ShadowDepth{};
	private:
		IDirect3DDevice9* Device{ nullptr };
	};
};



