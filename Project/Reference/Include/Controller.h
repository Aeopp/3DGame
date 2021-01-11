#pragma once
#include "DLLHELPER.H"
#include "SingletonInterface.h"
#include "TypeAlias.h"
#include <numeric>
#include <bitset>

namespace Engine
{
	class DLL_DECL Controller : public SingletonInterface< Controller >
	{
	public:
		enum class KeyState : uint8
		{
			Begin=0u,
			Up,
			Down,
			Pressing,
			END,
		};
	public:
		void Update()&noexcept;
		void Clear() & noexcept;
	private:
		static constexpr uint8 KeyMappingTableSize =
			(std::numeric_limits<uint8>::max)();
		std::bitset<KeyMappingTableSize> KeyTable{};
	};
};


	