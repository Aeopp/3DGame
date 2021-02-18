#pragma once
#include "DxHelper.h"
#include "DllHelper.H"
#include "SingletonInterface.h"
#include "TypeAlias.h"
#include <numeric>
#include <array>

#define DIRECTINPUT_VERSION	0x0800
#include <dinput.h>

#define DIK_LEFTCLICK  256u
#define DIK_RIGHTCLICK 257u
#define DIK_MIDCLICK   258u

namespace Engine
{
	enum class MouseMove :  uint8{ X, Y, Z, End };
	enum class InputState : uint8{Down,Up,Pressing,None};

	class DLL_DECL Controller : public SingletonInterface<Controller>
	{
	public:
		void Initialize(HINSTANCE hInst, HWND hWnd)&;
		void Update() & noexcept;
	public:
		inline InputState	 GetKeyState(const uint16 byKeyID)const&;
		inline float GetMouseMove(const MouseMove _MouseMove)const&;
		inline bool IsDown(const uint16 byKeyID)const&;
		inline bool IsPressing(const uint16 byKeyID)const&;
		inline bool IsUp(const uint16 byKeyID)const&;
		inline bool IsNone(const uint16 byKeyID)const&;
	private:
		enum class MouseKey : uint8 { Left, Right, Mid, End };
	private:
		inline bool InputCheckImplementation(const uint16 byKeyID,
										   const InputState _KeyState)const&;
		inline int8	 GetMouseState(const MouseKey _MouseKey)const&;
	private:
		void InputStateCalc()&;
	private:
		DX::UniquePtr<IDirectInput8>       InputSDK{ nullptr };
		DX::UniquePtr<IDirectInputDevice8> Mouse   { nullptr };
		DX::UniquePtr<IDirectInputDevice8> KeyBoard{ nullptr };
	private:
										/*256(키보드) +3(마우스)*/
		static inline constexpr uint16 InputStateNum = 256u + 3u;
		std::array<uint8, InputStateNum> CurrentFrameInput{ 0,};
		std::array<InputState, InputStateNum> InputStates{ InputState::None, };
		DIMOUSESTATE			MouseState;
	};
};

inline Engine::InputState Engine::Controller::GetKeyState(const uint16 byKeyID) const&
{ 
	return InputStates[byKeyID];
}

inline int8 Engine::Controller::GetMouseState(const MouseKey _MouseKey) const&
{
	return (MouseState.rgbButtons[static_cast<uint8>(_MouseKey)]);
}

inline float Engine::Controller::GetMouseMove(const MouseMove _MouseMove) const&
{
	return  static_cast<float>(*(((int32*)&MouseState) + static_cast<uint8>(_MouseMove)));
}

inline bool Engine::Controller::InputCheckImplementation(
	const uint16 byKeyID, const InputState _KeyState) const&
{
	return InputStates[byKeyID] == _KeyState;
};

inline bool Engine::Controller::IsDown(const uint16 byKeyID)const&
{
	return InputCheckImplementation(byKeyID, InputState::Down);
};
inline bool Engine::Controller::IsPressing(const uint16 byKeyID)const&
{
	return InputCheckImplementation(byKeyID, InputState::Pressing);
};
inline bool Engine::Controller::IsUp(const uint16 byKeyID)const&
{
	return InputCheckImplementation(byKeyID, InputState::Up);
};
inline bool Engine::Controller::IsNone(const uint16 byKeyID)const&
{
	return InputCheckImplementation(byKeyID, InputState::None);
}
