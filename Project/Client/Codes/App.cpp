#include "..\\stdafx.h"
#include "App.h"
#include "ExportUtility.hpp"
#include "Sound.h"
#include "FontManager.h"
#include "StartScene.h"
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
		std::filesystem::path("") / ".." / ".." / "Resource" / "Sound");

	auto Device = GetGraphic().GetDevice();

	GetFontManager().AddFont(Device.get(), L"Font_Default", L"¹ÙÅÁ", 15, 20, FW_HEAVY);
	GetFontManager().AddFont(Device.get(), L"Font_Jinji", L"±Ã¼­", 15, 20, FW_THIN);
	
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
