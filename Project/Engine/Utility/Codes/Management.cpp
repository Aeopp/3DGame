#include "Management.h"

void Engine::Management::Update(const float DeltaTime)&
{
	_CurrentScene->Update(DeltaTime);
}

void Engine::Management::Render()&
{
	_CurrentScene->Render();
}
