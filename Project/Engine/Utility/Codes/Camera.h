#pragma once
#include "TypeAlias.h"
#include "DllHelper.H"
#include "Object.h"


namespace Engine
{
	class DLL_DECL Camera : public  Object
	{
	public :
		using Super = Object;
		virtual void Update(const float DeltaTime)& override;
		virtual void LateUpdate(const float DeltaTime) & override;
		void Initialize()&;
		void PrototypeInitialize()&;
	private:
	};
};



