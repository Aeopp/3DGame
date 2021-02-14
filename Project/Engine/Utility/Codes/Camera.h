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
		virtual void Event() & override;
		virtual void Update(const float DeltaTime)& override;
		virtual void LateUpdate(const float DeltaTime) & override;
		void Initialize(const float FovY,
						const float Near,
						const float Far,
						const float Aspect)&;
		void PrototypeInitialize(IDirect3DDevice9* const Device)&;
	public:
		float FovY{ 0.0f };
		float Far{ 1000.f };
	protected:
		Vector3 Up{0,1,0};
		Vector3 Look{ 0,0,1 };
		IDirect3DDevice9* Device{ nullptr };
		float Near{ 0.1f };
		float Aspect{};
	};
};



