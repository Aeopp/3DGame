#include "Management.h"
#include "Timer.h"
#include "GraphicDevice.h"
#include <chrono>

void Engine::Management::Initialize(
	const HWND _Hwnd,
	const std::pair<uint32_t, uint32_t> ClientSize)&
{
	Hwnd = _Hwnd;
	this->ClientSize = ClientSize;
}

void Engine::Management::Update(const float DeltaTime)&
{
	if(_CurrentScene)
		_CurrentScene->Update(DeltaTime);
}

void Engine::Management::Render()&
{
	// ·»´õ·¯ÀÇ ·»´õ¸µ ¼öÇà.
}

void Engine::Management::PendingKill()&
{
	if (_CurrentScene)
		_CurrentScene->PendingKill();
}

std::vector<std::shared_ptr<Engine::Layer>>& Engine::Management::RefLayers()&
{
	return _CurrentScene->RefLayers();
};

