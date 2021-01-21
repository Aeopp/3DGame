#pragma once
#include "DllHelper.h"

namespace Engine
{
	class DLL_DECL Global
	{
	public:
		static void DebugToggle();
		static bool bDebugMode;
	};
};

bool Engine::Global::bDebugMode{ false };








