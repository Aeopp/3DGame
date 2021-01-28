#pragma once
#include "DllHelper.h"
#include <filesystem>

namespace Engine::Global
{
	extern DLL_DECL bool bDebugMode;
	extern DLL_DECL std::filesystem::path ResourcePath;
}
