#include "Management.h"

void Engine::Management::Update(const float DeltaTime)&
{
	_CurrentScene->Update(DeltaTime);

	_CurrentScene->Render();


};

