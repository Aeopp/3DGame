#pragma once
#include "TypeAlias.h"
#include "DllHelper.H"
#include "Camera.h"


namespace Engine
{
	class DLL_DECL DynamicCamera : public  Camera
	{
	public :
		using Super = Camera;
		virtual void Update(const float DeltaTime)& override;
		virtual void LateUpdate(const float DeltaTime) & override;
		virtual void Event()& override;
		void Initialize(const float FovY,
						const float Near,
						const float Far,
						const float Aspect ,
						const float Speed , 
						class Controller*const _Control)&;
		void PrototypeInitialize(IDirect3DDevice9* const Device ,
								  const HWND Hwnd)&;
	public:
		float Speed = 100.f;
	private:
		void MouseFix()&;
		void MouseFixToggle()&;
	private:
		bool bMouseFix{ false };
		HWND Hwnd{ NULL };
		Controller* _Control{ nullptr };
	};
};



