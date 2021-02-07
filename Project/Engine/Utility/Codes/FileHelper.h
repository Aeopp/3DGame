#pragma once
#include "TypeAlias.h"
#include "DllHelper.H"
#include <filesystem>

namespace Engine
{
	class DLL_DECL FileHelper
	{
	public :
		static std::filesystem::path OpenDialogBox();
	};
};



