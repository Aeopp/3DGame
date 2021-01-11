#pragma once
#include "SingletonInterface.h"
#include "TypeAlias.h"

class App : public SingletonInterface<App>
{
public:
	void Initialize(const HWND _Hwnd)&;
public:
	const auto& GetClientSize()const& { return ClientSize; };
	const auto& GetHwnd()const&       { return _Hwnd; };
private:
	std::pair<uint32,uint32> ClientSize{ 1920u,1080u };
	HWND _Hwnd;
};

