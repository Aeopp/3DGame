#pragma once
#include "DxHelper.h"
#include "TypeAlias.h"
#include "DllHelper.H"


namespace Engine
{
	class DLL_DECL Aniamtion 
	{
	public:
		void Initialize(ID3DXAnimationController* const AnimationControl)&;
		void Clone()&;
		void SetAnimationIndex(const uint32 Index)&;
		// GetAnimationControl() 
	private:
		DX::SharedPtr<ID3DXAnimationController>AnimationControl{ nullptr };
		uint32 CurrentTrack{ 0u };
		uint32 NewTrack{ 1u};
		float AccelerationTime{ 0.f };
		uint32 OriginIndex{ 999 };
		double Duration{ 0.0 };
	};
};



