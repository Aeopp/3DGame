#pragma once
#include "DLLHELPER.H"
#include "SingletonInterface.h"
#include "TypeAlias.h"
#include <numeric>
#include <array>

namespace Engine
{
	class DLL_DECL Controller : public SingletonInterface< Controller >
	{
	public:
		enum class KeyState : uint8
		{
			Begin = 0u,
			Up,
			Down,
			Pressing,
			None,
			END,
		};
	public:
		void Initialize()&;
		void Update()&noexcept;
	public:
		FORCEINLINE KeyState GetState(const uint8 Key)const&;
		FORCEINLINE bool IsDown(const uint8 Key)const&;
		FORCEINLINE bool IsPressing(const uint8 Key)const&;
		FORCEINLINE bool IsUp(const uint8 Key)const&;
		FORCEINLINE bool IsNone(const uint8 Key)const&;
	private:
		static constexpr uint8 KeyMappingTableSize =(std::numeric_limits<uint8>::max)();
		std::array<KeyState, KeyMappingTableSize> KeyStateTable;
	};
};

Engine::Controller::KeyState Engine::Controller::GetState(const uint8 Key) const&
{
	return KeyStateTable[Key];
}

bool Engine::Controller::IsDown(const uint8 Key) const&
{
	return KeyStateTable[Key] == Controller::KeyState::Down;
};
bool Engine::Controller::IsPressing(const uint8 Key) const&
{
	return KeyStateTable[Key] == Controller::KeyState::Pressing;
};
bool Engine::Controller::IsUp(const uint8 Key) const&
{
	return KeyStateTable[Key] == Controller::KeyState::Up;
};
bool Engine::Controller::IsNone(const uint8 Key) const&
{
	return KeyStateTable[Key] == Controller::KeyState::None;
};
