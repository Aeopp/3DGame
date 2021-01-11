#include "Controller.h"


void Engine::Controller::Update() & noexcept
{
	for (uint8 CurrentVirtualKey =0x01; CurrentVirtualKey <= 0xFE; ++CurrentVirtualKey)
	{
		auto& [CurrentKeyState] = KeyStateTable[CurrentVirtualKey];

		bool  bCurrentTableState = (GetAsyncKeyState(CurrentVirtualKey) & 0x8000);

		// 현재 호출시점 눌린 상태
		if (bCurrentTableState)
		{
			switch (CurrentKeyState)
			{
			case Controller::KeyState::Down:
				CurrentKeyState = Controller::KeyState::Pressing;
				break;
			case Controller::KeyState::Pressing:
				break;
			case Controller::KeyState::Up:
				CurrentKeyState = Controller::KeyState::Down;
				break;
			case Controller::KeyState::None:
				CurrentKeyState = Controller::KeyState::Down;
				break;
			default:
				break;
			}
		}
		else
		{
			switch (CurrentKeyState)
			{
			case Controller::KeyState::Down:
				CurrentKeyState = Controller::KeyState::Up;
				break;
			case Controller::KeyState::Pressing:
				CurrentKeyState = Controller::KeyState::Up;
				break;
			case Controller::KeyState::Up:
				CurrentKeyState = Controller::KeyState::None;
				break;
			case Controller::KeyState::None:
				break;
			default:
				break;
			}
		};
	}
}
