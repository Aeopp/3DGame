#include "Management.h"

void Engine::Management::Update(const float DeltaTime)&
{
	_CurrentScene->Update(DeltaTime);

	// ·»´õ·¯ÀÇ ·»´õ¸µ ¼öÇà.

	_CurrentScene->PendingKill();
};

