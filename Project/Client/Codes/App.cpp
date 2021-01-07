#include "stdafx.h"
#include "App.h"
#include "GraphicDevice.h"
#include "Timer.h"
#include <chrono>

void App::Initialize(const HWND _Hwnd)&
{
	this->_Hwnd = _Hwnd;

	Engine::GraphicDevice::Instance().Initialize(_Hwnd,
		false, { ClientSize.first,ClientSize.second}, D3DMULTISAMPLE_16_SAMPLES);

	using namespace std::chrono_literals;

	Engine::Timer::Instance().Initialize(60u, 30ms,
		[this](const float DeltaTime) {Update(DeltaTime); },
		[this](const float DeltaTime) {LateUpdate(DeltaTime); },
		[this]() {Render(); });
}

void App::Update(const float DeltaTime)&
{

}

void App::LateUpdate(const float DeltaTime)&
{
}

void App::Render()&
{
}

