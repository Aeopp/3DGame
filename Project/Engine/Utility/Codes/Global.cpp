#include "Global.h"

bool Engine::Global::bDebugMode{ false };

bool Engine::Global::IsDebug()
{
	return bDebugMode;
}
