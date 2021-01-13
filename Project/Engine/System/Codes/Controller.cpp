#include "Controller.h"


void Engine::Controller::Initialize(HINSTANCE hInst, HWND hWnd)&
{
	IDirectInput8* InputSDKTemp;
	// DInput �İ�ü�� �����ϴ� �Լ�
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

	// ��ġ�� ���� �������� �������ִ� �Լ�, (Ŭ���̾�Ʈ�� ���ִ� ���¿��� Ű �Է��� ������ ������ �����ϴ� �Լ�)
	KeyBoard->SetCooperativeLevel(hWnd, DISCL_BACKGROUND | DISCL_NONEXCLUSIVE);

	// ��ġ�� ���� access ������ �޾ƿ��� �Լ�
	KeyBoard->Acquire();

	// ���콺 ��ü ����
	IDirectInputDevice8* MouseTemp;
	InputSDK->CreateDevice(GUID_SysMouse, &MouseTemp, nullptr);

	Mouse= DX::MakeUnique(MouseTemp);

	// ������ ���콺 ��ü�� ���� ������ �� ��ü���� �����ϴ� �Լ�
	Mouse->SetDataFormat(&c_dfDIMouse);

	// ��ġ�� ���� �������� �������ִ� �Լ�, Ŭ���̾�Ʈ�� ���ִ� ���¿��� Ű �Է��� ������ ������ �����ϴ� �Լ�
	Mouse->SetCooperativeLevel(hWnd, DISCL_BACKGROUND | DISCL_NONEXCLUSIVE);

	// ��ġ�� ���� access ������ �޾ƿ��� �Լ�
	Mouse->Acquire();
}

void Engine::Controller::Update() & noexcept
{
	KeyBoard->GetDeviceState(256, KeyState.data());
	Mouse->GetDeviceState(sizeof(MouseState), &MouseState);
}
