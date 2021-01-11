#include "Management.h"
#include "Timer.h"
#include "GraphicDevice.h"
#include <chrono>

void Engine::Management::Initialize(
	const HWND _Hwnd,
	const std::pair<uint32_t,uint32_t> ClientSize)&
{
	Hwnd = _Hwnd;
	this->ClientSize = ClientSize;

	/*Engine::GraphicDevice::Instance().Initialize(
		_Hwnd,
		false,
		{ ClientSize.first,ClientSize.second },
		D3DMULTISAMPLE_16_SAMPLES);

	using namespace std::chrono_literals;

	Engine::Timer::Instance().Initialize(
		60u, 30ms,
		[](const float DeltaTime) {Engine::Management::Instance().Update(DeltaTime); },
		[]() {Engine::Management::Instance().PendingKill(); },
		[]() {Engine::Management::Instance().Render(); });*/
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

