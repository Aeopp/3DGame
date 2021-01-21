#pragma once
#include "DllHelper.h"


namespace Engine
{
	class DLL_DECL Global
	{
	public:
		bool IsDebug();
	private:
		static bool bDebugMode;
	};
};

inline bool Engine::Global::IsDebug()
{
	return bDebugMode;
}




