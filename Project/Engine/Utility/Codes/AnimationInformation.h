#pragma once
#include "DllHelper.h"
#include <string>

namespace Engine
{
	struct DLL_DECL AnimationInformation
	{
		std::string Name{};
		double      Duration = 1.f;
		double      TickPerSecond = 30.f;
		double      TransitionTime = 0.25f;
		double      Acceleration = 1.f;
	};
};