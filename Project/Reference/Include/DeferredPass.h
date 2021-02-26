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
		RenderTarget Albedo{};
		RenderTarget Normal{};
		RenderTarget WorldLocations{};
	private:
		IDirect3DDevice9* Device{ nullptr };
	};
};



