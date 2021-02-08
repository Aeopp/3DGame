#pragma once
#include "AssimpHelper.h"
#include "DllHelper.h"
#include <filesystem>


namespace Engine
{
	namespace Global
	{
		extern DLL_DECL std::pair<uint32, uint32> ClientSize;
		extern DLL_DECL HWND Hwnd;
		extern DLL_DECL bool bDebugMode;
		extern DLL_DECL std::filesystem::path ResourcePath;
		extern DLL_DECL std::filesystem::path ResourcePathA;
		extern DLL_DECL Assimp::Importer AssimpImporter;
	}
}
