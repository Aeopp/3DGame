#include "Controller.h"


void Engine::Controller::Initialize(HINSTANCE hInst, HWND hWnd)&
{
	CurrentFrameInput.fill(0u);
	InputStates.fill(InputState::None);

	IDirectInput8* InputSDKTemp;
	// DInput 컴객체를 생성하는 함수
	DirectInput8Create(hInst,
		DIRECTINPUT_VERSION,
		IID_IDirectInput8,
		(void**)&InputSDKTemp,
		NULL);

	InputSDK = DX::MakeUnique(InputSDKTemp);

	IDirectInputDevice8* KeyBoardTemp;
	InputSDK->CreateDevice(GUID_SysKeyboard, &KeyBoardTemp, nullptr);

	KeyBoard = DX::MakeUnique(KeyBoardTemp);

	KeyBoard->SetDataFormat(&c_dfDIKeyboard);

	// 장치에 대한 독점권을 설정해주는 함수, (클라이언트가 떠있는 상태에서 키 입력을 받을지 말지를 결정하는 함수)
	KeyBoard->SetCooperativeLevel(hWnd, DISCL_BACKGROUND | DISCL_NONEXCLUSIVE);

	// 장치에 대한 access 버전을 받아오는 함수
	KeyBoard->Acquire();

	// 마우스 객체 생성
	IDirectInputDevice8* MouseTemp;
	InputSDK->CreateDevice(GUID_SysMouse, &MouseTemp, nullptr);

	Mouse= DX::MakeUnique(MouseTemp);

	// 생성된 마우스 객체의 대한 정보를 컴 객체에게 전달하는 함수
	Mouse->SetDataFormat(&c_dfDIMouse);

	// 장치에 대한 독점권을 설정해주는 함수, 클라이언트가 떠있는 상태에서 키 입력을 받을지 말지를 결정하는 함수
	Mouse->SetCooperativeLevel(hWnd, DISCL_BACKGROUND | DISCL_NONEXCLUSIVE);

	// 장치에 대한 access 버전을 받아오는 함수
	Mouse->Acquire();
}

void Engine::Controller::Update() & noexcept
{
	KeyBoard->GetDeviceState(256, CurrentFrameInput.data());
	Mouse->GetDeviceState(sizeof(MouseState), &MouseState);
	InputStateCalc();
}


void Engine::Controller::InputStateCalc()&
{
	CurrentFrameInput[DIK_LEFTCLICK] =  GetMouseState(MouseKey::Left);
	CurrentFrameInput[DIK_RIGHTCLICK] = GetMouseState(MouseKey::Right);
	CurrentFrameInput[DIK_MIDCLICK] =   GetMouseState(MouseKey::Mid);

	for (size_t Idx = 0u; Idx < InputStateNum; ++Idx)
	{
		const bool bDown = (CurrentFrameInput[Idx] & 0x80);

		InputState& CurrentKeyState = InputStates[Idx];

		if (bDown)
		{
			switch (CurrentKeyState)
			{
			case Engine::InputState::Down:
				CurrentKeyState = InputState::Pressing;
				break;
			case Engine::InputState::None:
			case Engine::InputState::Up:
				CurrentKeyState = InputState::Down;
				break;
			default:
				break;
			}
		}
		else
		{
			switch (CurrentKeyState)
			{
			case Engine::InputState::Pressing:
			case Engine::InputState::Down:
				CurrentKeyState = InputState::Up;
				break;
			case Engine::InputState::Up:
				CurrentKeyState = InputState::None;
				break;
			default:
				break;
			}
		}
	}
}
