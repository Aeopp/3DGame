#pragma once
#include "SingletonInterface.h"

class App : public SingletonInterface<App>
{
public:
	void Initialize(const HWND _Hwnd)&;
	void Update(const float DeltaTime)&;
	void LateUpdate(const float DeltaTime)&;
	void Render()&;
public:
	const auto& GetClientSize()const& { return ClientSize; };
	const auto& GetHwnd()const& { return _Hwnd; };
private:
	std::pair<uint32_t, uint32_t> ClientSize{1920,1080};
	HWND _Hwnd;
};

