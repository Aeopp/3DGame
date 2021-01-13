#pragma once
#include "DxHelper.h"
#include "DllHelper.H"
#include "SingletonInterface.h"
#include "TypeAlias.h"
#include <numeric>
#include <array>

#define DIRECTINPUT_VERSION	0x0800
#include <dinput.h>

namespace Engine
{
	enum class MouseKey : uint8{ Left, Right, Mid, End};
	enum class MouseMove :uint8{ X, Y, Z, End };

	class DLL_DECL Controller : public SingletonInterface< Controller >
	{
	public:
		void Initialize(HINSTANCE hInst, HWND hWnd)&;
		void Update() & noexcept;
	public:
		inline int8	 GetKeyState(const uint8 byKeyID)const&;
		inline int8	 GetMouseState(const MouseKey _MouseKey)const&;
		inline int32 GetMouseMove(const MouseMove _MouseMove)const&;
	private:
		DX::UniquePtr<IDirectInput8> InputSDK{ nullptr };
		DX::UniquePtr<IDirectInputDevice8> Mouse {  nullptr};
		DX::UniquePtr<IDirectInputDevice8> KeyBoard{ nullptr };
	private:
		std::array<uint8, 256> KeyState{ 0,};
		DIMOUSESTATE			MouseState;
	};
};

inline int8 Engine::Controller::GetKeyState(const uint8 byKeyID) const&
{ 
	return KeyState[byKeyID]; 
}

inline int8 Engine::Controller::GetMouseState(const MouseKey _MouseKey) const&
{
	return MouseState.rgbButtons[static_cast<uint8>(_MouseKey)];
}

inline int32 Engine::Controller::GetMouseMove(const MouseMove _MouseMove) const&
{
	return *(((int32*)&MouseState) + static_cast<uint8>(_MouseMove));
}
