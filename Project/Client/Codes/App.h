#pragma once
#include "SingletonInterface.h"
#include "TypeAlias.h"
#include "Management.h"

class App : public SingletonInterface<App>
{
public:
	void Initialize(const HWND _Hwnd)&;
public:
	static const auto& GetClientSize() { return ClientSize; };
	const auto& GetHwnd()const& { return _Hwnd; };
	Engine::Management*  GetManagement() { return _Management; };
private:
	static inline std::pair<uint32,uint32> ClientSize{ 1920u,1080u };
	HWND _Hwnd;
	Engine::Management* _Management{ nullptr };
};

