#include "..\\stdafx.h"
#include "App.h"
#include "ExportUtility.hpp"
#include "Sound.h"
#include "StartScene.h"
#include <chrono>
#include <filesystem>

void App::Initialize(const HWND _Hwnd, const HINSTANCE HInstance)&
{
	this->_Hwnd = _Hwnd;

    Engine::Management::Init(
		_Hwnd,
		HInstance,
		false,
		ClientSize,
		D3DMULTISAMPLE_NONE,
		1.f/20.f,
		std::filesystem::path("") / ".." / ".." / "Resource" / "Sound");

	GetSound().Play("song", 1.f, true, true);

	StartSceneLoad();
}
void App::GameLoop()
{
	GetManager().GameLoop();
}

void App::StartSceneLoad()&
{
	Engine::Management::Instance->SetScene<StartScene>();
};
