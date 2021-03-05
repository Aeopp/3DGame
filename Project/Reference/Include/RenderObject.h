#pragma once
#include "Object.h"
#include "RenderInterface.h"
#include "MathStruct.h"

// 해당 클래스는 렌더와 관련된 컴포넌트의 도움 없이 직접 렌더링을 수행하는 오브젝트.
namespace Engine
{
	class DLL_DECL RenderObject : public Object
	{
	public:
		using Super = Object; 
	public:
		void Initialize()&;
		void PrototypeInitialize(IDirect3DDevice9* const Device)&;
	public:
		virtual void Event()&;
		virtual void LateUpdate(const float DeltaTime) & override;
		virtual void Update(const float DeltaTime) & override;
	protected:
		IDirect3DDevice9* Device{ nullptr };
	};
};



