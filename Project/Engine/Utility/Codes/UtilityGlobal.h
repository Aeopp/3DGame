#pragma once
#include "AssimpHelper.h"
#include "DllHelper.h"
#include <filesystem>


namespace Engine
{
	namespace Global
	{
		extern DLL_DECL bool bDebugMode;
		extern DLL_DECL std::filesystem::path ResourcePath;
		extern DLL_DECL Assimp::Importer AssimpImporter;
	}
}
