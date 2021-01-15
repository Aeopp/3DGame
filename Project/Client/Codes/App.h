#pragma once
#include "SingletonInterface.h"
#include "TypeAlias.h"
#include "Management.h"

class App : public SingletonInterface<App>
{
public:
	void Initialize(const HWND _Hwnd,const HINSTANCE HInstance)&;
public:
	void GameLoop();
public:
	const auto& GetHInstance()const&	{ return HInstance; };
private:
	void StartSceneLoad()&;
public : 
	template<typename Type>
	static constexpr inline std::pair<Type,Type> ClientSize{ 1920,1080 };
	static inline HWND Hwnd;
	static inline HINSTANCE HInstance;
private : 
};

