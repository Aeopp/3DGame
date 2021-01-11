#pragma once
#include "SingletonInterface.h"
#include "Management.h"

class App : public SingletonInterface<App>
{
public:
	void Initialize(const HWND _Hwnd)&;
	void GameLoop()&;
public:
	const auto& GetClientSize()const& { return ClientSize; };
private:
	std::pair<uint32_t, uint32_t> ClientSize{1920u,1080u};
};

