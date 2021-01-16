#include "..\\stdafx.h"
#include "App.h"
#include "ExportUtility.hpp"
#include "Sound.h"
#include "FontManager.h"
#include "StartScene.h"
#include "imgui.h"
#include "imgui_impl_dx9.h"
#include "imgui_impl_win32.h"
#include <chrono>
#include <filesystem>

void App::Initialize(const HWND _Hwnd, const HINSTANCE HInstance)&
{
	this->Hwnd = _Hwnd;

    Engine::Management::Init(
		_Hwnd,
		HInstance,
		false,
		ClientSize<uint32>,
		D3DMULTISAMPLE_NONE,
		1.f/20.f,
		ResourcePath);

	auto Device = RefGraphic().GetDevice();

	RefFontManager().AddFont(Device.get(), L"Font_Default", L"����", 15, 20, FW_HEAVY);
	RefFontManager().AddFont(Device.get(), L"Font_Jinji", L"�ü�", 15, 20, FW_THIN);
	
	RefSound().Play("song", 1.f, true, true);

	StartSceneLoad();
}
void App::GameLoop()
{
	RefManager().GameLoop();
}

void App::StartSceneLoad()&
{
	Engine::Management::Instance->ChangeScene<StartScene>();
};
