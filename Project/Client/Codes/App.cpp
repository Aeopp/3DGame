#include "stdafx.h"
#include "App.h"
#include <chrono>
#include <filesystem>
#include "ExportUtility.hpp"
#include "Sound.h"

void App::Initialize(const HWND _Hwnd)&
{
	this->_Hwnd = _Hwnd;

    Engine::Management::Init(
		_Hwnd,
		false,
		ClientSize,
		D3DMULTISAMPLE_TYPE::D3DMULTISAMPLE_NONE,
		60u,
		30,
		std::filesystem::current_path() / ".." / ".." / "Resource" / "Sound");

	GetSound()->Play("song", 1.f, true, true);
}
void App::GameLoop()
{
	Engine::Management::Instance->GameLoop();
};
