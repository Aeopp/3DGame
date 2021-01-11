#pragma once
#include "SingletonInterface.h"

class App : public SingletonInterface<App>
{
public:
	void Initialize(const HWND _Hwnd)&;
	void GameLoop()&;
public:
	const auto& GetClientSize()const& { return ClientSize; };
	const auto& GetHwnd()const& { return _Hwnd; };
private:
	std::pair<uint32_t, uint32_t> ClientSize{ 1920u,1080u };
	HWND _Hwnd;
};

