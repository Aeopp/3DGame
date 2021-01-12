#pragma once
#include "SingletonInterface.h"
#include "TypeAlias.h"
#include "Management.h"

class App : public SingletonInterface<App>
{
public:
	void Initialize(const HWND _Hwnd)&;
public:
	void GameLoop();
public:
	static const auto& GetClientSize() { return ClientSize; };
	const auto& GetHwnd()const& { return _Hwnd; };
private:
	static inline std::pair<uint32,uint32> ClientSize{ 1920u,1080u };
	HWND _Hwnd;
};

